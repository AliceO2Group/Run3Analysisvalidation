#!/bin/bash

# Script to download files from the Grid

# Check correct input.
if [ -z "$2" ]; then
  echo "Error: You did not call the script correctly."
  echo "Usage: $0 <Grid source path> <local target path> <file names>"
  echo "Usage: $0 <file list> <local target path>"
  exit 1
fi

# Check correct environment.
if [ -z "$(which alien_find)" ]; then
  echo "Error: This script requires the JAliEn tools."
  echo "Load the xjalienfs environment and run the script again."
  echo "alienv enter xjalienfs/latest"
  exit 1
fi

# Check presence of GNU Parallel.
PARALLEL=1
[ "$(which parallel)" ] || PARALLEL=0

# Process command line arguments.
LISTEXT=0
if [ "$#" -eq 2 ]; then
  LISTEXT=1
  inputlist="$1" # file list
  path_local="$2" # local path
else
  path_grid="$1" # Grid path
  shift
  path_local="$1" # local path
  shift
  filenames="$@" # names of files
fi

timestamp="$(date +%Y%m%d_%H%M%S)_${BASHPID}"
logfile="stdouterr_${timestamp}.log"
scriptfile="script_${timestamp}.sh"
rm -f "$logfile"

# Handle list of files.
if [ "$LISTEXT" -eq 1 ]; then
  [ -f "$inputlist" ] || { echo "Error: File $inputlist does not exist"; exit 1; }
else
  inputlist="filelist_${timestamp}.txt"
  rm -f "$inputlist"
  echo "Creating list of files"
  for file in $filenames; do
    alien_find $path_grid/ $file >> "$inputlist" || { echo "Error"; exit 1; }
  done
fi
nfiles=$(wc -l < "$inputlist")

# Display summary and ask for confirmation.
if [ "$LISTEXT" -eq 1 ]; then
  echo "File list: $inputlist"
else
  echo "Source Grid path: $path_grid"
  echo "File names: $filenames"
fi
echo "Target local path: $path_local"
echo "Number of files: $nfiles"
echo -e "\nDo you wish to continue? (y/n)"
while true; do
  read -p "Answer: " yn
  case $yn in
    [y] ) echo "Proceeding"; break;;
    [n] ) echo "Aborting";  [ "$LISTEXT" -eq 0 ] && rm -f "$inputlist"; exit 0; break;;
    * ) echo "Please answer y or n.";;
  esac
done

# Process list of files.
speed=10 # number of download threads started per second
delay=$(echo "scale=10 ; 1 / $speed" | bc)
while read -r file; do
  path_alien="alien://${file}"
  [ "$LISTEXT" -eq 1 ] && path_target="$path_local/$file" || path_target="$path_local/${file/$path_grid/}"
  path_target="file:$path_target"
  if [ "$PARALLEL" -eq 1 ]; then
    # Prepare script for GNU Parallel.
    echo "mkdir -p \"$(dirname $path_target)\" && alien_cp -f -cksum \"$path_alien\" \"$path_target\"" >> "$scriptfile"
  else
    # Download using parallel background processes.
    mkdir -p "$(dirname $path_target)" || { echo "Error"; exit 1; }
    alien_cp -f -cksum ${path_alien} ${path_target} >> $logfile 2>&1 &
    sleep $delay
  fi
done < "$inputlist"

if [ "$PARALLEL" -eq 1 ]; then
  # Download with GNU Parallel.
  parallel --halt soon,fail=100% --will-cite --progress < "$scriptfile" > "$logfile"
  # Report result.
  nsuccess=$(grep "STATUS OK" "$logfile" | wc -l)
  nvalid=$(grep "TARGET VALID" "$logfile" | wc -l)
  ndone=$((nsuccess + nvalid))
  echo "Validated: $nvalid,  Downloaded: $nsuccess,  Done: $ndone/$nfiles"
else
  # Watch progress of background processes.
  nrunning=0
  pause=2 # [s] status update interval
  CMDNRUN="top -u $USER -n 1 -b -c | grep python3 | grep jalien | wc -l"
  # Wait for the start.
  while [ $nrunning -eq 0 ]; do nrunning=$(eval $CMDNRUN); done
  # Report status
  while [ $nrunning -gt 0 ]; do
    nstarted=$(grep "Start" $logfile | wc -l)
    nsuccess=$(grep "STATUS OK" $logfile | wc -l)
    nvalid=$(grep "TARGET VALID" $logfile | wc -l)
    ndone=$((nsuccess + nvalid))
    nrunning=$(eval $CMDNRUN)
    echo -ne "Started: $nstarted,  Running: $nrunning,  Validated: $nvalid,  Downloaded: $nsuccess,  Done: $ndone/$nfiles\r"
    if [ $nrunning -gt 0 ]; then
      sleep $pause
    else
      echo -e "\n"
    fi
  done
fi
echo -e "Logfile: $logfile"
echo "Done"

# Clean.
find /tmp -name *meta4 -user $USER -delete 2> /dev/null

# Check number of valid files.
if [ $nvalid -eq $nfiles ]; then
  echo "All downloaded files are valid."
elif [ $nvalid -lt $nfiles ]; then
  echo "Run the script again to validate the downloaded files."
fi

# Check number of successfully processed files.
if [ $ndone -ne $nfiles ]; then
  echo "Something went wrong."
  exit 1
fi

echo "All good!"
[ "$LISTEXT" -eq 1 ] && inputlist=""
rm -f "$inputlist" "$logfile" "$scriptfile"
exit 0
