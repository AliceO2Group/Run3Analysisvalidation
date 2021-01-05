#!/bin/bash

# Script to run Run 2 to Run 3 conversion in jobs

LISTINPUT="$1"
LISTOUTPUT="$2"
ISMC=$3
DEBUG=$4
FILEOUT="AO2D.root"

[ $DEBUG -eq 1 ] && echo "Running $0"

# This directory
DIR_THIS="$(dirname $(realpath $0))"

# Load utilities.
source "$DIR_THIS/utilities.sh" || { echo "Error: Failed to load utilities."; exit 1; }

LogFile="log_convert.log"
ListIn="list_convert.txt"
DirBase="$PWD"
Index=0
ListRunScripts="$DirBase/ListRunScriptsConversion.txt"
DirOutMain="output_conversion"

# Clean before running.
rm -f $ListRunScripts && \
rm -f $LISTOUTPUT && \
rm -rf $DirOutMain || ErrExit "Failed to delete output files."

CheckFile "$LISTINPUT"
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
# Loop over input files
while read FileIn; do
  CheckFile "$FileIn"
  FileIn="$(realpath $FileIn)"
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut || ErrExit "Failed to mkdir $DirOut."
  echo $FileIn > "$DirOut/$ListIn" || ErrExit "Failed to echo to $DirOut/$ListIn."
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo "$DirBase/$FileOut" >> "$DirBase/$LISTOUTPUT" || ErrExit "Failed to echo to $DirBase/$LISTOUTPUT."
  # Add this job in the list of commands.
  echo "cd \"$DirOut\" && bash \"$DIR_THIS/run_convert.sh\" \"$ListIn\" $ISMC \"$LogFile\"" >> "$ListRunScripts" || ErrExit "Failed to echo to $ListRunScripts."
  ((Index+=1))
done < "$LISTINPUT"

echo "Running conversion jobs... ($(cat $ListRunScripts | wc -l) jobs)"
OPT_PARALLEL="--halt soon,fail=100%"
if [ $DEBUG -eq 0 ]; then
  parallel $OPT_PARALLEL < $ListRunScripts > $LogFile 2>&1
else
  parallel $OPT_PARALLEL --will-cite --progress < $ListRunScripts > $LogFile
fi
[ $? -ne 0 ] && ErrExit "\nCheck $(realpath $LogFile)"
rm -f $ListRunScripts || ErrExit "Failed to rm $ListRunScripts."

exit 0
