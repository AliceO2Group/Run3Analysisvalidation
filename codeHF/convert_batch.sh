#!/bin/bash

LISTINPUT="$1"
LISTOUTPUT="$2"
ISMC=$3

LogFile="log_convert.log"
ListInOne="list_input.txt"
DirBase=$(pwd)
Index=0

rm -f $LISTOUTPUT
DirOutMain="output_conversion"
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  if [ ! -f "$FileIn" ]; then
    echo "Error: File $FileIn does not exist."
    exit 1
  fi
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut
  cd $DirOut
  echo $FileIn > $ListInOne
  echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/AO2D.root"
  echo "$DirBase/$FileOut" >> $DirBase/$LISTOUTPUT
  #echo "Output file: $FileOut"
  root -b -q -l "$DirBase/convertAO2D.C(\"$ListInOne\", $ISMC, -1)" > $LogFile 2>&1 &
  PIDS+=($!)
  ((Index+=1))
  cd $DirBase
done < "$LISTINPUT"

echo "Waiting for conversion to finish..."
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

exit 0
