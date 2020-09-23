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

[ -f "$LISTINPUT" ] || { echo "Error: File $LISTINPUT does not exist."; exit 1; }
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  [ -f "$FileIn" ] || { echo "Error: File $FileIn does not exist."; exit 1; }
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo "$FileOut" >> $FilesToMerge
  #root -b -q -l "$DirBase/ComputeVerticesRun1.C(\"$FileIn\",\"$FileOut\",\"$JSON\")" > "$DirOut/$LogFile" 2>&1
  echo "root -b -q -l $DirBase/ComputeVerticesRun1.C\(\\\"$FileIn\\\",\\\"$FileOut\\\",\\\"$JSON\\\"\) > $DirOut/$LogFile 2>&1" >> "$ListRunCommands"
  ((Index+=1))
done < "$LISTINPUT"

echo "Running AliPhysics jobs..."
parallel --halt soon,fail=100% < $ListRunCommands > $LogFile 2>&1 || \
{ echo -e "Error\nCheck $(realpath $LogFile)"; exit 1; }
rm -f $ListRunCommands

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" >> $LogFile 2>&1 || \
{ echo -e "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
rm -f $FilesToMerge

exit 0
