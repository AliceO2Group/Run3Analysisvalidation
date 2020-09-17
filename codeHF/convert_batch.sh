#!/bin/bash

LISTINPUT="$1"
LISTOUTPUT="$2"
ISMC=$3

LogFile="log_convert.log"
ListInOne="list_input.txt"
DirBase=$(pwd)
Index=0
ListRunScripts="$DirBase/ListRunScripts.txt"

rm -f $ListRunScripts
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
parallel -j0 --halt now,fail=1 < $ListRunScripts > $LogFile 2>&1
if [ $? -ne 0 ]; then echo -e "Error\nCheck $(realpath $LogFile)"; exit 1; fi # Exit if error.
rm -f $ListRunScripts

exit 0
