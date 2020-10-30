#!/bin/bash

##############################################################################
# User settings

# Steps
DOCLEAN=1           # Delete created files (before and after running tasks).
DOCONVERT=1         # Convert AliESDs.root to AO2D.root.
DOALI=1             # Run AliPhysics tasks.
DOO2=1              # Run O2 tasks.
DOCOMPARE=1         # Compare AliPhysics and O2 output.

# O2 tasks
# (Dependencies are handled automatically.)
DOO2_QA=0           # qatask
DOO2_SKIM=0         # hf-track-index-skims-creator
DOO2_CAND_2PRONG=0  # hf-candidate-creator-2prong
DOO2_CAND_3PRONG=0  # hf-candidate-creator-3prong
DOO2_PID_TPC=0      # pid-tpc
DOO2_PID_TOF=0      # pid-tof
DOO2_SEL_D0=0       # hf-d0-candidate-selector
DOO2_TASK_D0=1      # hf-task-d0
DOO2_TASK_DPLUS=1   # hf-task-dplus

INPUT_TYPE=4        # Input type (Run 3) (See the choices below.)
RUN5=0              # Use Run 5 settings and input.
SAVETREES=0         # Save O2 tables to trees.
PARALLELISE=0       # Parallelise O2 tasks. (not working!)
TWOPRONGSEL=0       # Apply D0 selection cuts.
DEBUG=0             # Print out more information.
##############################################################################

# Default settings
JSON="$PWD/dpl-config_std.json"       # Run 3 configuration
JSONRUN5="$PWD/dpl-config_run5.json"  # Run 5 configuration
ISMC=0                                # Switch for MC input
MASS=1.8                              # Hadron mass (only for comparison plots, not used)
TRIGGERSTRINGRUN2=""                  # Run 2 trigger (not used)
TRIGGERBITRUN3=-1                     # Run 3 trigger (not used)

# Input specification
if [ $INPUT_TYPE -eq 0 ]; then
  INPUTDIR="../twikiinput"
  STRING="AliESDs_ppK0starToyMC.root"
  MASS=1.0
fi

if [ $INPUT_TYPE -eq 1 ]; then # Pb-Pb real LHC15o
  INPUTDIR="/mnt/temp/Run3data/data/LHC15o_246751/pass1"
  STRING="15000246751019.110/AliESDs.root"
  TRIGGERSTRINGRUN2="CV0L7-B-NOPF-CENT"
  TRIGGERBITRUN3=5 #FIXME
fi

if [ $INPUT_TYPE -eq 2 ]; then # Pb-Pb MC LHC15o
  INPUTDIR="/data/Run3data/alice_sim_2015_LHC15k1a3_246391/246391"
  STRING="00*/AliESDs.root"
  ISMC=1
fi

if [ $INPUT_TYPE -eq 3 ]; then
  INPUTDIR="/data/Run3data/output"
  STRING="00*/AliESDs.root"
  MASS=1.0
fi

if [ $INPUT_TYPE -eq 4 ]; then # p-p MC LHC17p
  INPUTDIR="/data/Run3data/alice_sim_2018_LHC18a4a2_cent/282099"
  STRING="001/AliESDs.root"
  ISMC=1
fi

if [ $INPUT_TYPE -eq 5 ]; then # p-p MC LHC17p
  INPUTDIR="/mnt/temp/Run3data_Vit/LHC18a4a2_cent/282341"
  STRING="001/AliESDs.root"
  ISMC=1
fi

if [ $INPUT_TYPE -eq 6 ]; then # p-p real LHC17p
  INPUTDIR="/mnt/temp/Run3data_Vit/LHC17p_pass1_CENT_woSDD/282341"
  STRING="17000282099019.1001/AliESDs.root"
fi

if [ $INPUT_TYPE -eq 7 ]; then # Pb-Pb real LHC15o (AliHyperloop LHC15o_test sample)
  find /mnt/temp/Run3data_Vit/LHC15o_converted -name AO2D.root > listrun3.txt || exit 1
  DOCONVERT=0; DOALI=0; DOCOMPARE=0
fi

