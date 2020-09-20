#!/bin/bash

LISTINPUT="$1"
JSON="$2"
SCRIPT="$3"
DEBUG=$4
FILEOUT="AnalysisResults.root"

LogFile="log_o2.log"
FilesToMerge="ListOutToMergeO2.txt"
DirBase=$(pwd)
Index=0
JSONLocal=$(basename $JSON)
ListRunScripts="$DirBase/ListRunScripts.txt"

rm -f $ListRunScripts
rm -f $FilesToMerge
DirOutMain="output_o2"
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  if [ ! -f "$FileIn" ]; then
    echo "Error: File $FileIn does not exist."
    exit 1
  fi
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut
  cd $DirOut
  cp "$JSON" $JSONLocal
  sed -e "s!@$LISTINPUT!$FileIn!g" $JSONLocal > $JSONLocal.tmp && mv $JSONLocal.tmp $JSONLocal
  cp "$DirBase/$SCRIPT" .
  sed -e "s!$DirBase!$PWD!g" $SCRIPT > $SCRIPT.tmp && mv $SCRIPT.tmp $SCRIPT
  if [ $DEBUG -eq 1 ]; then
    echo "Input file ($Index): $FileIn"
  fi
  FileOut="$DirOut/$FILEOUT"
  echo $FileOut >> "$DirBase/$FilesToMerge"
  RUNSCRIPT="run.sh"
  cat << EOF > $RUNSCRIPT # Create a temporary script.
#!/bin/bash
cd "$DirBase/$DirOut"
bash $SCRIPT > $LogFile 2>&1
ExitCode=\$?
grep WARN "$LogFile" | sort -u
pid=\$(tail -n 2 "$LogFile" | grep "is exiting" | cut -d " " -f 3) # Get the process ID from the O2 log.
find /tmp -group \$USER -name "localhost\${pid}_*" -delete 2> /dev/null # Delete the process sockets.
exit \$ExitCode
EOF
  echo "bash $(realpath $RUNSCRIPT)" >> "$ListRunScripts"
  ((Index+=1))
  cd $DirBase
done < "$LISTINPUT"

echo "Running O2 jobs..."
parallel --halt soon,fail=100% < $ListRunScripts > $LogFile 2>&1
ExitCode=$?
find /tmp -group $USER -name "localhost*_*" -delete 2> /dev/null # Delete all user's sockets.
if [ $ExitCode -ne 0 ]; then echo -e "Error\nCheck $(realpath $LogFile)"; exit 1; fi # Exit if error.
rm -f $ListRunScripts

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" >> $LogFile 2>&1
if [ $? -ne 0 ]; then echo -e "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; fi # Exit if error.
rm -f $FilesToMerge

exit 0
