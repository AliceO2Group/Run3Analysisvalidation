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

[ -f "$LISTINPUT" ] || { echo "Error: File $LISTINPUT does not exist."; exit 1; }
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  [ -f "$FileIn" ] || { echo "Error: File $FileIn does not exist."; exit 1; }
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut
  cd $DirOut
  echo $FileIn > $ListInOne
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
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
parallel --halt soon,fail=100% < $ListRunScripts > $LogFile 2>&1 || \
{ echo -e "Error\nCheck $(realpath $LogFile)"; exit 1; }
rm -f $ListRunScripts

exit 0
