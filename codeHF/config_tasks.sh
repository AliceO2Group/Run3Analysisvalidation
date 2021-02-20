#!/bin/bash
# shellcheck disable=SC2034 # Ignore unused parameters.

# Configuration of tasks for runtest.sh
# (Cleans directory, modifies step activation, modifies JSON, generates step scripts.)

# Mandatory functions:
#   Clean                  Performs cleanup before (argument=1) and after (argument=2) running.
#   AdjustJson             Modifies the JSON file.
#   MakeScriptAli          Generates the AliPhysics script.
#   MakeScriptO2           Generates the O2 script.
#   MakeScriptPostprocess  Generates the postprocessing script.

####################################################################################################

# Steps
DOCLEAN=1           # Delete created files (before and after running tasks).
DOCONVERT=1         # Convert AliESDs.root to AO2D.root.
DOALI=1             # Run AliPhysics tasks.
DOO2=1              # Run O2 tasks.
DOPOSTPROCESS=1     # Run output postprocessing. (Compare AliPhysics and O2 output.)

# Disable incompatible steps.
[ "$ISINPUTO2" -eq 1 ] && { DOCONVERT=0; DOALI=0; }

# Activation of O2 tasks
# QA
DOO2_QA_EFF=1       # qa-efficiency
DOO2_QA_SIM=1       # qa-simple
DOO2_MC_VALID=0     # hf-mc-validation
# PID
DOO2_PID_TPC=0      # pid-tpc
DOO2_PID_TOF=0      # pid-tof
# Vertexing
DOO2_SKIM=0         # hf-track-index-skims-creator
DOO2_CAND_2PRONG=0  # hf-candidate-creator-2prong
DOO2_CAND_3PRONG=0  # hf-candidate-creator-3prong
# Selectors
DOO2_SEL_D0=0       # hf-d0-candidate-selector
DOO2_SEL_LC=0       # hf-lc-candidate-selector
DOO2_SEL_JPSI=0     # hf-jpsi-toee-candidate-selector
# User tasks
DOO2_TASK_D0=1      # hf-task-d0
DOO2_TASK_DPLUS=0   # hf-task-dplus
DOO2_TASK_LC=0      # hf-task-lc
DOO2_TASK_JPSI=0    # hf-task-jpsi
DOO2_TASK_BPLUS=0   # hf-task-bplus
# Tree creators
DOO2_TREE_D0=0      # hf-tree-creator-d0-tokpi
DOO2_TREE_LC=0      # hf-tree-creator-lc-topkpi

# Selection cuts
APPLYCUTS_D0=0      # Apply D0 selection cuts.
APPLYCUTS_LC=0      # Apply Λc selection cuts.
APPLYCUTS_JPSI=0    # Apply J/ψ selection cuts.

SAVETREES=0         # Save O2 tables to trees.
USEO2VERTEXER=0     # Use the O2 vertexer in AliPhysics.
DORATIO=0           # Plot histogram ratios in comparison.

####################################################################################################

# Clean before (argument=1) and after (argument=2) running.
function Clean {
  # Cleanup before running
  [ "$1" -eq 1 ] && { bash "$DIR_TASKS/clean.sh" || ErrExit; }

  # Cleanup after running
  [ "$1" -eq 2 ] && {
    rm -f "$LISTFILES_ALI" "$LISTFILES_O2" "$SCRIPT_ALI" "$SCRIPT_O2" "$SCRIPT_POSTPROCESS" || ErrExit "Failed to rm created files."
    [ "$JSON_EDIT" ] && { rm "$JSON_EDIT" || ErrExit "Failed to rm $JSON_EDIT."; }
  }

  return 0
}

