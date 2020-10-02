#!/bin/bash

bash clean.sh

CASE=4

DOCONVERT=1   # Convert AliESDs.root to AO2D.root.
DOQA=0        # Run the QA task with O2.
DORUN1=1      # Run the heavy-flavour tasks with AliPhysics.
DORUN3=1      # Run the heavy-flavour tasks with O2.
DOCOMPARE=1   # Compare AliPhysics and O2 output.

RUN5=0        # Use Run 5 input.
CONVSEP=1     # Convert ESD files separately.
PARALLELISE=0 # Parallelise O2 tasks.
TWOPRONGSEL=0 # Apply D0 selection cuts.
DEBUG=0       # Print out more information.

# Default settings
JSON="$PWD/dpl-config_std.json"
JSONRUN5="$PWD/dpl-config_run5.json"
ISMC=0
MASS=1.8
TRIGGERSTRINGRUN2=""
TRIGGERBITRUN3=-1
NMAX=-1

if [ $CASE -eq 0 ]; then
  INPUTDIR="../twikiinput"
  STRING="AliESDs_ppK0starToyMC.root"
  MASS=1.0
fi

if [ $CASE -eq 1 ]; then
  INPUTDIR="/mnt/temp/Run3data/data/LHC15o_246751/pass1"
  STRING="15000246751019.110/AliESDs.root"
  TRIGGERSTRINGRUN2="CV0L7-B-NOPF-CENT"
  TRIGGERBITRUN3=5 #FIXME
  NMAX=5
fi

if [ $CASE -eq 2 ]; then
  INPUTDIR="/data/Run3data/alice_sim_2015_LHC15k1a3_246391/246391"
  STRING="00*/AliESDs.root"
  ISMC=1
  NMAX=1
fi

if [ $CASE -eq 3 ]; then
  INPUTDIR="/data/Run3data/output"
  STRING="00*/AliESDs.root"
  MASS=1.0
fi

if [ $CASE -eq 4 ]; then
  INPUTDIR="/data/Run3data/alice_sim_2018_LHC18a4a2_cent/282099"
  STRING="001/AliESDs.root"
  ISMC=1
fi

if [ $CASE -eq 5 ]; then
  INPUTDIR="/mnt/temp/Run3data_Vit/LHC18a4a2_cent/282341"
  STRING="001/AliESDs.root"
  ISMC=1
fi

if [ $CASE -eq 6 ]; then
  INPUTDIR="/mnt/temp/Run3data_Vit/LHC17p_pass1_CENT_woSDD/282341"
  STRING="17000282099019.1001/AliESDs.root"
  ISMC=0
fi

#INPUTDIR="/data/Run3data/output" #K0* MC injected
#INPUTDIR="/data/Run3data/alice_sim_2018_LHC18a4a2_cent/282099" #D2H MC sample
#INPUTDIR="/data/Run3data/alice_sim_2015_LHC15k1a3_246391/246391" #HIJING MC PbPb

# Lists of input files
LISTFILESALI="list_ali.txt"
ls $INPUTDIR/$STRING > $LISTFILESALI || { echo "Error: Failed to make a list of input files."; exit 1; }
LISTFILESO2="listrun3.txt"
LISTFILESO2RUN5="listrun5.txt"

# Output files names
FILEOUTALI="Vertices2prong-ITS1.root"
FILEOUTO2="AnalysisResults.root"
FILEOUTQA="AnalysisResultsQA.root"

# Steering commands
ENVALI="alienv setenv AliPhysics/latest -c"
ENVO2="alienv setenv O2/latest -c"
ENVALIO2="alienv setenv AliPhysics/latest,O2/latest -c"
CMDROOT="root -b -q -l"

# Adjust settings for Run5.
O2INPUT=$LISTFILESO2
if [ $RUN5 -eq 1 ]; then
  echo -e "\nUsing Run 5 settings and O2 input"
  O2INPUT=$LISTFILESO2RUN5
  JSON="$JSONRUN5"
fi

