#!/bin/bash

LISTINPUT="$1"
LISTOUTPUT="$2"
ISMC=$3

LogFile="log_convert.log"
ListInOne="list_input.txt"
DirBase=$(pwd)
Index=0
while read FileIn; do
  if [ ! -f "$FileIn" ]; then
    echo "Error: Fle $FileIn does not exist."
    exit 1
  fi
  DirOut="output_conversion/$Index"
  mkdir -p $DirOut
  cd $DirOut
  echo $FileIn > $ListInOne
  echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/AO2D.root"
  echo $FileOut >> $DirBase/$LISTOUTPUT
  echo "Output file: $FileOut"
  root -b -q -l "$DirBase/convertAO2D.C(\"$ListInOne\", $ISMC, -1)" > $LogFile 2>&1 &
  Index=$((Index+1))
  cd $DirBase
done < "$LISTINPUT"

exit 0