# Modify the JSON file.
function AdjustJson {
  # Make a copy of the default JSON file to modify it.
  JSON_EDIT=""
  if [[ $APPLYCUTS_D0 -eq 1 || $APPLYCUTS_LC -eq 1 || $APPLYCUTS_JPSI -eq 1 ]]; then
    JSON_EDIT="${JSON/.json/_edit.json}"
    cp "$JSON" "$JSON_EDIT" || ErrExit "Failed to cp $JSON $JSON_EDIT."
    JSON="$JSON_EDIT"
  fi

  # Enable D0 selection.
  if [ $APPLYCUTS_D0 -eq 1 ]; then
    MsgWarn "\nUsing D0 selection cuts"
    ReplaceString "\"d_selectionFlagD0\": \"0\"" "\"d_selectionFlagD0\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"d_selectionFlagD0bar\": \"0\"" "\"d_selectionFlagD0bar\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable Λc selection.
  if [ $APPLYCUTS_LC -eq 1 ]; then
    MsgWarn "\nUsing Λc selection cuts"
    ReplaceString "\"d_selectionFlagLc\": \"0\"" "\"d_selectionFlagLc\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

    # Enable J/ψ selection.
  if [ $APPLYCUTS_JPSI -eq 1 ]; then
    MsgWarn "\nUsing J/ψ selection cuts"
    ReplaceString "\"d_selectionFlagJpsi\": \"0\"" "\"d_selectionFlagJpsi\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi
}

