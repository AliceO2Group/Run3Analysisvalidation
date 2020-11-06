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
DOO2_SEL_LC=0       # hf-lc-candidate-selector
DOO2_TASK_D0=1      # hf-task-d0
DOO2_TASK_DPLUS=1   # hf-task-dplus
DOO2_TASK_LC=0      # hf-task-lc

INPUT_CASE=4        # Input case (See the input specification choices below.)
NFILESMAX=1         # Maximum number of processed input files. (Set to -0 to process all; to -N to process all but the last N files.)
SAVETREES=0         # Save O2 tables to trees.
PARALLELISE=0       # Parallelise O2 tasks. (not working!)
APPLYCUTS_D0=0      # Apply D0 selection cuts.
APPLYCUTS_LC=0      # Apply Λc selection cuts.
DEBUG=0             # Print out more information.
##############################################################################

# Default settings
INPUT_LABEL="nothing"                 # Input description
INPUT_DIR=""                          # Input directory
INPUT_FILES="*/AliESDs.root"          # Input file pattern
JSON="$PWD/dpl-config_run3.json"      # Run 3 configuration
JSONRUN5="$PWD/dpl-config_run5.json"  # Run 5 configuration
ISINPUTO2=0                           # Input files are in O2 format.
ISMC=0                                # Switch for MC input
MASS=1.8                              # Hadron mass (only for comparison plots, not used)
TRIGGERSTRINGRUN2=""                  # Run 2 trigger (not used)
TRIGGERBITRUN3=-1                     # Run 3 trigger (not used)

# Input specification
case $INPUT_CASE in
  0)
    INPUT_LABEL="ppK0starToyMC"
    INPUT_DIR="../twikiinput"
    INPUT_FILES="AliESDs_ppK0starToyMC.root"
    MASS=1.0;;
  1)
    INPUT_LABEL="Pb-Pb real LHC15o"
    INPUT_DIR="/mnt/temp/Run3data/data/LHC15o_246751/pass1"
    TRIGGERSTRINGRUN2="CV0L7-B-NOPF-CENT"
    TRIGGERBITRUN3=5;; #FIXME
  2)
    INPUT_LABEL="Pb-Pb MC LHC15o"
    INPUT_DIR="/data/Run3data/alice_sim_2015_LHC15k1a3_246391/246391"
    ISMC=1;;
  3)
    INPUT_LABEL="?"
    INPUT_DIR="/data/Run3data/output"
    MASS=1.0;;
  4)
    INPUT_LABEL="p-p MC LHC17p"
    INPUT_DIR="/data/Run3data/alice_sim_2018_LHC18a4a2_cent/282099"
    ISMC=1;;
  5)
    INPUT_LABEL="p-p MC LHC17p"
    INPUT_DIR="/mnt/temp/Run3data_Vit/LHC18a4a2_cent/282341"
    ISMC=1;;
  6)
    INPUT_LABEL="p-p real LHC17p"
    INPUT_DIR="/mnt/temp/Run3data_Vit/LHC17p_pass1_CENT_woSDD/282341";;
  7)
    INPUT_LABEL="Pb-Pb real LHC15o, converted (AliHyperloop LHC15o_test sample)"
    INPUT_DIR="/mnt/temp/Run3data_Vit/LHC15o_converted/alice/data/2015/LHC15o/000244918/pass5_lowIR/PWGZZ/Run3_Conversion/96_20201013-1346_child_1/"
    INPUT_FILES="*/AO2D.root"
    ISINPUTO2=1;;
  8)
    INPUT_LABEL="Run 5, p-p MC 14 TeV MB"
    INPUT_DIR="/data/Run5data/MB_100kev_100cmdefault_05112020"
    INPUT_FILES="AODRun5.*.root"
    JSON="$JSONRUN5"
    ISINPUTO2=1
    ISMC=1;;
esac

# Message formatting
function MsgStep { echo -e "\n\e[1;32m$@\e[0m"; }
function MsgSubStep { echo -e "\e[1m$@\e[0m"; }
function MsgWarn { echo -e "\e[1;36m$@\e[0m"; }
function MsgErr { echo -e "\e[1;31m$@\e[0m"; }

# Lists of input files
LISTFILES_ALI="list_ali.txt"  # conversion and AliPhysics input
LISTFILES_O2="list_o2.txt"    # O2 input

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

