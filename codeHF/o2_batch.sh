#!/bin/bash

LISTINPUT="$1"
JSON="$2"
SCRIPT="$3"
FILEOUT="AnalysisResults.root"

LogFile="log_o2.log"
FilesToMerge="ListOutToMergeO2.txt"
DirBase=$(pwd)
Index=0
JSONLocal=$(basename $JSON)

rm -f $FilesToMerge
while read FileIn; do
  if [ ! -f "$FileIn" ]; then
    echo "Error: Fle $FileIn does not exist."
    exit 1
  fi
  DirOut="output_o2/$Index"
  mkdir -p $DirOut
  cd $DirOut
  cp "$JSON" $JSONLocal
  sed -e "s!@$LISTINPUT!$FileIn!g" $JSONLocal > $JSONLocal.tmp && mv $JSONLocal.tmp $JSONLocal
  cp "$DirBase/$SCRIPT" .
  sed -e "s!$DirBase!$PWD!g" $SCRIPT > $SCRIPT.tmp && mv $SCRIPT.tmp $SCRIPT
  echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo $FileOut >> $DirBase/$FilesToMerge
  echo "Output file: $FileOut"
  bash $SCRIPT > $LogFile 2>&1 &
  Index=$((Index+1))
  cd $DirBase
done < "$LISTINPUT"
CmdNRun="top -u $USER -n 1 -c | grep o2 | grep configuration | wc -l"
echo "Waiting for O2 to start..."
while [ $(eval $CmdNRun) -eq 0 ]; do continue; done
echo "Waiting for O2 to finish..."
while [ $(eval $CmdNRun) -gt 0 ]; do
  echo $(eval $CmdNRun)
  sleep 1
done
sleep 1
echo "Merging output files... (output file: $FILEOUT)"
hadd $FILEOUT @"$FilesToMerge" > $LogFile 2>&1
if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
rm -f $FilesToMerge

exit 0