# Message formatting
function MsgStep { echo -e "\n\e[1;32m$@\e[0m"; }
function MsgSubStep { echo -e "\e[1m$@\e[0m"; }
function MsgWarn { echo -e "\e[1;36m$@\e[0m"; }
function MsgErr { echo -e "\e[1;31m$@\e[0m"; }

# Handle dependencies. (latest first)
[ $DOO2_TASK_D0 -eq 1 ] && { DOO2_SEL_D0=1; }
[ $DOO2_SEL_D0 -eq 1 ] && { DOO2_CAND_2PRONG=1; DOO2_PID_TPC=1; DOO2_PID_TOF=1; }
[ $DOO2_CAND_2PRONG -eq 1 ] && { DOO2_SKIM=1; }
[ $DOO2_TASK_DPLUS -eq 1 ] && { DOO2_CAND_3PRONG=1; }
[ $DOO2_CAND_3PRONG -eq 1 ] && { DOO2_SKIM=1; }

# Delete created files.
[ $DOCLEAN -eq 1 ] && bash clean.sh

# Lists of input files
LISTFILES_ALI="list_ali.txt"
ls $INPUTDIR/$STRING > $LISTFILES_ALI || { MsgErr "Error: Failed to make a list of input files."; exit 1; }
LISTFILES_O2="listrun3.txt"
LISTFILES_O2_RUN5="listrun5.txt"

# Output files names
FILEOUT="AnalysisResults.root"
FILEOUT_ALI="AnalysisResults_ALI.root"
FILEOUT_O2="AnalysisResults_O2.root"
FILEOUT_TREES="AnalysisResults_trees.root"
FILEOUT_TREES_O2="AnalysisResults_trees_O2.root"

# Steering commands
ENVALI="alienv setenv AliPhysics/latest -c"
ENVO2="alienv setenv O2/latest -c"
ENVALIO2="alienv setenv AliPhysics/latest,O2/latest -c"
CMDROOT="root -b -q -l"

# Default O2 input (Run 3)
O2INPUT=$LISTFILES_O2
# Adjust settings for Run5.
if [ $RUN5 -eq 1 ]; then
  MsgWarn "\nUsing Run 5 settings and input"
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
[[ $RUN5 -eq 1 && $DOCONVERT -eq 1 ]] && { MsgWarn "\nSkipping conversion for Run 5"; DOCONVERT=0; }
if [ $DOCONVERT -eq 1 ]; then
  [ -f "$LISTFILES_ALI" ] || { MsgErr "\nConverting: Error: File $LISTFILES_ALI does not exist."; exit 1; }
  MsgStep "Converting... ($(cat $LISTFILES_ALI | wc -l) files)"
  [ $ISMC -eq 1 ] && MsgWarn "Using MC mode"
  $ENVALI bash convert_batch.sh $LISTFILES_ALI $LISTFILES_O2 $ISMC $DEBUG || exit 1 # Run the batch script in the ALI environment.
fi

# Run AliPhysics tasks.
[[ $RUN5 -eq 1 && $DOALI -eq 1 ]] && { MsgWarn "\nSkipping HF tasks with AliPhysics for Run 5"; DOALI=0; }
if [ $DOALI -eq 1 ]; then
  [ -f "$LISTFILES_ALI" ] || { MsgErr "\nALI tasks: Error: File $LISTFILES_ALI does not exist."; exit 1; }
  MsgStep "Running tasks with AliPhysics... ($(cat $LISTFILES_ALI | wc -l) files)"
  rm -f $FILEOUT $FILEOUT_ALI
  $ENVALI bash ali_batch.sh $LISTFILES_ALI $JSON $ISMC $DEBUG || exit 1 # Run the batch script in the ALI environment.
  #$ENVALIO2 bash ali_batch.sh $LISTFILES_ALI $JSON $ISMC $DEBUG || exit 1 # Run the batch script in the ALI+O2 environment.
  mv $FILEOUT $FILEOUT_ALI
fi

