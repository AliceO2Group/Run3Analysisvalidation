#!/bin/bash

# Script to run Run 2 to Run 3 conversion in jobs.

LISTINPUT="$1"
LISTOUTPUT="$2"
ISMC=$3
DEBUG=$4
FILEOUT="AO2D.root"

[ $DEBUG -eq 1 ] && echo "Running $0"

# Load utilities.
source utilities.sh || ErrExit "Failed to load utilities."

LogFile="log_convert.log"
ListInOne="list_input.txt"
DirBase="$PWD"
Index=0
ListRunScripts="$DirBase/ListRunScripts.txt"
DirOutMain="output_conversion"

rm -f $ListRunScripts && \
rm -f $LISTOUTPUT && \
rm -rf $DirOutMain || ErrExit

CheckFile "$LISTINPUT"
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  CheckFile "$FileIn"
  FileIn="$(realpath $FileIn)"
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut && \
  cd $DirOut && \
  echo $FileIn > $ListInOne || ErrExit
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo "$DirBase/$FileOut" >> $DirBase/$LISTOUTPUT || ErrExit
  RUNSCRIPT="run.sh"
  cat << EOF > $RUNSCRIPT # Create the job script.
#!/bin/bash
cd "$DirBase/$DirOut"
root -b -q -l "$DirBase/convertAO2D.C(\"$ListInOne\", $ISMC)" > $LogFile 2>&1
EOF
  echo "bash $(realpath $RUNSCRIPT)" >> "$ListRunScripts" && \
  ((Index+=1)) && \
  cd $DirBase || ErrExit
done < "$LISTINPUT"

echo "Running conversion jobs..."
parallel --halt soon,fail=100% < $ListRunScripts > $LogFile 2>&1 || \
ErrExit "\nCheck $(realpath $LogFile)"
rm -f $ListRunScripts || ErrExit

exit 0
