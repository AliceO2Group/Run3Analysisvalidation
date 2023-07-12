#!/bin/bash

# Script to run Run 2 to Run 3 conversion in jobs

LISTINPUT="$1"
LISTOUTPUT="$2"
INPUT_IS_MC=$3
USEALIEVCUTS=$4
DEBUG=$5
NFILESPERJOB=$6
FILEOUT="AO2D.root"

[ "$DEBUG" -eq 1 ] && echo "Running $0"

# This directory
DIR_THIS="$(dirname "$(realpath "$0")")"

# Load utilities.
# shellcheck disable=SC1091 # Ignore not following.
source "$DIR_THIS/utilities.sh" || { echo "Error: Failed to load utilities."; exit 1; }


LogFile="log_convert.log"
ListIn="list_convert.txt"
IndexFile=0
IndexJob=0
DirOutMain="output_conversion"

CMDPARALLEL="cd \"$DirOutMain/{}\" && bash \"$DIR_THIS/run_convert.sh\" \"$ListIn\" $INPUT_IS_MC $USEALIEVCUTS \"$LogFile\""

# Clean before running.
rm -rf "$LISTOUTPUT" "$DirOutMain" || ErrExit "Failed to delete output files."

CheckFile "$LISTINPUT"
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
# Loop over input files
while read -r FileIn; do
  CheckFile "$FileIn"
  FileIn="$(realpath "$FileIn")"
  IndexJob=$((IndexFile / NFILESPERJOB))
  DirOut="$DirOutMain/$IndexJob"
  # New job
  if [ $((IndexFile % NFILESPERJOB)) -eq 0 ]; then
    mkdir -p $DirOut || ErrExit "Failed to mkdir $DirOut."
    echo "$DirOut/$FILEOUT" >> "$LISTOUTPUT" || ErrExit "Failed to echo to $LISTOUTPUT."
  fi
  echo "$FileIn" >> "$DirOut/$ListIn" || ErrExit "Failed to echo to $DirOut/$ListIn."
  [ "$DEBUG" -eq 1 ] && echo "Input file ($IndexFile, job $IndexJob): $FileIn"
  ((IndexFile++))
done < "$LISTINPUT"

echo "Running conversion jobs... ($((IndexJob+1)) jobs, $NFILESPERJOB files/job)"
OPT_PARALLEL="--halt soon,fail=100%"
if [ "$DEBUG" -eq 0 ]; then
  # shellcheck disable=SC2086 # Ignore unquoted options.
  parallel $OPT_PARALLEL "$CMDPARALLEL" ::: $(seq 0 $IndexJob) > $LogFile 2>&1
else
  # shellcheck disable=SC2086 # Ignore unquoted options.
  parallel $OPT_PARALLEL --will-cite --progress "$CMDPARALLEL" ::: $(seq 0 $IndexJob) > $LogFile
fi || ErrExit "\nCheck $(realpath $LogFile)"
grep -q -e '^'"W-" -e '^'"Warning" "$LogFile" && MsgWarn "There were warnings!\nCheck $(realpath $LogFile)"
grep -q -e '^'"E-" -e '^'"Error" "$LogFile" && MsgErr "There were errors!\nCheck $(realpath $LogFile)"
grep -q -e '^'"F-" -e '^'"Fatal" -e "segmentation" -e "Segmentation" "$LogFile" && ErrExit "There were fatal errors!\nCheck $(realpath $LogFile)"

exit 0
