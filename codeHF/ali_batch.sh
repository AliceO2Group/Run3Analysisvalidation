#!/bin/bash

LISTINPUT="$1"
JSON="$2"
FILEOUT="$3"
DEBUG=$4

LogFile="log_ali_hf.log"
FilesToMerge="ListOutToMergeALI.txt"
DirBase="$PWD"
Index=0
ListRunCommands="$DirBase/ListRunCommands.txt"
DirOutMain="output_ali"

rm -f $ListRunCommands
rm -f $FilesToMerge
rm -f $FILEOUT
rm -rf $DirOutMain

echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  if [ ! -f "$FileIn" ]; then
    echo "Error: File $FileIn does not exist."
    exit 1
  fi
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut
  if [ $DEBUG -eq 1 ]; then
    echo "Input file ($Index): $FileIn"
  fi
  FileOut="$DirOut/$FILEOUT"
  echo "$FileOut" >> $FilesToMerge
  #root -b -q -l "$DirBase/ComputeVerticesRun1.C(\"$FileIn\",\"$FileOut\",\"$JSON\")" > "$DirOut/$LogFile" 2>&1
  echo "root -b -q -l $DirBase/ComputeVerticesRun1.C\(\\\"$FileIn\\\",\\\"$FileOut\\\",\\\"$JSON\\\"\) > $DirOut/$LogFile 2>&1" >> "$ListRunCommands"
  ((Index+=1))
done < "$LISTINPUT"

echo "Running AliPhysics jobs..."
parallel --halt soon,fail=100% < $ListRunCommands > $LogFile 2>&1
if [ $? -ne 0 ]; then echo -e "Error\nCheck $(realpath $LogFile)"; exit 1; fi # Exit if error.
rm -f $ListRunCommands

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" >> $LogFile 2>&1
if [ $? -ne 0 ]; then echo -e "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; fi # Exit if error.
rm -f $FilesToMerge

exit 0
