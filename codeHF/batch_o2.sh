#!/bin/bash

# Script to run O2 tasks in jobs.

LISTINPUT="$1"
JSON="$2"
SCRIPT="$3"
DEBUG=$4
FILEOUT_TREE="$5"
FILEOUT="AnalysisResults.root"

[ $DEBUG -eq 1 ] && echo "Running $0"

# Load utilities.
source utilities.sh || ErrExit "Failed to load utilities."

CheckFile "$SCRIPT"
CheckFile "$JSON"
SCRIPT="$(realpath $SCRIPT)"
JSON="$(realpath $JSON)"

LogFile="log_o2.log"
FilesToMerge="ListOutToMergeO2.txt"
FilesToMergeTree="ListOutToMergeO2Tree.txt"
DirBase="$PWD"
Index=0
JSONLocal=$(basename $JSON)
ListRunScripts="$DirBase/ListRunScripts.txt"
DirOutMain="output_o2"

rm -f $ListRunScripts && \
rm -f $FilesToMerge && \
rm -f $FilesToMergeTree && \
rm -f $FILEOUT && \
rm -f $FILEOUT_TREE && \
rm -rf $DirOutMain || ErrExit

CheckFile "$LISTINPUT"
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  CheckFile "$FileIn"
  FileIn="$(realpath $FileIn)"
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut && \
  cd $DirOut && \
  cp "$JSON" $JSONLocal && \
  sed -e "s!@$LISTINPUT!$FileIn!g" $JSONLocal > $JSONLocal.tmp && mv $JSONLocal.tmp $JSONLocal || ErrExit
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo $FileOut >> "$DirBase/$FilesToMerge" || ErrExit
  [ "$FILEOUT_TREE" ] && {
    FileOutTree="$DirOut/$FILEOUT_TREE"
    echo $FileOutTree >> "$DirBase/$FilesToMergeTree" || ErrExit
  }
  RUNSCRIPT="run.sh"
  cat << EOF > $RUNSCRIPT # Create the job script.
#!/bin/bash
cd "$DirBase/$DirOut"
bash $SCRIPT "$DirBase/$DirOut/$JSONLocal" > $LogFile 2>&1
ExitCode=\$?
grep WARN "$LogFile" | sort -u
pid=\$(tail -n 2 "$LogFile" | grep "is exiting" | cut -d " " -f 3) # Get the process ID from the O2 log.
find /tmp -group \$USER -name "localhost\${pid}_*" -delete 2> /dev/null # Delete the process sockets.
exit \$ExitCode
EOF
  echo "bash $(realpath $RUNSCRIPT)" >> "$ListRunScripts" && \
  ((Index+=1)) && \
  cd $DirBase || ErrExit
done < "$LISTINPUT"

echo "Running O2 jobs..."
parallel --halt soon,fail=100% < $ListRunScripts > $LogFile 2>&1
ExitCode=$?
find /tmp -group $USER -name "localhost*_*" -delete 2> /dev/null # Delete all user's sockets.
[ $ExitCode -ne 0 ] && ErrExit "\nCheck $(realpath $LogFile)"
[ "$(grep WARN "$LogFile")" ] && MsgWarn "There were warnings!\nCheck $(realpath $LogFile)"
rm -f $ListRunScripts || ErrExit

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" >> $LogFile 2>&1 || \
{ MsgErr "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
rm -f $FilesToMerge || ErrExit

[ "$FILEOUT_TREE" ] && {
  echo "Merging output trees... (output file: $FILEOUT_TREE, logfile: $LogFile)"
  hadd $FILEOUT_TREE @"$FilesToMergeTree" >> $LogFile 2>&1 || \
  { MsgErr "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
  rm -f $FilesToMergeTree || ErrExit
}

exit 0
