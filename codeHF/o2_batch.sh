#!/bin/bash

# Load message formatting.
source messages.sh || { MsgErr "Error: Failed to load message formatting."; exit 1; }

LISTINPUT="$1"
JSON="$2"
SCRIPT="$3"
DEBUG=$4
FILEOUT_TREE="$5"
FILEOUT="AnalysisResults.root"

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
rm -rf $DirOutMain || { MsgErr "Error"; exit 1; }

[ -f "$LISTINPUT" ] || { MsgErr "Error: File $LISTINPUT does not exist."; exit 1; }
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  [ -f "$FileIn" ] || { MsgErr "Error: File $FileIn does not exist."; exit 1; }
  FileIn="$(realpath $FileIn)"
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut && \
  cd $DirOut && \
  cp "$JSON" $JSONLocal && \
  sed -e "s!@$LISTINPUT!$FileIn!g" $JSONLocal > $JSONLocal.tmp && mv $JSONLocal.tmp $JSONLocal && \
  cp "$DirBase/$SCRIPT" . && \
  sed -e "s!$DirBase!$PWD!g" $SCRIPT > $SCRIPT.tmp && mv $SCRIPT.tmp $SCRIPT || { MsgErr "Error"; exit 1; }
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo $FileOut >> "$DirBase/$FilesToMerge" || { MsgErr "Error"; exit 1; }
  [ "$FILEOUT_TREE" ] && {
    FileOutTree="$DirOut/$FILEOUT_TREE"
    echo $FileOutTree >> "$DirBase/$FilesToMergeTree" || { MsgErr "Error"; exit 1; }
  }
  RUNSCRIPT="run.sh"
  cat << EOF > $RUNSCRIPT # Create a temporary script.
#!/bin/bash
cd "$DirBase/$DirOut"
bash $SCRIPT > $LogFile 2>&1
ExitCode=\$?
grep WARN "$LogFile" | sort -u
pid=\$(tail -n 2 "$LogFile" | grep "is exiting" | cut -d " " -f 3) # Get the process ID from the O2 log.
find /tmp -group \$USER -name "localhost\${pid}_*" -delete 2> /dev/null # Delete the process sockets.
exit \$ExitCode
EOF
  echo "bash $(realpath $RUNSCRIPT)" >> "$ListRunScripts" && \
  ((Index+=1)) && \
  cd $DirBase || { MsgErr "Error"; exit 1; }
done < "$LISTINPUT"

echo "Running O2 jobs..."
parallel --halt soon,fail=100% < $ListRunScripts > $LogFile 2>&1
ExitCode=$?
find /tmp -group $USER -name "localhost*_*" -delete 2> /dev/null # Delete all user's sockets.
[ $ExitCode -ne 0 ] && { MsgErr "Error\nCheck $(realpath $LogFile)"; exit 1; }
[ "$(grep WARN "$LogFile")" ] && MsgWarn "There were warnings!\nCheck $(realpath $LogFile)"
rm -f $ListRunScripts || { MsgErr "Error"; exit 1; }

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" >> $LogFile 2>&1 || \
{ MsgErr "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
rm -f $FilesToMerge || { MsgErr "Error"; exit 1; }

[ "$FILEOUT_TREE" ] && {
  echo "Merging output trees... (output file: $FILEOUT_TREE, logfile: $LogFile)"
  hadd $FILEOUT_TREE @"$FilesToMergeTree" >> $LogFile 2>&1 || \
  { MsgErr "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
  rm -f $FilesToMergeTree || { MsgErr "Error"; exit 1; }
}

exit 0