# Handle dependencies. (latest first)
[ $ISINPUTO2 -eq 1 ] && { DOCONVERT=0; DOALI=0; DOCOMPARE=0; }
[ $DOO2_TASK_D0 -eq 1 ] && { DOO2_SEL_D0=1; }
[ $DOO2_SEL_D0 -eq 1 ] && { DOO2_CAND_2PRONG=1; DOO2_PID_TPC=1; DOO2_PID_TOF=1; }
[ $DOO2_CAND_2PRONG -eq 1 ] && { DOO2_SKIM=1; }
[ $DOO2_TASK_DPLUS -eq 1 ] && { DOO2_CAND_3PRONG=1; }
[ $DOO2_TASK_LC -eq 1 ] && { DOO2_SEL_LC=1; }
[ $DOO2_SEL_LC -eq 1 ] && { DOO2_CAND_3PRONG=1; DOO2_PID_TPC=1; DOO2_PID_TOF=1; }
[ $DOO2_CAND_3PRONG -eq 1 ] && { DOO2_SKIM=1; }

########## END OF CONFIGURATION ##########

########## START OF EXECUTION ##########

# Print out input description.
MsgStep "Processing case $INPUT_CASE: $INPUT_LABEL"

# Delete created files.
[ $DOCLEAN -eq 1 ] && { bash clean.sh || exit 1; }

# Generate list of input files.
[ $ISINPUTO2 -eq 1 ] && LISTFILES=$LISTFILES_O2 || LISTFILES=$LISTFILES_ALI
ls $INPUT_DIR/$INPUT_FILES | head -n $NFILESMAX > $LISTFILES
[ ${PIPESTATUS[0]} -eq 0 ] || { MsgErr "Error: Failed to make a list of input files."; exit 1; }

# Make a copy of the default JSON file to modify it.
JSON_EDIT=""
if [[ $APPLYCUTS_D0 -eq 1 || $APPLYCUTS_LC -eq 1 ]]; then
  JSON_EDIT="${JSON/.json/_sel.json}"
  cp "$JSON" "$JSON_EDIT" || { MsgErr "Error"; exit 1; }
  JSON="$JSON_EDIT"
fi

# Enable D0 selection.
if [ $APPLYCUTS_D0 -eq 1 ]; then
  MsgWarn "\nUsing D0 selection cuts"
  sed -e "s!\"d_selectionFlagD0\": \"0\"!\"d_selectionFlagD0\": \"1\"!g" "$JSON" > "$JSON.tmp" && mv "$JSON.tmp" "$JSON" && \
  sed -e "s!\"d_selectionFlagD0bar\": \"0\"!\"d_selectionFlagD0bar\": \"1\"!g" "$JSON" > "$JSON.tmp" && mv "$JSON.tmp" "$JSON" || { MsgErr "Error"; exit 1; }
fi

# Enable Λc selection.
if [ $APPLYCUTS_LC -eq 1 ]; then
  MsgWarn "\nUsing Λc selection cuts"
  sed -e "s!\"d_selectionFlagLc\": \"0\"!\"d_selectionFlagLc\": \"1\"!g" "$JSON" > "$JSON.tmp" && mv "$JSON.tmp" "$JSON" || { MsgErr "Error"; exit 1; }
fi

# Convert AliESDs.root to AO2D.root.
if [ $DOCONVERT -eq 1 ]; then
  [ -f "$LISTFILES_ALI" ] || { MsgErr "\nConverting: Error: File $LISTFILES_ALI does not exist."; exit 1; }
  MsgStep "Converting... ($(cat $LISTFILES_ALI | wc -l) files)"
  [ $ISMC -eq 1 ] && MsgWarn "Using MC mode"
  $ENVALI bash convert_batch.sh $LISTFILES_ALI $LISTFILES_O2 $ISMC $DEBUG || exit 1 # Run the batch script in the ALI environment.
fi

# Run AliPhysics tasks.
if [ $DOALI -eq 1 ]; then
  [ -f "$LISTFILES_ALI" ] || { MsgErr "\nALI tasks: Error: File $LISTFILES_ALI does not exist."; exit 1; }
  MsgStep "Running tasks with AliPhysics... ($(cat $LISTFILES_ALI | wc -l) files)"
  rm -f $FILEOUT $FILEOUT_ALI || { MsgErr "Error"; exit 1; }
  $ENVALI bash ali_batch.sh $LISTFILES_ALI $JSON $ISMC $DEBUG || exit 1 # Run the batch script in the ALI environment.
  #$ENVALIO2 bash ali_batch.sh $LISTFILES_ALI $JSON $ISMC $DEBUG || exit 1 # Run the batch script in the ALI+O2 environment.
  mv $FILEOUT $FILEOUT_ALI || { MsgErr "Error"; exit 1; }
