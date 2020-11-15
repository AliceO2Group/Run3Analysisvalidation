#!/bin/bash

# Load utilities.
source utilities.sh || { MsgErr "Error: Failed to load utilities."; exit 1; }

LISTINPUT="$1"
JSON="$2"
SCRIPT="$3"
DEBUG=$4
FILEOUT="AnalysisResults.root"

SCRIPT="$(realpath $SCRIPT)"
JSON="$(realpath $JSON)"
ok=1
for file in "$SCRIPT" "$JSON"; do
  [ -f "$file" ] || { MsgErr "Error: File $file does not exist."; ok=0; }
done
[ $ok -ne 1 ] && exit 1

LogFile="log_ali.log"
FilesToMerge="ListOutToMergeALI.txt"
DirBase="$PWD"
Index=0
ListRunScripts="$DirBase/ListRunScripts.txt"
DirOutMain="output_ali"

rm -f $ListRunScripts && \
rm -f $FilesToMerge && \
rm -f $FILEOUT && \
rm -rf $DirOutMain || { MsgErr "Error"; exit 1; }

[ -f "$LISTINPUT" ] || { MsgErr "Error: File $LISTINPUT does not exist."; exit 1; }
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  [ -f "$FileIn" ] || { MsgErr "Error: File $FileIn does not exist."; exit 1; }
  FileIn="$(realpath $FileIn)"
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut && \
  cd $DirOut || { MsgErr "Error"; exit 1; }
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo "$FileOut" >> "$DirBase/$FilesToMerge" || { MsgErr "Error"; exit 1; }
  RUNSCRIPT="run.sh"
  cat << EOF > $RUNSCRIPT # Create the job script.
#!/bin/bash
cd "$DirBase/$DirOut"
bash $SCRIPT "$FileIn" "$JSON" > $LogFile 2>&1
EOF
  echo "bash $(realpath $RUNSCRIPT)" >> "$ListRunScripts" && \
  ((Index+=1)) && \
  cd $DirBase || { MsgErr "Error"; exit 1; }
done < "$LISTINPUT"

echo "Running AliPhysics jobs..."
parallel --halt soon,fail=100% < $ListRunScripts > $LogFile 2>&1 || \
{ MsgErr "Error\nCheck $(realpath $LogFile)"; exit 1; }
rm -f $ListRunScripts || { MsgErr "Error"; exit 1; }

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" >> $LogFile 2>&1 || \
{ MsgErr "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
rm -f $FilesToMerge || { MsgErr "Error"; exit 1; }

exit 0
