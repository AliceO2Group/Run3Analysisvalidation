#!/bin/bash

LISTINPUT="$1"
JSON="$2"
FILEOUT="$3"
TWOPRONGSEL="$4"

LogFile="log_ali_hf.log"
FilesToMerge="ListOutToMergeALI.txt"
DirBase=$(pwd)
Index=0

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
  #echo "Output file: $FileOut"
  root -b -q -l "$DirBase/ComputeVerticesRun1.C(\"$FileIn\",\"$FileOut\",\"$JSON\",$TWOPRONGSEL)" > "$DirOut/$LogFile" 2>&1 &
  PIDS+=($!)
  ((Index+=1))
done < "$LISTINPUT"

echo "Waiting for AliPhysics to finish..."
for pid in ${PIDS[@]}; do
  wait $pid
  STATUS+=($?)
done

i=0
ok=1
for st in ${STATUS[@]}; do
  if [ $st -ne 0 ]; then
    echo "$i failed"
    ok=0
  fi
  ((i+=1))
done
if [ $ok -ne 1 ]; then exit 1; fi # Exit if error.

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" > $LogFile 2>&1
if [ $? -ne 0 ]; then echo "Error"; exit 1; fi # Exit if error.
rm -f $FilesToMerge

exit 0
