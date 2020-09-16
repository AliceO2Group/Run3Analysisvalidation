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
DirOutMain="output_o2"
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  if [ ! -f "$FileIn" ]; then
    echo "Error: File $FileIn does not exist."
    exit 1
  fi
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut
  cd $DirOut
  cp "$JSON" $JSONLocal
  sed -e "s!@$LISTINPUT!$FileIn!g" $JSONLocal > $JSONLocal.tmp && mv $JSONLocal.tmp $JSONLocal
  cp "$DirBase/$SCRIPT" .
  sed -e "s!$DirBase!$PWD!g" $SCRIPT > $SCRIPT.tmp && mv $SCRIPT.tmp $SCRIPT
  echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo $FileOut >> $DirBase/$FilesToMerge
  #echo "Output file: $FileOut"
  bash $SCRIPT > $LogFile 2>&1 &
  PIDS+=($!)
  ((Index+=1))
  cd $DirBase
done < "$LISTINPUT"

echo "Waiting for O2 to finish..."
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
