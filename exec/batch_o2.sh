#!/bin/bash

# Script to run O2 tasks in jobs

LISTINPUT="$1"
JSON="$2"
SCRIPT="$3"
DEBUG=$4
NFILESPERJOB=$5
FILEOUT_TREE="$6"
FILEOUT="AnalysisResults.root"

[ $DEBUG -eq 1 ] && echo "Running $0"

# This directory
DIR_THIS="$(dirname $(realpath $0))"

# Load utilities.
source "$DIR_THIS/utilities.sh" || { echo "Error: Failed to load utilities."; exit 1; }

CheckFile "$SCRIPT"
CheckFile "$JSON"
SCRIPT="$(realpath $SCRIPT)"
JSON="$(realpath $JSON)"

LogFile="log_o2.log"
ListIn="list_o2.txt"
FilesToMerge="ListOutToMergeO2.txt"
FilesToMergeTree="ListOutToMergeO2Tree.txt"
DirBase="$PWD"
IndexFile=0
ListRunScripts="$DirBase/ListRunScriptsO2.txt"
DirOutMain="output_o2"

# Clean before running.
rm -f $ListRunScripts && \
rm -f $FilesToMerge && \
rm -f $FilesToMergeTree && \
rm -f $FILEOUT && \
rm -f $FILEOUT_TREE && \
rm -rf $DirOutMain || ErrExit "Failed to delete output files."

CheckFile "$LISTINPUT"
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
# Loop over input files
while read FileIn; do
  CheckFile "$FileIn"
  FileIn="$(realpath $FileIn)"
  IndexJob=$((IndexFile / NFILESPERJOB))
  DirOut="$DirOutMain/$IndexJob"
  # New job
  if [ $((IndexFile % NFILESPERJOB)) -eq 0 ]; then
    mkdir -p $DirOut || ErrExit "Failed to mkdir $DirOut."
    FileOut="$DirOut/$FILEOUT"
    echo $FileOut >> "$DirBase/$FilesToMerge" || ErrExit "Failed to echo to $DirBase/$FilesToMerge."
    [ "$FILEOUT_TREE" ] && {
      FileOutTree="$DirOut/$FILEOUT_TREE"
      echo $FileOutTree >> "$DirBase/$FilesToMergeTree" || ErrExit "Failed to echo to $DirBase/$FilesToMergeTree."
    }
    # Add this job in the list of commands.
    echo "cd \"$DirOut\" && bash \"$DIR_THIS/run_o2.sh\" \"$SCRIPT\" \"$ListIn\" \"$JSON\" \"$LogFile\"" >> "$ListRunScripts" || ErrExit "Failed to echo to $ListRunScripts."
  fi
  echo $FileIn >> "$DirOut/$ListIn" || ErrExit "Failed to echo to $DirOut/$ListIn."
  [ $DEBUG -eq 1 ] && echo "Input file ($IndexFile, job $IndexJob): $FileIn"
  ((IndexFile+=1))
done < "$LISTINPUT"

echo "Running O2 jobs... ($(cat $ListRunScripts | wc -l) jobs)"
OPT_PARALLEL="--halt soon,fail=100%"
if [ $DEBUG -eq 0 ]; then
  parallel $OPT_PARALLEL < $ListRunScripts > $LogFile 2>&1
else
  parallel $OPT_PARALLEL --will-cite --progress < $ListRunScripts > $LogFile
fi
ExitCode=$?
find /tmp -maxdepth 1 -type s -group $USER -name "localhost*_*" -delete 2> /dev/null # Delete all user's sockets.
[ $ExitCode -ne 0 ] && ErrExit "\nCheck $(realpath $LogFile)"
[ "$(grep \\[WARN\\] "$LogFile")" ] && MsgWarn "There were warnings!\nCheck $(realpath $LogFile)"
[ "$(grep \\[ERROR\\] "$LogFile")" ] && MsgErr "There were errors!\nCheck $(realpath $LogFile)"
rm -f $ListRunScripts || ErrExit "Failed to rm $ListRunScripts."

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" >> $LogFile 2>&1 || \
{ MsgErr "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
rm -f $FilesToMerge || ErrExit "Failed to rm $FilesToMerge."

[ "$FILEOUT_TREE" ] && {
  echo "Merging output trees... (output file: $FILEOUT_TREE, logfile: $LogFile)"
  hadd $FILEOUT_TREE @"$FilesToMergeTree" >> $LogFile 2>&1 || \
  { MsgErr "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
  rm -f $FilesToMergeTree || ErrExit "Failed to rm $FilesToMergeTree."
}

exit 0
