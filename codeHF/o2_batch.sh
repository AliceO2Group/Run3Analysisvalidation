#!/bin/bash

LISTINPUT="$1"
JSON="$2"
SCRIPT="$3"
DEBUG=$4
FILEOUT="AnalysisResults.root"

LogFile="log_o2.log"
FilesToMerge="ListOutToMergeO2.txt"
DirBase="$PWD"
Index=0
JSONLocal=$(basename $JSON)
ListRunScripts="$DirBase/ListRunScripts.txt"
DirOutMain="output_o2"

rm -f $ListRunScripts
rm -f $FilesToMerge
rm -f $FILEOUT
rm -rf $DirOutMain

[ -f "$LISTINPUT" ] || { echo "Error: File $LISTINPUT does not exist."; exit 1; }
echo "Output directory: $DirOutMain (logfiles: $LogFile)"
while read FileIn; do
  [ -f "$FileIn" ] || { echo "Error: File $FileIn does not exist."; exit 1; }
  DirOut="$DirOutMain/$Index"
  mkdir -p $DirOut
  cd $DirOut
  cp "$JSON" $JSONLocal
  sed -e "s!@$LISTINPUT!$FileIn!g" $JSONLocal > $JSONLocal.tmp && mv $JSONLocal.tmp $JSONLocal
  cp "$DirBase/$SCRIPT" .
  sed -e "s!$DirBase!$PWD!g" $SCRIPT > $SCRIPT.tmp && mv $SCRIPT.tmp $SCRIPT
  [ $DEBUG -eq 1 ] && echo "Input file ($Index): $FileIn"
  FileOut="$DirOut/$FILEOUT"
  echo $FileOut >> "$DirBase/$FilesToMerge"
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
  echo "bash $(realpath $RUNSCRIPT)" >> "$ListRunScripts"
  ((Index+=1))
  cd $DirBase
done < "$LISTINPUT"

echo "Running O2 jobs..."
parallel --halt soon,fail=100% < $ListRunScripts > $LogFile 2>&1
ExitCode=$?
find /tmp -group $USER -name "localhost*_*" -delete 2> /dev/null # Delete all user's sockets.
[ $ExitCode -ne 0 ] && { echo -e "Error\nCheck $(realpath $LogFile)"; exit 1; }
[ "$(grep WARN "$LogFile")" ] && echo -e "There were warnings!\nCheck $(realpath $LogFile)"
rm -f $ListRunScripts

echo "Merging output files... (output file: $FILEOUT, logfile: $LogFile)"
hadd $FILEOUT @"$FilesToMerge" >> $LogFile 2>&1 || \
{ echo -e "Error\nCheck $(realpath $LogFile)"; tail -n 2 "$LogFile"; exit 1; }
rm -f $FilesToMerge

exit 0
