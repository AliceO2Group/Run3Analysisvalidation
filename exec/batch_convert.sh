#!/bin/bash

# Script to run Run 2 to Run 3 conversion in jobs

LISTINPUT="$1"
LISTOUTPUT="$2"
ISMC=$3
DEBUG=$4
FILEOUT="AO2D.root"

[ $DEBUG -eq 1 ] && echo "Running $0"

# This directory
DIR_THIS="$(dirname $(realpath $0))"

# Load utilities.
source "$DIR_THIS/utilities.sh" || { echo "Error: Failed to load utilities."; exit 1; }

LogFile="log_convert.log"
ListInOne="list_input.txt"
DirBase="$PWD"
Index=0
ListRunScripts="$DirBase/ListRunScripts.txt"
DirOutMain="output_conversion"

rm -f $ListRunScripts && \
rm -f $LISTOUTPUT && \
rm -rf $DirOutMain || ErrExit "Failed to delete output files."

CheckFile "$LISTINPUT"
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  CheckFile "$FileIn"
  FileIn="$(realpath $FileIn)"
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut && \
  cd $DirOut && \
  echo $FileIn > "$ListInOne" || ErrExit "Failed to echo to $ListInOne."
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo "$DirBase/$FileOut" >> "$DirBase/$LISTOUTPUT" || ErrExit "Failed to echo to $DirBase/$LISTOUTPUT."
  RUNSCRIPT="run.sh"
  cat << EOF > $RUNSCRIPT # Create the job script.
#!/bin/bash
DirThis="\$(dirname \$(realpath \$0))"
cd "\$DirThis"
root -b -q -l "$DIR_THIS/convertAO2D.C(\"$ListInOne\", $ISMC)" > $LogFile 2>&1
EOF
  echo "bash $(realpath $RUNSCRIPT)" >> "$ListRunScripts" && \
  ((Index+=1)) && \
  cd $DirBase || ErrExit "Failed to cd $DirBase."
done < "$LISTINPUT"

echo "Running conversion jobs..."
OPT_PARALLEL="--halt soon,fail=100%"
if [ $DEBUG -eq 0 ]; then
  parallel $OPT_PARALLEL < $ListRunScripts > $LogFile 2>&1
else
  parallel $OPT_PARALLEL --will-cite --progress < $ListRunScripts > $LogFile
fi
[ $? -ne 0 ] && ErrExit "\nCheck $(realpath $LogFile)"
rm -f $ListRunScripts || ErrExit "Failed to rm $ListRunScripts."

exit 0
