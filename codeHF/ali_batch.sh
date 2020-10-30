#!/bin/bash

# Message formatting
function MsgStep { echo -e "\n\e[1;32m$@\e[0m"; }
function MsgWarn { echo -e "\e[1;36m$@\e[0m"; }
function MsgErr { echo -e "\e[1;31m$@\e[0m"; }

LISTINPUT="$1"
JSON="$2"
ISMC=$3
DEBUG=$4
FILEOUT="AnalysisResults.root"

LogFile="log_ali.log"
FilesToMerge="ListOutToMergeALI.txt"
DirBase="$PWD"
Index=0
ListRunScripts="$DirBase/ListRunScripts.txt"
DirOutMain="output_ali"

rm -f $ListRunScripts
rm -f $FilesToMerge
rm -f $FILEOUT
rm -rf $DirOutMain

[ -f "$LISTINPUT" ] || { MsgErr "Error: File $LISTINPUT does not exist."; exit 1; }
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  [ -f "$FileIn" ] || { MsgErr "Error: File $FileIn does not exist."; exit 1; }
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut
  cd $DirOut
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo "$FileOut" >> "$DirBase/$FilesToMerge"
  RUNSCRIPT="run.sh"
  cat << EOF > $RUNSCRIPT # Create a temporary script.
#!/bin/bash
cd "$DirBase/$DirOut"
root -b -q -l "$DirBase/RunHFTaskLocal.C(\"$FileIn\", \"$JSON\", $ISMC)" > $LogFile 2>&1
EOF
  echo "bash $(realpath $RUNSCRIPT)" >> "$ListRunScripts"
  ((Index+=1))
  cd $DirBase
done < "$LISTINPUT"

echo "Running AliPhysics jobs..."
parallel --halt soon,fail=100% < $ListRunScripts > $LogFile 2>&1 || \
{ MsgErr "Error\nCheck $(realpath $LogFile)"; exit 1; }
rm -f $ListRunScripts

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" >> $LogFile 2>&1 || \
{ MsgErr "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
rm -f $FilesToMerge

exit 0