# Run O2 tasks.
if [ $DOO2 -eq 1 ]; then
  [ -f "$O2INPUT" ] || { MsgErr "\nO2 tasks: Error: File $O2INPUT does not exist."; exit 1; }
  MsgStep "Running tasks with O2... ($(cat $O2INPUT | wc -l) files)"
  rm -f $FILEOUT $FILEOUT_O2
  # Basic common options
  O2ARGS="--shm-segment-size 16000000000 --configuration json://$JSON -b"
  # Options to save tables in trees
  [ $SAVETREES -eq 1 ] && {
    MsgWarn "Tables will be saved in trees."
    O2TABLES=""
    [ $DOO2_SKIM -eq 1 ] && { O2TABLES+="AOD/HFSELTRACK/0,AOD/HFTRACKIDXP2/0,AOD/HFTRACKIDXP3/0"; }
    [ $DOO2_CAND_2PRONG -eq 1 ] && { O2TABLES+=",AOD/HFCANDP2BASE/0,AOD/HFCANDP2EXT/0"; [ $ISMC -eq 1 ] && O2TABLES+=",AOD/HFCANDP2MCREC/0,AOD/HFCANDP2MCGEN/0"; }
    [ $DOO2_CAND_3PRONG -eq 1 ] && { O2TABLES+=",AOD/HFCANDP3BASE/0,AOD/HFCANDP3EXT/0"; }
    [ "$O2TABLES" ] && { O2ARGS+=" --aod-writer-keep $O2TABLES"; } || { MsgWarn "Empty list of tables!"; }
  }
  # Task-specific options
  O2ARGS_QA="$O2ARGS"
  O2ARGS_SKIM="$O2ARGS"
  O2ARGS_CAND_2PRONG="$O2ARGS"; [ $ISMC -eq 1 ] && O2ARGS_CAND_2PRONG+=" --doMC"
  O2ARGS_CAND_3PRONG="$O2ARGS"
  O2ARGS_PID_TPC="$O2ARGS"
  O2ARGS_PID_TOF="$O2ARGS"
  O2ARGS_SEL_D0="$O2ARGS"
  O2ARGS_TASK_D0="$O2ARGS"
  O2ARGS_TASK_DPLUS="$O2ARGS"
  # Options to parallelise
  if [ $PARALLELISE -eq 1 ]; then
    NPROC=3
    MsgWarn "O2 parallelisation ON ($NPROC)"
    O2ARGS_QA+=" --pipeline qa-tracking-kine:$NPROC,qa-tracking-resolution:$NPROC"
    O2ARGS_SKIM+=" --pipeline hf-produce-sel-track:$NPROC,hf-track-index-skims-creator:$NPROC"
    O2ARGS_CAND_2PRONG+=" --pipeline hf-cand-creator-2prong:$NPROC,hf-cand-creator-2prong-expressions:$NPROC"
    O2ARGS_CAND_3PRONG+=" --pipeline hf-cand-creator-3prong:$NPROC,hf-cand-creator-3prong-expressions:$NPROC"
    O2ARGS_TASK_D0+=" --pipeline hf-task-d0:$NPROC"
    O2ARGS_TASK_DPLUS+=" --pipeline hf-task-dplus:$NPROC"
  fi
  # Pair O2 executables with their respective options.
  O2EXEC_QA="o2-analysis-qatask $O2ARGS_QA"
  O2EXEC_SKIM="o2-analysis-hf-track-index-skims-creator $O2ARGS_SKIM"
  O2EXEC_CAND_2PRONG="o2-analysis-hf-candidate-creator-2prong $O2ARGS_CAND_2PRONG"
  O2EXEC_CAND_3PRONG="o2-analysis-hf-candidate-creator-3prong $O2ARGS_CAND_3PRONG"
  O2EXEC_PID_TPC="o2-analysis-pid-tpc $O2ARGS_PID_TPC"
  O2EXEC_PID_TOF="o2-analysis-pid-tof $O2ARGS_PID_TOF"
  O2EXEC_SEL_D0="o2-analysis-hf-d0-candidate-selector $O2ARGS_SEL_D0"
  O2EXEC_TASK_D0="o2-analysis-hf-task-d0 $O2ARGS_TASK_D0"
  O2EXEC_TASK_DPLUS="o2-analysis-hf-task-dplus $O2ARGS_TASK_DPLUS"
  # Form the full O2 command.
  [[ $DOO2_QA -eq 1 && $ISMC -eq 0 ]] && { MsgWarn "Skipping the QA task for non-MC input"; DOO2_QA=0; } # Disable running the QA task for non-MC input.
  echo "Tasks to be executed:"
  O2EXEC=""
  [ $DOO2_QA -eq 1 ] && { O2EXEC+=" | $O2EXEC_QA"; MsgSubStep "  qatask"; }
  [ $DOO2_SKIM -eq 1 ] && { O2EXEC+=" | $O2EXEC_SKIM"; MsgSubStep "  hf-track-index-skims-creator"; }
  [ $DOO2_CAND_2PRONG -eq 1 ] && { O2EXEC+=" | $O2EXEC_CAND_2PRONG"; MsgSubStep "  hf-candidate-creator-2prong"; }
  [ $DOO2_CAND_3PRONG -eq 1 ] && { O2EXEC+=" | $O2EXEC_CAND_3PRONG"; MsgSubStep "  hf-candidate-creator-3prong"; }
  [ $DOO2_PID_TPC -eq 1 ] && { O2EXEC+=" | $O2EXEC_PID_TPC"; MsgSubStep "  pid-tpc"; }
  [ $DOO2_PID_TOF -eq 1 ] && { O2EXEC+=" | $O2EXEC_PID_TOF"; MsgSubStep "  pid-tof"; }
  [ $DOO2_SEL_D0 -eq 1 ] && { O2EXEC+=" | $O2EXEC_SEL_D0"; MsgSubStep "  hf-d0-candidate-selector"; }
  [ $DOO2_TASK_D0 -eq 1 ] && { O2EXEC+=" | $O2EXEC_TASK_D0"; MsgSubStep "  hf-task-d0"; }
  [ $DOO2_TASK_DPLUS -eq 1 ] && { O2EXEC+=" | $O2EXEC_TASK_DPLUS"; MsgSubStep "  hf-task-dplus"; }
  O2EXEC=${O2EXEC:3} # Remove the starting " | ".
  O2SCRIPT="script_o2.sh"
  cat << EOF > $O2SCRIPT # Create a temporary script with the full O2 command.