# Enable D0 selection.
if [ $TWOPRONGSEL -eq 1 ]; then
  echo -e "\nUsing D0 selection cuts"
  JSONSEL="${JSON/.json/_sel.json}"
  cp "$JSON" "$JSONSEL"
  sed -e "s!\"d_selectionFlagD0\": \"0\"!\"d_selectionFlagD0\": \"1\"!g" "$JSONSEL" > "$JSONSEL.tmp" && mv "$JSONSEL.tmp" "$JSONSEL"
  sed -e "s!\"d_selectionFlagD0bar\": \"0\"!\"d_selectionFlagD0bar\": \"1\"!g" "$JSONSEL" > "$JSONSEL.tmp" && mv "$JSONSEL.tmp" "$JSONSEL"
  JSON="$JSONSEL"
fi

# Convert AliESDs.root to AO2D.root.
if [ $DOCONVERT -eq 1 ]; then
  [ -f "$LISTFILESALI" ] || { echo -e "\nConverting: Error: File $LISTFILESALI does not exist."; exit 1; }
  echo -e "\nConverting... ($(cat $LISTFILESALI | wc -l) files)"
  [ $ISMC -eq 1 ] && echo "Using MC mode"
  if [ $CONVSEP -eq 1 ]; then
    echo "Converting files separately"
    $ENVALI bash convert_batch.sh $LISTFILESALI $LISTFILESO2 $ISMC $DEBUG || exit 1 # Run the batch script in the ALI environment.
  else
    LOGFILE="log_convert.log"
    rm -f $LOGFILE
    echo "logfile: $LOGFILE"
    $ENVALI $CMDROOT "convertAO2D.C(\"$LISTFILESALI\", $ISMC, $NMAX)" > $LOGFILE 2>&1 || { echo "Error"; exit 1; }
    echo "$PWD/AO2D.root" > $LISTFILESO2
    rm -f $FILEOUTO2
  fi
fi

# Perform simple QA studies with O2.
[[ $DOQA -eq 1 && ISMC -eq 0 ]] && { echo -e "\nSkipping the QA task for non-MC input"; DOQA=0; } # Disable running the QA task for non-MC input.
if [ $DOQA -eq 1 ]; then
  #LOGFILE="log_o2_qa.log"
  [ -f "$O2INPUT" ] || { echo -e "\nQA task: Error: File $O2INPUT does not exist."; exit 1; }
  echo -e "\nRunning the QA task with O2... ($(cat $O2INPUT | wc -l) files)"
  rm -f $FILEOUTO2 $FILEOUTQA
  O2ARGS="--shm-segment-size 16000000000 --configuration json://$JSON"
  if [ $PARALLELISE -eq 1 ]; then
    NPROC=3
    echo "O2 parallelisation ON ($NPROC)"
    O2ARGS="$O2ARGS --pipeline qa-tracking-kine:$NPROC,qa-tracking-resolution:$NPROC"
  fi
  O2EXEC="o2-analysis-qatask $O2ARGS -b"
  O2SCRIPT="script_o2_qa.sh"
  cat << EOF > $O2SCRIPT # Create a temporary script with the full O2 commands.
#!/bin/bash
$O2EXEC
EOF
  #$ENVO2 bash $O2SCRIPT > $LOGFILE 2>&1 || { echo "Error"; exit 1; } # Run the script in the O2 environment.
  #grep WARN $LOGFILE | sort -u
  $ENVO2 bash o2_batch.sh $O2INPUT $JSON $O2SCRIPT $DEBUG || exit 1 # Run the batch script in the O2 environment.
  rm -f $O2SCRIPT
  mv $FILEOUTO2 $FILEOUTQA
  rm -rf output_o2_qa
  mv output_o2 output_o2_qa
  mv log_o2.log log_o2_qa.log
fi

# Run the heavy-flavour tasks with AliPhysics.
if [ $DORUN1 -eq 1 ]; then
  [ -f "$LISTFILESALI" ] || { echo -e "\nHF tasks ALI: Error: File $LISTFILESALI does not exist."; exit 1; }
  echo -e "\nRunning the HF tasks with AliPhysics... ($(cat $LISTFILESALI | wc -l) files)"
  #$ENVALI bash ali_batch.sh $LISTFILESALI $JSON $FILEOUTALI # Run the batch script in the ALI environment.
  $ENVALIO2 bash ali_batch.sh $LISTFILESALI $JSON $FILEOUTALI $DEBUG || exit 1 # Run the batch script in the ALI+O2 environment.