# Generate the O2 script containing the full workflow specification.
function MakeScriptO2 {
  # Deactivate MC-only tasks for non-MC input.
  [ "$ISMC" -eq 0 ] && {
    [ $DOO2_QA_EFF -eq 1 ] && { MsgWarn "Skipping the QA task efficiency for non-MC input"; DOO2_QA_EFF=0; }
    [ $DOO2_QA_SIM -eq 1 ] && { MsgWarn "Skipping the QA task simple for non-MC input"; DOO2_QA_SIM=0; }
    [ $DOO2_TREE_D0 -eq 1 ] && { MsgWarn "Skipping the D0 tree creator for non-MC input"; DOO2_TREE_D0=0; }
    [ $DOO2_TREE_LC -eq 1 ] && { MsgWarn "Skipping the Λc tree creator for non-MC input"; DOO2_TREE_LC=0; }
    [ $DOO2_MC_VALID -eq 1 ] && { MsgWarn "Skipping the MC validation task for non-MC input"; DOO2_MC_VALID=0; }
  }

  # Handle dependencies. (latest first)
  # QA
  [ $DOO2_MC_VALID -eq 1 ] && { DOO2_SEL_D0=1; }
  # Tree creators
  [ $DOO2_TREE_D0 -eq 1 ] && { DOO2_SEL_D0=1; }
  [ $DOO2_TREE_LC -eq 1 ] && { DOO2_SEL_LC=1; }
  # User tasks
  [ $DOO2_TASK_D0 -eq 1 ] && { DOO2_SEL_D0=1; }
  [ $DOO2_TASK_JPSI -eq 1 ] && { DOO2_SEL_JPSI=1; }
  [ $DOO2_TASK_LC -eq 1 ] && { DOO2_SEL_LC=1; }
  [ $DOO2_TASK_DPLUS -eq 1 ] && { DOO2_CAND_3PRONG=1; }
  [ $DOO2_TASK_BPLUS -eq 1 ] && { DOO2_SEL_D0=1; }
  # Selectors
  [[ $DOO2_SEL_D0 -eq 1 || $DOO2_SEL_JPSI -eq 1 ]] && { DOO2_CAND_2PRONG=1; DOO2_PID_TPC=1; DOO2_PID_TOF=1; }
  [[ $DOO2_SEL_LC -eq 1 ]] && { DOO2_CAND_3PRONG=1; DOO2_PID_TPC=1; DOO2_PID_TOF=1; }
  # Vertexing
  [[ $DOO2_CAND_2PRONG -eq 1 || $DOO2_CAND_3PRONG -eq 1 ]] && { DOO2_SKIM=1; }

  # General options
  O2ARGS="--aod-memory-rate-limit 10000000000 --shm-segment-size 16000000000 --configuration json://\$JSON -b"

  # Options to save tables in trees
  [ $SAVETREES -eq 1 ] && {
    MsgWarn "Tables will be saved in trees."
    O2TABLES=""
    [ $DOO2_SKIM -eq 1 ] && { O2TABLES+=",AOD/HFSELTRACK/0,AOD/HFTRACKIDXP2/0,AOD/HFTRACKIDXP3/0"; }
    [ $DOO2_CAND_2PRONG -eq 1 ] && { O2TABLES+=",AOD/HFCANDP2BASE/0,AOD/HFCANDP2EXT/0"; [ "$ISMC" -eq 1 ] && O2TABLES+=",AOD/HFCANDP2MCREC/0,AOD/HFCANDP2MCGEN/0"; }
    [ $DOO2_CAND_3PRONG -eq 1 ] && { O2TABLES+=",AOD/HFCANDP3BASE/0,AOD/HFCANDP3EXT/0"; [ "$ISMC" -eq 1 ] && O2TABLES+=",AOD/HFCANDP3MCREC/0,AOD/HFCANDP3MCGEN/0"; }
    [ $DOO2_TREE_D0 -eq 1 ] && { O2TABLES+=",AOD/HFCANDP2Full/0,AOD/HFCANDP2FullE/0,AOD/HFCANDP2FullP/0"; }
    [ $DOO2_TREE_LC -eq 1 ] && { O2TABLES+=",AOD/HFCANDP3Full/0,AOD/HFCANDP3FullE/0,AOD/HFCANDP3FullP/0"; }
    O2TABLES=${O2TABLES:1} # Remove the leading comma.
    # shellcheck disable=SC2015 # Ignore A && B || C.
    [ "$O2TABLES" ] && { O2ARGS+=" --aod-writer-keep $O2TABLES"; } || { ErrExit "Empty list of tables!"; }
  }

  # Task-specific options
  O2ARGS_QA_EFF="$O2ARGS"
  O2ARGS_QA_SIM="$O2ARGS"
  O2ARGS_SKIM="$O2ARGS"
  O2ARGS_CAND_2PRONG="$O2ARGS"
  O2ARGS_CAND_3PRONG="$O2ARGS"
  O2ARGS_PID_TPC="$O2ARGS"
  O2ARGS_PID_TOF="$O2ARGS"
  O2ARGS_SEL_D0="$O2ARGS"
  O2ARGS_SEL_JPSI="$O2ARGS"
  O2ARGS_SEL_LC="$O2ARGS"
  O2ARGS_TASK_D0="$O2ARGS"
  O2ARGS_TASK_JPSI="$O2ARGS"
  O2ARGS_TASK_DPLUS="$O2ARGS"
  O2ARGS_TASK_LC="$O2ARGS"
  O2ARGS_TASK_BPLUS="$O2ARGS"
  O2ARGS_TREE_D0="$O2ARGS"
  O2ARGS_TREE_LC="$O2ARGS"
  O2ARGS_MC_VALID="$O2ARGS"
  # MC
  [ "$ISMC" -eq 1 ] && {
    O2ARGS_CAND_2PRONG+=" --doMC"
    O2ARGS_CAND_3PRONG+=" --doMC"
    O2ARGS_TASK_D0+=" --doMC"
    O2ARGS_TASK_LC+=" --doMC"
    O2ARGS_TASK_JPSI+=" --doMC"
  }

  # Pair O2 executables with their respective options.
  O2EXEC_QA_EFF="o2-analysis-qa-efficiency $O2ARGS_QA_EFF"
  O2EXEC_QA_SIM="o2-analysis-qa-simple $O2ARGS_QA_SIM"
  O2EXEC_SKIM="o2-analysis-hf-track-index-skims-creator $O2ARGS_SKIM"
  O2EXEC_CAND_2PRONG="o2-analysis-hf-candidate-creator-2prong $O2ARGS_CAND_2PRONG"
  O2EXEC_CAND_3PRONG="o2-analysis-hf-candidate-creator-3prong $O2ARGS_CAND_3PRONG"
  O2EXEC_PID_TPC="o2-analysis-pid-tpc $O2ARGS_PID_TPC"
  O2EXEC_PID_TOF="o2-analysis-pid-tof $O2ARGS_PID_TOF"
  O2EXEC_SEL_D0="o2-analysis-hf-d0-candidate-selector $O2ARGS_SEL_D0"
  O2EXEC_SEL_LC="o2-analysis-hf-lc-candidate-selector $O2ARGS_SEL_LC"
  O2EXEC_SEL_JPSI="o2-analysis-hf-jpsi-toee-candidate-selector $O2ARGS_SEL_JPSI"
  O2EXEC_TASK_D0="o2-analysis-hf-task-d0 $O2ARGS_TASK_D0"
  O2EXEC_TASK_JPSI="o2-analysis-hf-task-jpsi $O2ARGS_TASK_JPSI"
  O2EXEC_TASK_DPLUS="o2-analysis-hf-task-dplus $O2ARGS_TASK_DPLUS"
  O2EXEC_TASK_LC="o2-analysis-hf-task-lc $O2ARGS_TASK_LC"
  O2EXEC_TASK_BPLUS="o2-analysis-hf-task-bplus $O2ARGS_TASK_BPLUS"
  O2EXEC_TREE_D0="o2-analysis-hf-tree-creator-d0-tokpi $O2ARGS_TREE_D0"
  O2EXEC_TREE_LC="o2-analysis-hf-tree-creator-lc-topkpi $O2ARGS_TREE_LC"
  O2EXEC_MC_VALID="o2-analysis-hf-mc-validation $O2ARGS_MC_VALID"

  # Form the full O2 command.
  echo "Tasks to be executed:"
  O2EXEC=""
  [ $DOO2_QA_EFF -eq 1 ] && { O2EXEC+=" | $O2EXEC_QA_EFF"; MsgSubStep "  qa-efficiency"; }
  [ $DOO2_QA_SIM -eq 1 ] && { O2EXEC+=" | $O2EXEC_QA_SIM"; MsgSubStep "  qa-simple"; }
  [ $DOO2_SKIM -eq 1 ] && { O2EXEC+=" | $O2EXEC_SKIM"; MsgSubStep "  hf-track-index-skims-creator"; }
  [ $DOO2_CAND_2PRONG -eq 1 ] && { O2EXEC+=" | $O2EXEC_CAND_2PRONG"; MsgSubStep "  hf-candidate-creator-2prong"; }
  [ $DOO2_CAND_3PRONG -eq 1 ] && { O2EXEC+=" | $O2EXEC_CAND_3PRONG"; MsgSubStep "  hf-candidate-creator-3prong"; }
  [ $DOO2_PID_TPC -eq 1 ] && { O2EXEC+=" | $O2EXEC_PID_TPC"; MsgSubStep "  pid-tpc"; }
  [ $DOO2_PID_TOF -eq 1 ] && { O2EXEC+=" | $O2EXEC_PID_TOF"; MsgSubStep "  pid-tof"; }
  [ $DOO2_SEL_D0 -eq 1 ] && { O2EXEC+=" | $O2EXEC_SEL_D0"; MsgSubStep "  hf-d0-candidate-selector"; }
  [ $DOO2_SEL_JPSI -eq 1 ] && { O2EXEC+=" | $O2EXEC_SEL_JPSI"; MsgSubStep "  hf-jpsi-toee-candidate-selector"; }
  [ $DOO2_SEL_LC -eq 1 ] && { O2EXEC+=" | $O2EXEC_SEL_LC"; MsgSubStep "  hf-lc-candidate-selector"; }
  [ $DOO2_TASK_D0 -eq 1 ] && { O2EXEC+=" | $O2EXEC_TASK_D0"; MsgSubStep "  hf-task-d0"; }
  [ $DOO2_TASK_DPLUS -eq 1 ] && { O2EXEC+=" | $O2EXEC_TASK_DPLUS"; MsgSubStep "  hf-task-dplus"; }
  [ $DOO2_TASK_LC -eq 1 ] && { O2EXEC+=" | $O2EXEC_TASK_LC"; MsgSubStep "  hf-task-lc"; }
  [ $DOO2_TASK_JPSI -eq 1 ] && { O2EXEC+=" | $O2EXEC_TASK_JPSI"; MsgSubStep "  hf-task-jpsi"; }
  [ $DOO2_TASK_BPLUS -eq 1 ] && { O2EXEC+=" | $O2EXEC_TASK_BPLUS"; MsgSubStep "  hf-task-bplus"; }
  [ $DOO2_TREE_D0 -eq 1 ] && { O2EXEC+=" | $O2EXEC_TREE_D0"; MsgSubStep "  hf-tree-creator-d0-tokpi"; }
  [ $DOO2_TREE_LC -eq 1 ] && { O2EXEC+=" | $O2EXEC_TREE_LC"; MsgSubStep "  hf-tree-creator-lc-topkpi"; }
  [ $DOO2_MC_VALID -eq 1 ] && { O2EXEC+=" | $O2EXEC_MC_VALID"; MsgSubStep "  hf-mc-validation"; }
  O2EXEC=${O2EXEC:3} # Remove the leading " | ".
  [ "$O2EXEC" ] || ErrExit "Nothing to do!"

  # Create the script with the full O2 command.
  cat << EOF > "$SCRIPT_O2"
#!/bin/bash
FileIn="\$1"
JSON="\$2"
mkdir sockets && \
$O2EXEC --fairmq-ipc-prefix sockets && \
rm -r sockets
EOF
}

