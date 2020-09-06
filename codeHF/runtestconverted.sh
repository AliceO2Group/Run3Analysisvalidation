#!/bin/bash

#bash clean.sh

CASE=0

DORUN3=1 # Run the tasks with O2.

if [ $CASE -eq 0 ]; then
  ISMC=0
  AOD3NAME=AO2D.root
  MASS=1.0
  TRIGGERBITRUN3=-1
  JSON=dpl-config_std.json
fi

# Output files
FILEOUTO2="AnalysisResults.root"
# Steering commands
ENVALI="alienv setenv AliPhysics/latest -c"
ENVO2="alienv setenv O2/latest -c"
CMDROOT="root -b -q -l"

# Run the tasks with O2.
if [ $DORUN3 -eq 1 ]; then
  LOGFILE="log_o2.log"
  echo -e "\nRunning the tasks with O2... (logfile: $LOGFILE)"
  rm -f $FILEOUTO2
  O2ARGS="--shm-segment-size 16000000000 --configuration json://$PWD/dpl-config_std.json --aod-file $AOD3NAME"
  O2EXEC="o2-analysis-hf-track-index-skims-creator $O2ARGS | o2-analysis-hf-candidate-creator-2prong $O2ARGS | o2-analysis-hf-task-d0 $O2ARGS -b"
  TMPSCRIPT="tmpscript.sh"
  cat << EOF > $TMPSCRIPT # Create a temporary script with the full O2 commands.
#!/bin/bash
$O2EXEC
EOF
  $ENVO2 bash $TMPSCRIPT > $LOGFILE 2>&1 # Run the script in the O2 environment.
  if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
  rm -f $TMPSCRIPT
fi


echo -e "\nDone"
exit 0