fi

# Run the heavy-flavour tasks with O2.
if [ $DORUN3 -eq 1 ]; then
  #LOGFILE="log_o2_hf.log"
  [ -f "$O2INPUT" ] || { echo -e "\nHF tasks O2: Error: File $O2INPUT does not exist."; exit 1; }
  echo -e "\nRunning the HF tasks with O2... ($(cat $O2INPUT | wc -l) files)"
  rm -f $FILEOUTO2
  # Option --configuration has priority over --aod-file.
#  O2ARGS="--shm-segment-size 16000000000 --configuration json://$PWD/dpl-config_std.json --aod-file $AOD3NAME"
  O2ARGS="--shm-segment-size 16000000000 --configuration json://$JSON"
  O2ARGS_SKIM="$O2ARGS"
  O2ARGS_CAND="$O2ARGS"
  O2ARGS_PIDTPC="$O2ARGS"
  O2ARGS_PIDTOF="$O2ARGS"
  O2ARGS_SEL="$O2ARGS"
  O2ARGS_TASK="$O2ARGS"
  if [ $PARALLELISE -eq 1 ]; then
    NPROC=3
    echo "O2 parallelisation ON ($NPROC)"
    O2ARGS_SKIM="$O2ARGS_SKIM --pipeline hf-produce-sel-track:$NPROC,hf-track-index-skims-creator:$NPROC"
    O2ARGS_CAND="$O2ARGS_CAND --pipeline hf-cand-creator-2prong:$NPROC,hf-cand-creator-2prong-expressions:$NPROC"
    O2ARGS_TASK="$O2ARGS_TASK --pipeline hf-task-d0:$NPROC"
  fi
  O2EXEC_SKIM="o2-analysis-hf-track-index-skims-creator $O2ARGS_SKIM"
  O2EXEC_CAND="o2-analysis-hf-candidate-creator-2prong $O2ARGS_CAND"
  O2EXEC_PIDTPC="o2-analysis-pid-tpc $O2ARGS_PIDTPC"
  O2EXEC_PIDTOF="o2-analysis-pid-tof $O2ARGS_PIDTOF"
  O2EXEC_SEL="o2-analysis-hf-d0-candidate-selector $O2ARGS_SEL"
  O2EXEC_TASK="o2-analysis-hf-task-d0 $O2ARGS_TASK"
  O2EXEC="$O2EXEC_SKIM | $O2EXEC_PIDTPC | $O2EXEC_PIDTOF | $O2EXEC_CAND | $O2EXEC_SEL | $O2EXEC_TASK -b"
  O2SCRIPT="script_o2_hf.sh"
  cat << EOF > $O2SCRIPT # Create a temporary script with the full O2 commands.
#!/bin/bash
$O2EXEC
EOF
  #$ENVO2 bash $O2SCRIPT > $LOGFILE 2>&1 || { echo "Error"; exit 1; } # Run the script in the O2 environment.
  #grep WARN $LOGFILE | sort -u
  $ENVO2 bash o2_batch.sh $O2INPUT $JSON $O2SCRIPT $DEBUG || exit 1 # Run the batch script in the O2 environment.
  rm -f $O2SCRIPT
  rm -rf output_o2_hf
  mv output_o2 output_o2_hf
  mv log_o2.log log_o2_hf.log
fi

if [ $TWOPRONGSEL -eq 1 ]; then
  rm "$JSONSEL"
fi

# Compare AliPhysics and O2 output.
if [ $DOCOMPARE -eq 1 ]; then
  LOGFILE="log_compare.log"
  echo -e "\nComparing... (logfile: $LOGFILE)"
  ok=1
  for file in "$FILEOUTALI" "$FILEOUTO2"; do
    [ -f "$file" ] || { echo "Error: File $file does not exist."; ok=0; }
  done
  [ $ok -ne 1 ] && exit 1
  $ENVALI $CMDROOT "Compare.C(\"$FILEOUTO2\",\"$FILEOUTALI\", $MASS)" > $LOGFILE 2>&1 || { echo "Error"; exit 1; }
fi

echo -e "\nDone"

rm -f $LISTFILESALI
rm -f $LISTFILESO2

exit 0
