#!/bin/bash

# Script to run AliPhysics tasks in jobs.

# Load utilities.
source utilities.sh || ErrExit "Failed to load utilities."

LISTINPUT="$1"
JSON="$2"
SCRIPT="$3"
DEBUG=$4
FILEOUT="AnalysisResults.root"

SCRIPT="$(realpath $SCRIPT)"
JSON="$(realpath $JSON)"
CheckFile "$SCRIPT"
CheckFile "$JSON"

LogFile="log_ali.log"
FilesToMerge="ListOutToMergeALI.txt"
DirBase="$PWD"
Index=0
ListRunScripts="$DirBase/ListRunScripts.txt"
DirOutMain="output_ali"

rm -f $ListRunScripts && \
rm -f $FilesToMerge && \
rm -f $FILEOUT && \
rm -rf $DirOutMain || ErrExit

CheckFile "$LISTINPUT"
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  CheckFile "$FileIn"
  FileIn="$(realpath $FileIn)"
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut && \
  cd $DirOut || ErrExit
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo "$FileOut" >> "$DirBase/$FilesToMerge" || ErrExit
  RUNSCRIPT="run.sh"
  cat << EOF > $RUNSCRIPT # Create the job script.
#!/bin/bash
cd "$DirBase/$DirOut"
bash $SCRIPT "$FileIn" "$JSON" > $LogFile 2>&1
EOF
  echo "bash $(realpath $RUNSCRIPT)" >> "$ListRunScripts" && \
  ((Index+=1)) && \
  cd $DirBase || ErrExit
done < "$LISTINPUT"

echo "Running AliPhysics jobs..."
parallel --halt soon,fail=100% < $ListRunScripts > $LogFile 2>&1 || \
ErrExit "\nCheck $(realpath $LogFile)"
rm -f $ListRunScripts || ErrExit

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" >> $LogFile 2>&1 || \
{ MsgErr "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
rm -f $FilesToMerge || ErrExit

exit 0
