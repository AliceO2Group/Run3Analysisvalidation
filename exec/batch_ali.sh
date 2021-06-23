#!/bin/bash

# Script to run AliPhysics tasks in jobs

LISTINPUT="$1"
JSON="$2"
SCRIPT="$3"
DEBUG=$4
NFILESPERJOB=$5
FILEOUT="AnalysisResults.root"

[ "$DEBUG" -eq 1 ] && echo "Running $0"

# This directory
DIR_THIS="$(dirname "$(realpath "$0")")"

# Load utilities.
# shellcheck disable=SC1091 # Ignore not following.
source "$DIR_THIS/utilities.sh" || { echo "Error: Failed to load utilities."; exit 1; }


CheckFile "$SCRIPT"
CheckFile "$JSON"
SCRIPT="$(realpath "$SCRIPT")"
JSON="$(realpath "$JSON")"

LogFile="log_ali.log"
ListIn="list_ali.txt"
FilesToMerge="ListOutToMergeAli.txt"
IndexFile=0
IndexJob=0
DirOutMain="output_ali"

CMDPARALLEL="cd \"$DirOutMain/{}\" && bash \"$DIR_THIS/run_ali.sh\" \"$SCRIPT\" \"$ListIn\" \"$JSON\" \"$LogFile\""

# Clean before running.
rm -rf "$FilesToMerge" "$FILEOUT" "$DirOutMain" || ErrExit "Failed to delete output files."

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
    echo "$DirOut/$FILEOUT" >> "$FilesToMerge" || ErrExit "Failed to echo to $FilesToMerge."
  fi
  echo "$FileIn" >> "$DirOut/$ListIn" || ErrExit "Failed to echo to $DirOut/$ListIn."
  [ "$DEBUG" -eq 1 ] && echo "Input file ($IndexFile, job $IndexJob): $FileIn"
  ((IndexFile++))
done < "$LISTINPUT"

echo "Running AliPhysics jobs... ($((IndexJob+1)) jobs, $NFILESPERJOB files/job)"
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
grep -q -e '^'"F-" -e '^'"Fatal" -e "segmentation" "$LogFile" && ErrExit "There were fatal errors!\nCheck $(realpath $LogFile)"

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd "$FILEOUT" @"$FilesToMerge" >> $LogFile 2>&1 || \
{ MsgErr "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
rm -f "$FilesToMerge" || ErrExit "Failed to rm $FilesToMerge."

exit 0
