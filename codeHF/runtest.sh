#!/bin/bash

bash clean.sh

CASE=4

DOCONVERT=1   # Convert AliESDs.root to AO2D.root.
DOQA=0        # Run the QA task with O2.
DORUN1=1      # Run the heavy-flavour tasks with AliPhysics.
DORUN3=1      # Run the heavy-flavour tasks with O2.
DOCOMPARE=1   # Compare AliPhysics and O2 output.

RUN5=0        # Use Run 5 input.
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
fi

# Message formatting
function MsgStep { echo -e "\n\e[1;32m$@\e[0m"; }
function MsgWarn { echo -e "\e[1;36m$@\e[0m"; }
function MsgErr { echo -e "\e[1;31m$@\e[0m"; }

# Lists of input files
LISTFILES_ALI="list_ali.txt"
ls $INPUTDIR/$STRING > $LISTFILES_ALI || { MsgErr "Error: Failed to make a list of input files."; exit 1; }
LISTFILES_O2="listrun3.txt"
LISTFILES_O2_RUN5="listrun5.txt"

# Output files names
FILEOUT="AnalysisResults.root"
FILEOUT_ALI_HF="AnalysisResults_ALI_HF.root"
FILEOUT_O2_HF="AnalysisResults_O2_HF.root"
FILEOUT_O2_QA="AnalysisResults_O2_QA.root"

# Steering commands
ENVALI="alienv setenv AliPhysics/latest -c"
ENVO2="alienv setenv O2/latest -c"
ENVALIO2="alienv setenv AliPhysics/latest,O2/latest -c"
CMDROOT="root -b -q -l"

# Adjust settings for Run5.
O2INPUT=$LISTFILES_O2
if [ $RUN5 -eq 1 ]; then
  MsgWarn "\nUsing Run 5 settings and O2 input"
  O2INPUT=$LISTFILES_O2_RUN5
  JSON="$JSONRUN5"
fi

# Enable D0 selection.
if [ $TWOPRONGSEL -eq 1 ]; then
  MsgWarn "\nUsing D0 selection cuts"
  JSONSEL="${JSON/.json/_sel.json}"
  cp "$JSON" "$JSONSEL"
  sed -e "s!\"d_selectionFlagD0\": \"0\"!\"d_selectionFlagD0\": \"1\"!g" "$JSONSEL" > "$JSONSEL.tmp" && mv "$JSONSEL.tmp" "$JSONSEL"
  sed -e "s!\"d_selectionFlagD0bar\": \"0\"!\"d_selectionFlagD0bar\": \"1\"!g" "$JSONSEL" > "$JSONSEL.tmp" && mv "$JSONSEL.tmp" "$JSONSEL"
  JSON="$JSONSEL"
fi

# Convert AliESDs.root to AO2D.root.
if [ $DOCONVERT -eq 1 ]; then
  [ -f "$LISTFILES_ALI" ] || { MsgErr "\nConverting: Error: File $LISTFILES_ALI does not exist."; exit 1; }
  MsgStep "Converting... ($(cat $LISTFILES_ALI | wc -l) files)"
  [ $ISMC -eq 1 ] && MsgWarn "Using MC mode"
  $ENVALI bash convert_batch.sh $LISTFILES_ALI $LISTFILES_O2 $ISMC $DEBUG || exit 1 # Run the batch script in the ALI environment.
fi

# Perform simple QA studies with O2.
[[ $DOQA -eq 1 && ISMC -eq 0 ]] && { MsgWarn "\nSkipping the QA task for non-MC input"; DOQA=0; } # Disable running the QA task for non-MC input.
if [ $DOQA -eq 1 ]; then
  [ -f "$O2INPUT" ] || { MsgErr "\nQA task: Error: File $O2INPUT does not exist."; exit 1; }
  MsgStep "Running the QA task with O2... ($(cat $O2INPUT | wc -l) files)"
  rm -f $FILEOUT $FILEOUT_O2_QA
  O2ARGS="--shm-segment-size 16000000000 --configuration json://$JSON"
  if [ $PARALLELISE -eq 1 ]; then
    NPROC=3
    MsgWarn "O2 parallelisation ON ($NPROC)"
    O2ARGS="$O2ARGS --pipeline qa-tracking-kine:$NPROC,qa-tracking-resolution:$NPROC"
  fi
  O2EXEC="o2-analysis-qatask $O2ARGS -b"
  O2SCRIPT="script_o2_qa.sh"
  cat << EOF > $O2SCRIPT # Create a temporary script with the full O2 commands.
#!/bin/bash
$O2EXEC
EOF
  $ENVO2 bash o2_batch.sh $O2INPUT $JSON $O2SCRIPT $DEBUG || exit 1 # Run the batch script in the O2 environment.
  rm -f $O2SCRIPT
  rm -rf output_o2_qa
  mv $FILEOUT $FILEOUT_O2_QA
  mv output_o2 output_o2_qa
  mv log_o2.log log_o2_qa.log
fi