function MakeScriptAli {
  ALIEXEC="root -b -q -l \"$DIR_TASKS/RunHFTaskLocal.C(\\\"\$FileIn\\\", \\\"\$JSON\\\", $ISMC, $USEO2VERTEXER)\""
  cat << EOF > "$SCRIPT_ALI"
#!/bin/bash
FileIn="\$1"
JSON="\$2"
$ALIEXEC
EOF
}

function MakeScriptPostprocess {
  POSTEXEC="echo Postprocessing"
  # Compare AliPhysics and O2 histograms.
  [[ $DOALI -eq 1 && $DOO2 -eq 1 ]] && {
    OPT_COMPARE=""
    [ $DOO2_SKIM -eq 1 ] && OPT_COMPARE+="-tracks-skim"
    [ $DOO2_CAND_2PRONG -eq 1 ] && OPT_COMPARE+="-cand2"
    [ $DOO2_CAND_3PRONG -eq 1 ] && OPT_COMPARE+="-cand3"
    [ $DOO2_TASK_D0 -eq 1 ] && OPT_COMPARE+="-d0"
    [ $DOO2_TASK_DPLUS -eq 1 ] && OPT_COMPARE+="-dplus"
    [ $DOO2_TASK_LC -eq 1 ] && OPT_COMPARE+="-lc"
    [ $DOO2_TASK_JPSI -eq 1 ] && OPT_COMPARE+="-jpsi"
    [ "$OPT_COMPARE" ] && POSTEXEC+=" && root -b -q -l \"$DIR_TASKS/Compare.C(\\\"\$FileO2\\\", \\\"\$FileAli\\\", \\\"$OPT_COMPARE\\\", $DORATIO)\""
  }
  # Plot particle reconstruction efficiencies.
  [[ $DOO2 -eq 1 && $ISMC -eq 1 ]] && {
    PARTICLES=""
    [ $DOO2_TASK_D0 -eq 1 ] && PARTICLES+="-d0"
    [ $DOO2_TASK_LC -eq 1 ] && PARTICLES+="-lc"
    [ $DOO2_TASK_JPSI -eq 1 ] && PARTICLES+="-jpsi"
    [ "$PARTICLES" ] && POSTEXEC+=" && root -b -q -l \"$DIR_TASKS/PlotEfficiency.C(\\\"\$FileO2\\\", \\\"$PARTICLES\\\")\""
  }
  cat << EOF > "$SCRIPT_POSTPROCESS"
#!/bin/bash
FileO2="\$1"
FileAli="\$2"
$POSTEXEC
EOF
}