fi

# Run O2 tasks.
if [ $DOO2 -eq 1 ]; then
  [ -f "$LISTFILES_O2" ] || { MsgErr "\nO2 tasks: Error: File $LISTFILES_O2 does not exist."; exit 1; }
  MsgStep "Running tasks with O2... ($(cat $LISTFILES_O2 | wc -l) files)"
  rm -f $FILEOUT $FILEOUT_O2 || { MsgErr "Error"; exit 1; }
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
  O2ARGS_SEL_LC="$O2ARGS"
  O2ARGS_TASK_D0="$O2ARGS"
  O2ARGS_TASK_DPLUS="$O2ARGS"
  O2ARGS_TASK_LC="$O2ARGS"
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
    O2ARGS_TASK_LC+=" --pipeline hf-task-lc:$NPROC"
  fi
  # Pair O2 executables with their respective options.
  O2EXEC_QA="o2-analysis-qatask $O2ARGS_QA"
  O2EXEC_SKIM="o2-analysis-hf-track-index-skims-creator $O2ARGS_SKIM"
  O2EXEC_CAND_2PRONG="o2-analysis-hf-candidate-creator-2prong $O2ARGS_CAND_2PRONG"
  O2EXEC_CAND_3PRONG="o2-analysis-hf-candidate-creator-3prong $O2ARGS_CAND_3PRONG"
  O2EXEC_PID_TPC="o2-analysis-pid-tpc $O2ARGS_PID_TPC"
  O2EXEC_PID_TOF="o2-analysis-pid-tof $O2ARGS_PID_TOF"
  O2EXEC_SEL_D0="o2-analysis-hf-d0-candidate-selector $O2ARGS_SEL_D0"
  O2EXEC_SEL_LC="o2-analysis-hf-lc-candidate-selector $O2ARGS_SEL_LC"
  O2EXEC_TASK_D0="o2-analysis-hf-task-d0 $O2ARGS_TASK_D0"
  O2EXEC_TASK_DPLUS="o2-analysis-hf-task-dplus $O2ARGS_TASK_DPLUS"
  O2EXEC_TASK_LC="o2-analysis-hf-task-lc $O2ARGS_TASK_LC"
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
  [ $DOO2_SEL_LC -eq 1 ] && { O2EXEC+=" | $O2EXEC_SEL_LC"; MsgSubStep "  hf-lc-candidate-selector"; }
  [ $DOO2_TASK_D0 -eq 1 ] && { O2EXEC+=" | $O2EXEC_TASK_D0"; MsgSubStep "  hf-task-d0"; }
  [ $DOO2_TASK_DPLUS -eq 1 ] && { O2EXEC+=" | $O2EXEC_TASK_DPLUS"; MsgSubStep "  hf-task-dplus"; }
  [ $DOO2_TASK_LC -eq 1 ] && { O2EXEC+=" | $O2EXEC_TASK_LC"; MsgSubStep "  hf-task-lc"; }
  O2EXEC=${O2EXEC:3} # Remove the leading " | ".
  O2SCRIPT="script_o2.sh"
  cat << EOF > $O2SCRIPT # Create a temporary script with the full O2 command.
#!/bin/bash
$O2EXEC
EOF
  [ $SAVETREES -eq 1 ] || FILEOUT_TREES=""
  $ENVO2 bash o2_batch.sh $LISTFILES_O2 $JSON $O2SCRIPT $DEBUG $FILEOUT_TREES || exit 1 # Run the batch script in the O2 environment.
  rm -f $O2SCRIPT && \
  mv $FILEOUT $FILEOUT_O2 || { MsgErr "Error"; exit 1; }
  [[ $SAVETREES -eq 1 && "$FILEOUT_TREES" ]] && { mv $FILEOUT_TREES $FILEOUT_TREES_O2 || { MsgErr "Error"; exit 1; } }
fi

# Compare AliPhysics and O2 output.
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

# Delete created files.
[ $DOCLEAN -eq 1 ] && {
  rm -f $LISTFILES_ALI && \
  rm -f $LISTFILES_O2 || { MsgErr "Error"; exit 1; }
  [ "$JSON_EDIT" ] && { rm "$JSON_EDIT" || { MsgErr "Error"; exit 1; } }
}

MsgStep "Done"

exit 0