# Run the heavy-flavour tasks with AliPhysics.
if [ $DORUN1 -eq 1 ]; then
  [ -f "$LISTFILES_ALI" ] || { MsgErr "\nHF tasks ALI: Error: File $LISTFILES_ALI does not exist."; exit 1; }
  MsgStep "Running the HF tasks with AliPhysics... ($(cat $LISTFILES_ALI | wc -l) files)"
  rm -f $FILEOUT $FILEOUT_ALI_HF
  $ENVALI bash ali_batch.sh $LISTFILES_ALI $JSON $ISMC $DEBUG || exit 1 # Run the batch script in the ALI environment.
  #$ENVALIO2 bash ali_batch.sh $LISTFILES_ALI $JSON $ISMC $DEBUG || exit 1 # Run the batch script in the ALI+O2 environment.
  mv $FILEOUT $FILEOUT_ALI_HF
fi

# Run the heavy-flavour tasks with O2.
if [ $DORUN3 -eq 1 ]; then
  [ -f "$O2INPUT" ] || { MsgErr "\nHF tasks O2: Error: File $O2INPUT does not exist."; exit 1; }
  MsgStep "Running the HF tasks with O2... ($(cat $O2INPUT | wc -l) files)"
  rm -f $FILEOUT $FILEOUT_O2_HF
  O2ARGS="--shm-segment-size 16000000000 --configuration json://$JSON"
  O2ARGS_SKIM="$O2ARGS"
  O2ARGS_CAND_2PRONG="$O2ARGS"
  O2ARGS_CAND_3PRONG="$O2ARGS"
  O2ARGS_PID_TPC="$O2ARGS"
  O2ARGS_PID_TOF="$O2ARGS"
  O2ARGS_SEL_D0="$O2ARGS"
  O2ARGS_TASK_D0="$O2ARGS"
  O2ARGS_TASK_DPLUS="$O2ARGS"
  if [ $PARALLELISE -eq 1 ]; then
    NPROC=3
    MsgWarn "O2 parallelisation ON ($NPROC)"
    O2ARGS_SKIM="$O2ARGS_SKIM --pipeline hf-produce-sel-track:$NPROC,hf-track-index-skims-creator:$NPROC"
    O2ARGS_CAND_2PRONG="$O2ARGS_CAND_2PRONG --pipeline hf-cand-creator-2prong:$NPROC,hf-cand-creator-2prong-expressions:$NPROC"
    O2ARGS_CAND_3PRONG="$O2ARGS_CAND_3PRONG --pipeline hf-cand-creator-3prong:$NPROC,hf-cand-creator-3prong-expressions:$NPROC"
    O2ARGS_TASK_D0="$O2ARGS_TASK_D0 --pipeline hf-task-d0:$NPROC"
    O2ARGS_TASK_DPLUS="$O2ARGS_TASK_DPLUS --pipeline hf-task-dplus:$NPROC"
  fi
  O2EXEC_SKIM="o2-analysis-hf-track-index-skims-creator $O2ARGS_SKIM"
  O2EXEC_CAND_2PRONG="o2-analysis-hf-candidate-creator-2prong $O2ARGS_CAND_2PRONG"
  O2EXEC_CAND_3PRONG="o2-analysis-hf-candidate-creator-3prong $O2ARGS_CAND_3PRONG"
  O2EXEC_PID_TPC="o2-analysis-pid-tpc $O2ARGS_PID_TPC"
  O2EXEC_PID_TOF="o2-analysis-pid-tof $O2ARGS_PID_TOF"
  O2EXEC_SEL_D0="o2-analysis-hf-d0-candidate-selector $O2ARGS_SEL_D0"
  O2EXEC_TASK_D0="o2-analysis-hf-task-d0 $O2ARGS_TASK_D0"
  O2EXEC_TASK_DPLUS="o2-analysis-hf-task-dplus $O2ARGS_TASK_DPLUS"
  O2EXEC="$O2EXEC_SKIM | $O2EXEC_PID_TPC | $O2EXEC_PID_TOF | $O2EXEC_CAND_2PRONG | $O2EXEC_CAND_3PRONG | $O2EXEC_SEL_D0 | $O2EXEC_TASK_D0 | $O2EXEC_TASK_DPLUS -b"
  O2SCRIPT="script_o2_hf.sh"
  cat << EOF > $O2SCRIPT # Create a temporary script with the full O2 commands.
#!/bin/bash
$O2EXEC
EOF
  $ENVO2 bash o2_batch.sh $O2INPUT $JSON $O2SCRIPT $DEBUG || exit 1 # Run the batch script in the O2 environment.
  rm -f $O2SCRIPT
  rm -rf output_o2_hf
  mv $FILEOUT $FILEOUT_O2_HF
  mv output_o2 output_o2_hf
  mv log_o2.log log_o2_hf.log
fi

if [ $TWOPRONGSEL -eq 1 ]; then
  rm "$JSONSEL"
fi

# Compare AliPhysics and O2 output.
if [ $DOCOMPARE -eq 1 ]; then
  LOGFILE="log_compare.log"
  MsgStep "Comparing... (logfile: $LOGFILE)"
  ok=1
  for file in "$FILEOUT_ALI_HF" "$FILEOUT_O2_HF"; do
    [ -f "$file" ] || { MsgErr "Error: File $file does not exist."; ok=0; }
  done
  [ $ok -ne 1 ] && exit 1
  $ENVALI $CMDROOT "Compare.C(\"$FILEOUT_O2_HF\", \"$FILEOUT_ALI_HF\", $MASS)" > $LOGFILE 2>&1 || { MsgErr "Error"; exit 1; }
fi

MsgStep "Done"

rm -f $LISTFILES_ALI
rm -f $LISTFILES_O2

exit 0
