#!/bin/bash

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
[ $ISINPUTO2 -eq 1 ] && { DOCONVERT=0; DOALI=0; }

# Activation of O2 tasks
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

# Selection cuts
APPLYCUTS_D0=0      # Apply D0 selection cuts.
APPLYCUTS_LC=0      # Apply Λc selection cuts.

SAVETREES=0         # Save O2 tables to trees.
DEBUG=0             # Print out more information.

MASS=1.8            # Hadron mass (only for comparison plots, not used)

####################################################################################################

# Clean before (argument=1) and after (argument=2) running.
function Clean {
  # Cleanup before running
  [ $1 -eq 1 ] && { bash "$DIR_TASKS/clean.sh" || ErrExit; }

  # Cleanup after running
  [ $1 -eq 2 ] && {
    rm -f $LISTFILES_ALI && \
    rm -f $LISTFILES_O2 && \
    rm -f $SCRIPT_ALI && \
    rm -f $SCRIPT_O2 && \
    rm -f $SCRIPT_POSTPROCESS || ErrExit "Failed to rm created files."
    [ "$JSON_EDIT" ] && { rm "$JSON_EDIT" || ErrExit "Failed to rm $JSON_EDIT."; }
  }

  return 0
}

# Modify the JSON file.
function AdjustJson {
  # Make a copy of the default JSON file to modify it.
  JSON_EDIT=""
  if [[ $APPLYCUTS_D0 -eq 1 || $APPLYCUTS_LC -eq 1 ]]; then
    JSON_EDIT="${JSON/.json/_edit.json}"
    cp "$JSON" "$JSON_EDIT" || ErrExit "Failed to cp $JSON $JSON_EDIT."
    JSON="$JSON_EDIT"
  fi

  # Enable D0 selection.
  if [ $APPLYCUTS_D0 -eq 1 ]; then
    MsgWarn "\nUsing D0 selection cuts"
    ReplaceString "\"d_selectionFlagD0\": \"0\"" "\"d_selectionFlagD0\": \"1\"" "$JSON" && \
    ReplaceString "\"d_selectionFlagD0bar\": \"0\"" "\"d_selectionFlagD0bar\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable Λc selection.
  if [ $APPLYCUTS_LC -eq 1 ]; then
    MsgWarn "\nUsing Λc selection cuts"
    ReplaceString "\"d_selectionFlagLc\": \"0\"" "\"d_selectionFlagLc\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi
}

# Generate the O2 script containing the full workflow specification.
function MakeScriptO2 {
  # Handle dependencies. (latest first)
  [ $DOO2_TASK_D0 -eq 1 ] && { DOO2_SEL_D0=1; }
  [ $DOO2_SEL_D0 -eq 1 ] && { DOO2_CAND_2PRONG=1; DOO2_PID_TPC=1; DOO2_PID_TOF=1; }
  [ $DOO2_CAND_2PRONG -eq 1 ] && { DOO2_SKIM=1; }
  [ $DOO2_TASK_DPLUS -eq 1 ] && { DOO2_CAND_3PRONG=1; }
  [ $DOO2_TASK_LC -eq 1 ] && { DOO2_SEL_LC=1; }
  [ $DOO2_SEL_LC -eq 1 ] && { DOO2_CAND_3PRONG=1; DOO2_PID_TPC=1; DOO2_PID_TOF=1; }
  [ $DOO2_CAND_3PRONG -eq 1 ] && { DOO2_SKIM=1; }

  # Basic common options
  O2ARGS="--shm-segment-size 16000000000 --configuration json://\$JSON -b"
  # Options to save tables in trees
  [ $SAVETREES -eq 1 ] && {
    MsgWarn "Tables will be saved in trees."
    O2TABLES=""
    [ $DOO2_SKIM -eq 1 ] && { O2TABLES+="AOD/HFSELTRACK/0,AOD/HFTRACKIDXP2/0,AOD/HFTRACKIDXP3/0"; }
    [ $DOO2_CAND_2PRONG -eq 1 ] && { O2TABLES+=",AOD/HFCANDP2BASE/0,AOD/HFCANDP2EXT/0"; [ $ISMC -eq 1 ] && O2TABLES+=",AOD/HFCANDP2MCREC/0,AOD/HFCANDP2MCGEN/0"; }
    [ $DOO2_CAND_3PRONG -eq 1 ] && { O2TABLES+=",AOD/HFCANDP3BASE/0,AOD/HFCANDP3EXT/0"; [ $ISMC -eq 1 ] && O2TABLES+=",AOD/HFCANDP3MCREC/0,AOD/HFCANDP3MCGEN/0"; }
    [ "$O2TABLES" ] && { O2ARGS+=" --aod-writer-keep $O2TABLES"; } || { MsgWarn "Empty list of tables!"; }
  }
  # Task-specific options
  O2ARGS_QA="$O2ARGS"
  O2ARGS_SKIM="$O2ARGS"
  O2ARGS_CAND_2PRONG="$O2ARGS"
  O2ARGS_CAND_3PRONG="$O2ARGS"
  O2ARGS_PID_TPC="$O2ARGS"
  O2ARGS_PID_TOF="$O2ARGS"
  O2ARGS_SEL_D0="$O2ARGS"
  O2ARGS_SEL_LC="$O2ARGS"
  O2ARGS_TASK_D0="$O2ARGS"
  O2ARGS_TASK_DPLUS="$O2ARGS"
  O2ARGS_TASK_LC="$O2ARGS"
  # MC
  [ $ISMC -eq 1 ] && {
    O2ARGS_CAND_2PRONG+=" --doMC"
    O2ARGS_CAND_3PRONG+=" --doMC"
    O2ARGS_TASK_D0+=" --doMC"
  }

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
  [ "$O2EXEC" ] || ErrExit "Nothing to do!"

  # Create the script with the full O2 command.
  cat << EOF > $SCRIPT_O2
#!/bin/bash
FileIn="\$1"
JSON="\$2"
$O2EXEC
EOF
}

function MakeScriptAli {
  ALIEXEC="root -b -q -l \"$DIR_TASKS/RunHFTaskLocal.C(\\\"\$FileIn\\\", \\\"\$JSON\\\", $ISMC)\""
  cat << EOF > $SCRIPT_ALI
#!/bin/bash
FileIn="\$1"
JSON="\$2"
$ALIEXEC
EOF
}

function MakeScriptPostprocess {
  POSTEXEC="echo Postprocessing"
  [[ $DOALI -eq 1 && $DOO2 -eq 1 && $DOO2_TASK_D0 -eq 1 && $DOO2_TASK_DPLUS -eq 1 ]] && POSTEXEC+=" && root -b -q -l \"$DIR_TASKS/Compare.C(\\\"\$FileO2\\\", \\\"\$FileAli\\\", $MASS)\""
  [[ $DOO2 -eq 1 && $DOO2_TASK_D0 -eq 1 && $ISMC -eq 1 ]] && POSTEXEC+=" && root -b -q -l \"$DIR_TASKS/PlotEfficiency.C(\\\"\$FileO2\\\")\""
  cat << EOF > $SCRIPT_POSTPROCESS
#!/bin/bash
FileO2="\$1"
FileAli="\$2"
$POSTEXEC
EOF
}
