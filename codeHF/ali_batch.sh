#!/bin/bash

LISTINPUT="$1"
JSON="$2"
FILEOUT="$3"

LogFile="log_ali_hf.log"
FilesToMerge="ListOutToMergeALI.txt"
DirBase=$(pwd)
Index=0
ListRunCommands="$DirBase/ListRunCommands.txt"

rm -f $ListRunCommands
rm -f $FilesToMerge
DirOutMain="output_ali"
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  if [ ! -f "$FileIn" ]; then
    echo "Error: File $FileIn does not exist."
    exit 1
  fi
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut
  echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo "$FileOut" >> $FilesToMerge
  #root -b -q -l "$DirBase/ComputeVerticesRun1.C(\"$FileIn\",\"$FileOut\",\"$JSON\")" > "$DirOut/$LogFile" 2>&1
  echo "root -b -q -l $DirBase/ComputeVerticesRun1.C\(\\\"$FileIn\\\",\\\"$FileOut\\\",\\\"$JSON\\\"\) > $DirOut/$LogFile 2>&1" >> "$ListRunCommands"
  ((Index+=1))
done < "$LISTINPUT"

echo "Running AliPhysics jobs..."
parallel -j0 --halt soon,fail=1 < $ListRunCommands > $LogFile 2>&1
if [ $? -ne 0 ]; then echo -e "Error\nCheck $(realpath $LogFile)"; exit 1; fi # Exit if error.
rm -f $ListRunCommands

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" >> $LogFile 2>&1
if [ $? -ne 0 ]; then echo -e "Error\nCheck $(realpath $LogFile)"; exit 1; fi # Exit if error.
rm -f $FilesToMerge

exit 0