#!/bin/bash
$O2EXEC
EOF
  [ $SAVETREES -eq 1 ] || FILEOUT_TREES=""
  $ENVO2 bash o2_batch.sh $O2INPUT $JSON $O2SCRIPT $DEBUG $FILEOUT_TREES || exit 1 # Run the batch script in the O2 environment.
  rm -f $O2SCRIPT
  mv $FILEOUT $FILEOUT_O2
  [[ $SAVETREES -eq 1 && "$FILEOUT_TREES" ]] && { mv $FILEOUT_TREES $FILEOUT_TREES_O2 || { MsgErr "Error"; exit 1; } }
fi

# Compare AliPhysics and O2 output.
[[ $RUN5 -eq 1 && $DOCOMPARE -eq 1 ]] && { MsgWarn "\nSkipping comparison for Run 5"; DOCOMPARE=0; }
if [ $DOCOMPARE -eq 1 ]; then
  LOGFILE="log_compare.log"
  MsgStep "Comparing... (logfile: $LOGFILE)"
  ok=1
  for file in "$FILEOUT_ALI" "$FILEOUT_O2"; do
    [ -f "$file" ] || { MsgErr "Error: File $file does not exist."; ok=0; }
  done
  [ $ok -ne 1 ] && exit 1
  $ENVALI $CMDROOT "Compare.C(\"$FILEOUT_O2\", \"$FILEOUT_ALI\", $MASS)" > $LOGFILE 2>&1 || { MsgErr "Error"; exit 1; }
fi

MsgStep "Done"

# Delete created files.
[ $DOCLEAN -eq 1 ] && {
  rm -f $LISTFILES_ALI
  rm -f $LISTFILES_O2
  [ $TWOPRONGSEL -eq 1 ] && rm "$JSONSEL"
}

exit 0
