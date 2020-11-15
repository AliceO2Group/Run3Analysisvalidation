#!/bin/bash

# Load message formatting.
source messages.sh || { MsgErr "Error: Failed to load message formatting."; exit 1; }

LISTINPUT="$1"
LISTOUTPUT="$2"
ISMC=$3
DEBUG=$4
FILEOUT="AO2D.root"

LogFile="log_convert.log"
ListInOne="list_input.txt"
DirBase="$PWD"
Index=0
ListRunScripts="$DirBase/ListRunScripts.txt"
DirOutMain="output_conversion"

rm -f $ListRunScripts && \
rm -f $LISTOUTPUT && \
rm -rf $DirOutMain || { MsgErr "Error"; exit 1; }

[ -f "$LISTINPUT" ] || { MsgErr "Error: File $LISTINPUT does not exist."; exit 1; }
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  [ -f "$FileIn" ] || { MsgErr "Error: File $FileIn does not exist."; exit 1; }
  FileIn="$(realpath $FileIn)"
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut && \
  cd $DirOut && \
  echo $FileIn > $ListInOne || { MsgErr "Error"; exit 1; }
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo "$DirBase/$FileOut" >> $DirBase/$LISTOUTPUT || { MsgErr "Error"; exit 1; }
  RUNSCRIPT="run.sh"
  cat << EOF > $RUNSCRIPT # Create the job script.
#!/bin/bash
cd "$DirBase/$DirOut"
root -b -q -l "$DirBase/convertAO2D.C(\"$ListInOne\", $ISMC)" > $LogFile 2>&1
EOF
  echo "bash $(realpath $RUNSCRIPT)" >> "$ListRunScripts" && \
  ((Index+=1)) && \
  cd $DirBase || { MsgErr "Error"; exit 1; }
done < "$LISTINPUT"

echo "Running conversion jobs..."
parallel --halt soon,fail=100% < $ListRunScripts > $LogFile 2>&1 || \
{ MsgErr "Error\nCheck $(realpath $LogFile)"; exit 1; }
rm -f $ListRunScripts || { MsgErr "Error"; exit 1; }

exit 0
