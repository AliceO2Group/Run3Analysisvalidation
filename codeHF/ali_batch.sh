#!/bin/bash

LISTINPUT="$1"
JSON="$2"
FILEOUT="$3"

LogFile="log_ali_hf.log"
FilesToMerge="ListOutToMergeALI.txt"
DirBase=$(pwd)
Index=0

echo -e "\nRunning the HF tasks with AliPhysics... (logfile: $LogFile)"
rm -f $FilesToMerge
while read FileIn; do
  if [ ! -f "$FileIn" ]; then
    echo "Error: Fle $FileIn does not exist."
    exit 1
  fi
  DirOut="output_ali/$Index"
  mkdir -p $DirOut
  echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo "$FileOut" >> $FilesToMerge
  echo "Output file: $FileOut"
  root -b -q -l "$DirBase/ComputeVerticesRun1.C(\"$FileIn\",\"$FileOut\",\"$JSON\")" > "$DirOut/$LogFile" 2>&1 &
  Index=$((Index+1))
done < "$LISTINPUT"
CmdNRun="top -u $USER -n 1 -c | grep root | grep ComputeVerticesRun1 | wc -l"
echo "Waiting for AliPhysics to start..."
while [ $(eval $CmdNRun) -eq 0 ]; do continue; done
echo "Waiting for AliPhysics to finish..."
while [ $(eval $CmdNRun) -gt 0 ]; do
  echo $(eval $CmdNRun)
  sleep 1
done
echo "Merging output files... (output file: $FILEOUT)"
hadd $FILEOUT @"$FilesToMerge" > $LogFile 2>&1
if [ $? -ne 0 ]; then echo "Error"; exit 1; fi # Exit if error.
rm -f $FilesToMerge

exit 0
