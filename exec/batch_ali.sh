#!/bin/bash

# Script to run AliPhysics tasks in jobs

LISTINPUT="$1"
JSON="$2"
SCRIPT="$3"
DEBUG=$4
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

LogFile="log_ali.log"
FilesToMerge="ListOutToMergeALI.txt"
DirBase="$PWD"
Index=0
ListRunScripts="$DirBase/ListRunScripts.txt"
DirOutMain="output_ali"

rm -f $ListRunScripts && \
rm -f $FilesToMerge && \
rm -f $FILEOUT && \
rm -rf $DirOutMain || ErrExit "Failed to delete output files."

CheckFile "$LISTINPUT"
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  CheckFile "$FileIn"
  FileIn="$(realpath $FileIn)"
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut && \
  cd $DirOut || ErrExit "Failed to cd $DirOut."
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo "$FileOut" >> "$DirBase/$FilesToMerge" || ErrExit "Failed to echo to $DirBase/$FilesToMerge."
  RUNSCRIPT="run.sh"
  cat << EOF > $RUNSCRIPT # Create the job script.
#!/bin/bash
DirThis="\$(dirname \$(realpath \$0))"
cd "\$DirThis"
bash $SCRIPT "$FileIn" "$JSON" > $LogFile 2>&1
EOF
  echo "bash $(realpath $RUNSCRIPT)" >> "$ListRunScripts" && \
  ((Index+=1)) && \
  cd $DirBase || ErrExit "Failed to cd $DirBase."
done < "$LISTINPUT"

echo "Running AliPhysics jobs... ($(cat $ListRunScripts | wc -l) jobs)"
OPT_PARALLEL="--halt soon,fail=100%"
if [ $DEBUG -eq 0 ]; then
  parallel $OPT_PARALLEL < $ListRunScripts > $LogFile 2>&1
else
  parallel $OPT_PARALLEL --will-cite --progress < $ListRunScripts > $LogFile
fi
[ $? -ne 0 ] && ErrExit "\nCheck $(realpath $LogFile)"
rm -f $ListRunScripts || ErrExit "Failed to rm $ListRunScripts."

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" >> $LogFile 2>&1 || \
{ MsgErr "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
rm -f $FilesToMerge || ErrExit "Failed to rm $FilesToMerge."

exit 0
