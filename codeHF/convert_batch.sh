#!/bin/bash

LISTINPUT="$1"
LISTOUTPUT="$2"
ISMC=$3
DEBUG=$4

LogFile="log_convert.log"
ListInOne="list_input.txt"
DirBase="$PWD"
Index=0
ListRunScripts="$DirBase/ListRunScripts.txt"
DirOutMain="output_conversion"

rm -f $ListRunScripts
rm -f $LISTOUTPUT
rm -rf $DirOutMain

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
  if [ $DEBUG -eq 1 ]; then
    echo "Input file ($Index): $FileIn"
  fi
  FileOut="$DirOut/AO2D.root"
  echo "$DirBase/$FileOut" >> $DirBase/$LISTOUTPUT
  #root -b -q -l "$DirBase/convertAO2D.C(\"$ListInOne\", $ISMC)" > $LogFile 2>&1
  RUNSCRIPT="run.sh"
  cat << EOF > $RUNSCRIPT # Create a temporary script.
#!/bin/bash
cd "$DirBase/$DirOut"
root -b -q -l "$DirBase/convertAO2D.C(\"$ListInOne\", $ISMC)" > $LogFile 2>&1
EOF
  echo "bash $(realpath $RUNSCRIPT)" >> "$ListRunScripts"
  ((Index+=1))
  cd $DirBase
done < "$LISTINPUT"

echo "Running conversion jobs..."
parallel --halt soon,fail=100% < $ListRunScripts > $LogFile 2>&1
if [ $? -ne 0 ]; then echo -e "Error\nCheck $(realpath $LogFile)"; exit 1; fi # Exit if error.
rm -f $ListRunScripts

exit 0
