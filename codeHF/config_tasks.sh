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
DOPOSTPROCESS=1     # Run output postprocessing. (Comparison plots. Requires DOALI=1 and/or DOO2=1)

# Disable incompatible steps.
[ "$ISINPUTO2" -eq 1 ] && { DOCONVERT=0; DOALI=0; }

# O2 database
DATABASE_O2="workflows.yml"
MAKE_GRAPH=0        # Make topology graph.

# Activation of O2 workflows
# Trigger selection
DOO2_TRIGSEL=0      # event-selection
# QA
DOO2_REJ_ALICE3=0   # hf-task-qa-pid-rejection
DOO2_QA_EFF=0       # qa-efficiency
DOO2_QA_EVTRK=0     # qa-event-track
DOO2_MC_VALID=0     # hf-task-mc-validation
# PID
DOO2_PID_TPC=0      # pid-tpc
DOO2_PID_TOF=0      # pid-tof/alice3-pid-tof
DOO2_PID_TOF_QA=0   # pid-tof-qa-mc
DOO2_PID_BAYES=0    # pid-bayes
# Vertexing
DOO2_SKIM=1         # hf-track-index-skim-creator
DOO2_CAND_2PRONG=1  # hf-candidate-creator-2prong
DOO2_CAND_3PRONG=1  # hf-candidate-creator-3prong
DOO2_CAND_CASC=0    # hf-candidate-creator-cascade
DOO2_CAND_LB=0      # hf-candidate-creator-lb
DOO2_CAND_X=0       # hf-candidate-creator-x
DOO2_CAND_CHIC=0    # hf-candidate-creator-chic
DOO2_CAND_XICC=0    # hf-candidate-creator-xicc
DOO2_CAND_B0=0      # hf-candidate-creator-b0
DOO2_CAND_BPLUS=0   # hf-candidate-creator-bplus
DOO2_CAND_DSTAR=0   # hf-candidate-creator-dstar
# Selectors
DOO2_SEL_D0=0       # hf-candidate-selector-d0
DOO2_SEL_DS=0       # hf-candidate-selector-ds-to-k-k-pi
DOO2_SEL_DPLUS=0    # hf-candidate-selector-dplus-to-pi-k-pi
DOO2_SEL_LC=0       # hf-candidate-selector-lc
DOO2_SEL_LB=0       # hf-candidate-selector-lb-to-lc-pi
DOO2_SEL_XIC=0      # hf-candidate-selector-xic-to-p-k-pi
DOO2_SEL_JPSI=0     # hf-candidate-selector-jpsi
DOO2_SEL_X=0        # hf-candidate-selector-x-to-jpsi-pi-pi
DOO2_SEL_CHIC=0     # hf-candidate-selector-chic-to-jpsi-gamma
DOO2_SEL_LCK0SP=0   # hf-candidate-selector-lc-to-k0s-p
DOO2_SEL_XICC=0     # hf-candidate-selector-xicc-to-p-k-pi-pi
DOO2_SEL_B0=0       # hf-candidate-selector-b0-to-d-pi
DOO2_SEL_BPLUS=0    # hf-candidate-selector-bplus-to-d0-pi
# User tasks
DOO2_TASK_D0=1      # hf-task-d0
DOO2_TASK_DS=0      # hf-task-ds
DOO2_TASK_DPLUS=0   # hf-task-dplus
DOO2_TASK_LC=1      # hf-task-lc
DOO2_TASK_LB=0      # hf-task-lb
DOO2_TASK_XIC=0     # hf-task-xic
DOO2_TASK_JPSI=0    # hf-task-jpsi
DOO2_TASK_X=0       # hf-task-x
DOO2_TASK_CHIC=0    # hf-task-chic
DOO2_TASK_LCK0SP=0  # hf-task-lc-to-k0s-p
DOO2_TASK_XICC=0    # hf-task-xicc
DOO2_TASK_B0=0      # hf-task-b0
DOO2_TASK_BPLUS=0   # hf-task-bplus
# Tree creators
DOO2_TREE_D0=0      # hf-tree-creator-d0-to-k-pi
DOO2_TREE_LC=0      # hf-tree-creator-lc-to-p-k-pi
DOO2_TREE_LB=0      # hf-tree-creator-lb-to-lc-pi
DOO2_TREE_X=0       # hf-tree-creator-x-to-jpsi-pi-pi
DOO2_TREE_XICC=0    # hf-tree-creator-xicc-to-p-k-pi-pi
DOO2_TREE_CHIC=0    # hf-tree-creator-chic-to-jpsi-gamma
DOO2_TREE_BPLUS=0   # hf-tree-creator-bplus-to-d0-pi
# Correlations
DOO2_D0D0BAR_DATA=0       # hf-correlator-d0-d0bar
DOO2_D0D0BAR_MCREC=0      # hf-correlator-d0-d0bar-mc-rec
DOO2_D0D0BAR_MCGEN=0      # hf-correlator-d0-d0bar-mc-gen
DOO2_DPLUSDMINUS_DATA=0   # hf-correlator-dplus-dminus
DOO2_DPLUSDMINUS_MCREC=0  # hf-correlator-dplus-dminus-mc-rec
DOO2_DPLUSDMINUS_MCGEN=0  # hf-correlator-dplus-dminus-mc-gen
DOO2_CORRELATOR_D0HADRON=0    # hf-correlator-d0-hadrons
DOO2_TASK_D0HADRON=0          # hf-task-correlation-d0-hadrons
DOO2_TASK_FLOW=0    # hf-task-flow
# Other
DOO2_MCCONV=0       # mc-converter
DOO2_FDDCONV=0      # fdd-converter
DOO2_COLLCONV=0     # collision-converter
DOO2_ZDCCONV=1      # zdc-converter

# Selection cuts
APPLYCUTS_D0=1      # Apply D0 selection cuts.
APPLYCUTS_DS=0      # Apply Ds selection cuts.
APPLYCUTS_DPLUS=0   # Apply D+ selection cuts.
APPLYCUTS_LC=1      # Apply Λc selection cuts.
APPLYCUTS_LB=0      # Apply Λb selection cuts.
APPLYCUTS_XIC=0     # Apply Ξc selection cuts.
APPLYCUTS_JPSI=0    # Apply J/ψ selection cuts.
APPLYCUTS_X=0       # Apply X selection cuts.
APPLYCUTS_CHIC=0    # Apply χc(1p) selection cuts.
APPLYCUTS_LCK0SP=0  # Apply Λc → K0S p selection cuts.
APPLYCUTS_XICC=0    # Apply Ξcc selection cuts.
APPLYCUTS_B0=0      # Apply B0 selection cuts.
APPLYCUTS_BPLUS=0   # Apply B+ selection cuts.

SAVETREES=0         # Save O2 tables to trees.
USEO2VERTEXER=1     # Use the O2 vertexer in AliPhysics.
USEALIEVCUTS=1      # Use AliEventCuts in AliPhysics (as used by conversion task)
DORATIO=1           # Plot histogram ratios in comparison.

####################################################################################################

# Clean before (argument=1) and after (argument=2) running.
function Clean {
  # Cleanup before running
  [ "$1" -eq 1 ] && { bash "$DIR_TASKS/clean.sh" || ErrExit; }

  # Cleanup after running
  [ "$1" -eq 2 ] && {
    rm -f "$LISTFILES_ALI" "$LISTFILES_O2" "$SCRIPT_ALI" "$SCRIPT_O2" "$SCRIPT_POSTPROCESS" || ErrExit "Failed to rm created files."
    [ "$JSON_EDIT" ] && { rm "$JSON_EDIT" || ErrExit "Failed to rm $JSON_EDIT."; }
    [ "$DATABASE_O2_EDIT" ] && { rm "$DATABASE_O2_EDIT" || ErrExit "Failed to rm $DATABASE_O2_EDIT."; }
  }

  return 0
}

# Modify the JSON file.
function AdjustJson {
  # Make a copy of the default JSON file to modify it.
  JSON_EDIT="${JSON/.json/_edit.json}"
  cp "$JSON" "$JSON_EDIT" || ErrExit "Failed to cp $JSON $JSON_EDIT."
  JSON="$JSON_EDIT"

  # Collision system
  MsgWarn "Setting collision system $INPUT_SYS"

  # Run 2/3/5
  MsgWarn "Using Run $INPUT_RUN"
  if [ "$INPUT_RUN" -eq 2 ]; then
    ReplaceString "\"processRun2\": \"false\"" "\"processRun2\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"processRun3\": \"true\"" "\"processRun3\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
  elif [ "$INPUT_RUN" -eq 3 ]; then
    ReplaceString "\"processRun2\": \"true\"" "\"processRun2\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"processRun3\": \"false\"" "\"processRun3\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # MC
  if [ "$ISMC" -eq 1 ]; then
    MsgWarn "Using MC data"
    ReplaceString "\"processMc\": \"false\"" "\"processMc\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"processMC\": \"false\"" "\"processMC\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"isMC\": \"false\"" "\"isMC\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
  else
    MsgWarn "Using real data"
    ReplaceString "\"processMc\": \"true\"" "\"processMc\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"processMC\": \"true\"" "\"processMC\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"isMC\": \"true\"" "\"isMC\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # event-selection
  ReplaceString "\"syst\": \"pp\"" "\"syst\": \"$INPUT_SYS\"" "$JSON" || ErrExit "Failed to edit $JSON."

  # hf-track-index-skim-creator-tag-sel-collisions
  if [ $DOO2_TRIGSEL -eq 1 ]; then
    # trigger selection
    ReplaceString "\"processTrigSel\": \"false\"" "\"processTrigSel\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"processNoTrigSel\": \"true\"" "\"processNoTrigSel\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi
  if [ "$INPUT_RUN" -eq 3 ]; then
    # do not use trigger selection for Run 3
    ReplaceString "\"processTrigSel\": \"true\"" "\"processTrigSel\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"processNoTrigSel\": \"false\"" "\"processNoTrigSel\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # hf-track-index-skim-creator-tag-sel-tracks, hf-track-index-skim-creator-cascades
  if [ "$INPUT_RUN" -eq 3 ]; then
    # do not perform track quality cuts for Run 3 until they are updated
    ReplaceString "\"doCutQuality\": \"true\"" "\"doCutQuality\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # timestamp-task
  if [[ "$ISMC" -eq 1 && "$INPUT_RUN" -eq 2 ]]; then
    ReplaceString "\"isRun2MC\": \"false\"" "\"isRun2MC\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
  else
    ReplaceString "\"isRun2MC\": \"true\"" "\"isRun2MC\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # track-selection
  if [ "$INPUT_RUN" -eq 3 ]; then
    ReplaceString "\"isRun3\": \"false\"" "\"isRun3\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
  else
    ReplaceString "\"isRun3\": \"true\"" "\"isRun3\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # lambdakzero-builder
  if [ "$INPUT_RUN" -eq 2 ]; then
    ReplaceString "\"isRun2\": \"0\"" "\"isRun2\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  else
    ReplaceString "\"isRun2\": \"1\"" "\"isRun2\": \"0\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # hf-track-index-skim-creator..., hf-candidate-creator-...
  if [ "$INPUT_RUN" -eq 2 ]; then
    ReplaceString "\"isRun2\": \"false\"" "\"isRun2\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
  else
    ReplaceString "\"isRun2\": \"true\"" "\"isRun2\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # tof-event-time
  if [ "$INPUT_RUN" -eq 3 ]; then
    ReplaceString "\"processNoFT0\": \"false\"" "\"processNoFT0\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
  else
    ReplaceString "\"processNoFT0\": \"true\"" "\"processNoFT0\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # hf-task-flow
  if [ "$INPUT_RUN" -eq 3 ]; then
    ReplaceString "\"processSameRun3\": \"false\"" "\"processSameRun3\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"processSameRun2\": \"true\"" "\"processSameRun2\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
  else
    ReplaceString "\"processSameRun3\": \"true\"" "\"processSameRun3\": \"false\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"processSameRun2\": \"false\"" "\"processSameRun2\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable D0 selection.
  if [ $APPLYCUTS_D0 -eq 1 ]; then
    MsgWarn "Using D0 selection cuts"
    ReplaceString "\"selectionFlagD0\": \"0\"" "\"selectionFlagD0\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"selectionFlagD0bar\": \"0\"" "\"selectionFlagD0bar\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable Ds selection.
  if [ $APPLYCUTS_DS -eq 1 ]; then
    MsgWarn "Using Ds selection cuts"
    ReplaceString "\"selectionFlagDs\": \"0\"" "\"selectionFlagDs\": \"7\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable D+ selection.
  if [ $APPLYCUTS_DPLUS -eq 1 ]; then
    MsgWarn "Using D+ selection cuts"
    ReplaceString "\"selectionFlagDplus\": \"0\"" "\"selectionFlagDplus\": \"7\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable Λc selection.
  if [ $APPLYCUTS_LC -eq 1 ]; then
    MsgWarn "Using Λc selection cuts"
    ReplaceString "\"selectionFlagLc\": \"0\"" "\"selectionFlagLc\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable Λb selection.
  if [ $APPLYCUTS_LB -eq 1 ]; then
    MsgWarn "Using Λb selection cuts"
    ReplaceString "\"selectionFlagLb\": \"0\"" "\"selectionFlagLb\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable Ξc selection.
  if [ $APPLYCUTS_XIC -eq 1 ]; then
    MsgWarn "Using Ξc selection cuts"
    ReplaceString "\"selectionFlagXic\": \"0\"" "\"selectionFlagXic\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable J/ψ selection.
  if [ $APPLYCUTS_JPSI -eq 1 ]; then
    MsgWarn "Using J/ψ selection cuts"
    ReplaceString "\"selectionFlagJpsi\": \"0\"" "\"selectionFlagJpsi\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable X(3872) selection.
  if [ $APPLYCUTS_X -eq 1 ]; then
    MsgWarn "Using X(3872) selection cuts"
    ReplaceString "\"selectionFlagX\": \"0\"" "\"selectionFlagX\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable χc(1p) selection.
  if [ $APPLYCUTS_CHIC -eq 1 ]; then
    MsgWarn "Using χc(1p) selection cuts"
    ReplaceString "\"selectionFlagChic\": \"0\"" "\"selectionFlagChic\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable Λc → K0S p selection.
  if [ $APPLYCUTS_LCK0SP -eq 1 ]; then
    MsgWarn "Using Λc → K0S p selection cuts"
    ReplaceString "\"selectionFlagLcToK0sP\": \"0\"" "\"selectionFlagLcToK0sP\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
    ReplaceString "\"selectionFlagLcbarToK0sP\": \"0\"" "\"selectionFlagLcbarToK0sP\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable Ξcc selection.
  if [ $APPLYCUTS_XICC -eq 1 ]; then
    MsgWarn "Using Ξcc selection cuts"
    ReplaceString "\"selectionFlagXicc\": \"0\"" "\"selectionFlagXicc\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable B0 selection.
  if [ $APPLYCUTS_B0 -eq 1 ]; then
    MsgWarn "Using B0 selection cuts"
    ReplaceString "\"selectionFlagB0\": \"0\"" "\"selectionFlagB0\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable B+ selection.
  if [ $APPLYCUTS_BPLUS -eq 1 ]; then
    MsgWarn "Using B+ selection cuts"
    ReplaceString "\"selectionFlagBplus\": \"0\"" "\"selectionFlagBplus\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi
}

# Generate the O2 script containing the full workflow specification.
function MakeScriptO2 {
  # Suffix to distinguish versions of the track index skim creator in the workflow database
  SUFFIX_SKIM_MASK="_skimX" # suffix mask to be replaced in the workflow names
  SUFFIX_SKIM="" # the actual suffix to be used instead of the mask
  # Λc → K0S p cascade reconstruction
  [[ $DOO2_CAND_CASC -eq 1 || $DOO2_SEL_LCK0SP -eq 1 || $DOO2_TASK_LCK0SP -eq 1 ]] && SUFFIX_SKIM+="_v0"
  # Suffix to distinguish versions of the same workflow for different runs in the workflow database
  SUFFIX_RUN_MASK="_runX" # suffix mask to be replaced in the workflow names
  SUFFIX_RUN="_run${INPUT_RUN}" # the actual suffix to be used instead of the mask

  WORKFLOWS=""
  # Trigger selection
  [ $DOO2_TRIGSEL -eq 1 ] && WORKFLOWS+=" o2-analysis-event-selection"
  # QA
  [ $DOO2_REJ_ALICE3 -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-qa-pid-rejection"
  [ $DOO2_QA_EFF -eq 1 ] && WORKFLOWS+=" o2-analysis-qa-efficiency"
  [ $DOO2_QA_EVTRK -eq 1 ] && WORKFLOWS+=" o2-analysis-qa-event-track"
  [ $DOO2_MC_VALID -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-mc-validation"
  # PID
  [ $DOO2_PID_TPC -eq 1 ] && WORKFLOWS+=" o2-analysis-pid-tpc"
  [ $DOO2_PID_BAYES -eq 1 ] && WORKFLOWS+=" o2-analysis-pid-bayes"
  [ $DOO2_PID_TOF -eq 1 ] && WORKFLOWS+=" o2-analysis-pid-tof${SUFFIX_RUN}"
  [ $DOO2_PID_TOF_QA -eq 1 ] && WORKFLOWS+=" o2-analysis-pid-tof-qa-mc"
  # Vertexing
  [ $DOO2_SKIM -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-track-index-skim-creator${SUFFIX_SKIM}"
  [ $DOO2_CAND_2PRONG -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-2prong"
  [ $DOO2_CAND_3PRONG -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-3prong"
  [ $DOO2_CAND_LB -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-lb"
  [ $DOO2_CAND_X -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-x"
  [ $DOO2_CAND_CHIC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-chic"
  [ $DOO2_CAND_CASC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-cascade"
  [ $DOO2_CAND_XICC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-xicc"
  [ $DOO2_CAND_B0 -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-b0"
  [ $DOO2_CAND_BPLUS -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-bplus"
  [ $DOO2_CAND_DSTAR -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-dstar"
  # Selectors
  [ $DOO2_SEL_D0 -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-d0"
  [ $DOO2_SEL_JPSI -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-jpsi${SUFFIX_RUN}"
  [ $DOO2_SEL_DS -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-ds-to-k-k-pi"
  [ $DOO2_SEL_DPLUS -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-dplus-to-pi-k-pi"
  [ $DOO2_SEL_LC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-lc"
  [ $DOO2_SEL_LB -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-lb-to-lc-pi"
  [ $DOO2_SEL_XIC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-xic-to-p-k-pi"
  [ $DOO2_SEL_X -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-x-to-jpsi-pi-pi"
  [ $DOO2_SEL_CHIC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-chic-to-jpsi-gamma"
  [ $DOO2_SEL_LCK0SP -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-lc-to-k0s-p"
  [ $DOO2_SEL_XICC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-xicc-to-p-k-pi-pi"
  [ $DOO2_SEL_B0 -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-b0-to-d-pi"
  [ $DOO2_SEL_BPLUS -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-selector-bplus-to-d0-pi"
  # User tasks
  [ $DOO2_TASK_D0 -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-d0"
  [ $DOO2_TASK_JPSI -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-jpsi"
  [ $DOO2_TASK_DS -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-ds"
  [ $DOO2_TASK_DPLUS -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-dplus"
  [ $DOO2_TASK_LC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-lc"
  [ $DOO2_TASK_LB -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-lb"
  [ $DOO2_TASK_XIC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-xic"
  [ $DOO2_TASK_X -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-x"
  [ $DOO2_TASK_CHIC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-chic"
  [ $DOO2_TASK_LCK0SP -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-lc-to-k0s-p"
  [ $DOO2_TASK_XICC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-xicc"
  [ $DOO2_TASK_B0 -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-b0"
  [ $DOO2_TASK_BPLUS -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-bplus"
  # Correlations
  WF_CORR=""
  [ $DOO2_D0D0BAR_DATA -eq 1 ] && WF_CORR="o2-analysis-hf-correlator-d0-d0bar o2-analysis-hf-task-correlation-d-dbar"
  [ $DOO2_D0D0BAR_MCREC -eq 1 ] && WF_CORR="o2-analysis-hf-correlator-d0-d0bar-mc-rec o2-analysis-hf-task-correlation-d-dbar-mc-rec"
  [ $DOO2_D0D0BAR_MCGEN -eq 1 ] && WF_CORR="o2-analysis-hf-correlator-d0-d0bar-mc-gen o2-analysis-hf-task-correlation-d-dbar-mc-gen"
  [ $DOO2_DPLUSDMINUS_DATA -eq 1 ] && WF_CORR="o2-analysis-hf-correlator-dplus-dminus o2-analysis-hf-task-correlation-d-dbar"
  [ $DOO2_DPLUSDMINUS_MCREC -eq 1 ] && WF_CORR="o2-analysis-hf-correlator-dplus-dminus-mc-rec o2-analysis-hf-task-correlation-d-dbar-mc-rec"
  [ $DOO2_DPLUSDMINUS_MCGEN -eq 1 ] && WF_CORR="o2-analysis-hf-correlator-dplus-dminus-mc-gen o2-analysis-hf-task-correlation-d-dbar-mc-gen"
  [ "$WF_CORR" ] && WORKFLOWS+=" $WF_CORR"
  [ $DOO2_CORRELATOR_D0HADRON -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-correlator-d0-hadrons"
  [ $DOO2_TASK_D0HADRON -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-correlation-d0-hadrons"
  [ $DOO2_TASK_FLOW -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-flow"
  # Tree creators
  [ $DOO2_TREE_D0 -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-tree-creator-d0-to-k-pi"
  [ $DOO2_TREE_LC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-tree-creator-lc-to-p-k-pi"
  [ $DOO2_TREE_LB -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-tree-creator-lb-to-lc-pi"
  [ $DOO2_TREE_X -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-tree-creator-x-to-jpsi-pi-pi"
  [ $DOO2_TREE_XICC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-tree-creator-xicc-to-p-k-pi-pi"
  [ $DOO2_TREE_CHIC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-tree-creator-chic-to-jpsi-gamma"
  [ $DOO2_TREE_BPLUS -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-tree-creator-bplus-to-d0-pi"
  # Other
  [ $DOO2_MCCONV -eq 1 ] && WORKFLOWS+=" o2-analysis-mc-converter"
  [ $DOO2_FDDCONV -eq 1 ] && WORKFLOWS+=" o2-analysis-fdd-converter"
  [ $DOO2_COLLCONV -eq 1 ] && WORKFLOWS+=" o2-analysis-collision-converter"
  [ $DOO2_ZDCCONV -eq 1 ] && WORKFLOWS+=" o2-analysis-zdc-converter"

  # Translate options into arguments of the generating script.
  OPT_MAKECMD=""
  [ "$ISMC" -eq 1 ] && OPT_MAKECMD+=" --mc"
  [ "$DEBUG" -eq 1 ] && OPT_MAKECMD+=" -d"
  [ $SAVETREES -eq 1 ] && OPT_MAKECMD+=" -t"
  [ $MAKE_GRAPH -eq 1 ] && OPT_MAKECMD+=" -g"

  # Make a copy of the default workflow database file before modifying it.
  DATABASE_O2_EDIT="${DATABASE_O2/.yml/_edit.yml}"
  cp "$DATABASE_O2" "$DATABASE_O2_EDIT" || ErrExit "Failed to cp $DATABASE_O2 $DATABASE_O2_EDIT."
  DATABASE_O2="$DATABASE_O2_EDIT"

  # Replace the workflow version masks with the actual values in the workflow database.
  ReplaceString "$SUFFIX_RUN_MASK" "$SUFFIX_RUN" "$DATABASE_O2" || ErrExit "Failed to edit $DATABASE_O2."
  ReplaceString "$SUFFIX_SKIM_MASK" "$SUFFIX_SKIM" "$DATABASE_O2" || ErrExit "Failed to edit $DATABASE_O2."

  # Generate the O2 command.
  MAKECMD="python3 $DIR_EXEC/make_command_o2.py $DATABASE_O2 $OPT_MAKECMD"
  O2EXEC=$($MAKECMD -w "$WORKFLOWS")
  $MAKECMD -w "$WORKFLOWS" 1> /dev/null 2> /dev/null || ErrExit "Generating of O2 command failed."
  [ "$O2EXEC" ] || ErrExit "Nothing to do!"

  # Create the script with the full O2 command.
  cat << EOF > "$SCRIPT_O2"
#!/bin/bash
FileIn="\$1"
JSON="\$2"
$O2EXEC
EOF
}

function MakeScriptAli {
  ALIEXEC="root -b -q -l \"$DIR_TASKS/RunHFTaskLocal.C(\\\"\$FileIn\\\", \\\"\$JSON\\\", $ISMC, $USEO2VERTEXER, $USEALIEVCUTS)\""
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
    [ $DOO2_SKIM -eq 1 ] && OPT_COMPARE+=" events tracks skim "
    [ $DOO2_CAND_2PRONG -eq 1 ] && OPT_COMPARE+=" cand2 "
    [ $DOO2_CAND_3PRONG -eq 1 ] && OPT_COMPARE+=" cand3 "
    [ $DOO2_TASK_D0 -eq 1 ] && { OPT_COMPARE+=" d0 "; [ "$ISMC" -eq 1 ] && OPT_COMPARE+=" d0-mc "; }
    [ $DOO2_TASK_DS -eq 1 ] && OPT_COMPARE+=" ds "
    [ $DOO2_TASK_DPLUS -eq 1 ] && OPT_COMPARE+=" dplus "
    [ $DOO2_TASK_LC -eq 1 ] && { OPT_COMPARE+=" lc "; [ "$ISMC" -eq 1 ] && OPT_COMPARE+=" lc-mc-pt  lc-mc-prompt  lc-mc-nonprompt  lc-mc-eta  lc-mc-phi "; }
    [ $DOO2_TASK_XIC -eq 1 ] && OPT_COMPARE+=" xic "
    [ $DOO2_TASK_JPSI -eq 1 ] && OPT_COMPARE+=" jpsi "
    [ "$OPT_COMPARE" ] && POSTEXEC+=" && root -b -q -l \"$DIR_TASKS/Compare.C(\\\"\$FileO2\\\", \\\"\$FileAli\\\", \\\"$OPT_COMPARE\\\", $DORATIO)\""
  }
  # Plot particle reconstruction efficiencies.
  [[ $DOO2 -eq 1 && $ISMC -eq 1 ]] && {
    PARTICLES=""
    [ $DOO2_TASK_D0 -eq 1 ] && PARTICLES+=" d0 "
    [ $DOO2_TASK_DS -eq 1 ] && PARTICLES+=" ds "
    [ $DOO2_TASK_DPLUS -eq 1 ] && PARTICLES+=" dplus "
    [ $DOO2_TASK_LC -eq 1 ] && PARTICLES+=" lc "
    [ $DOO2_TASK_LB -eq 1 ] && PARTICLES+=" lb "
    [ $DOO2_TASK_XIC -eq 1 ] && PARTICLES+=" xic "
    [ $DOO2_TASK_JPSI -eq 1 ] && PARTICLES+=" jpsi "
    [ $DOO2_TASK_LCK0SP -eq 1 ] && PARTICLES+=" lc-tok0sP "
    [ $DOO2_TASK_XICC -eq 1 ] && PARTICLES+=" xicc-mc "
    [ $DOO2_TASK_B0 -eq 1 ] && PARTICLES+=" b0-mc "
    [ $DOO2_TASK_BPLUS -eq 1 ] && PARTICLES+=" bplus "
    [ "$PARTICLES" ] && POSTEXEC+=" && root -b -q -l \"$DIR_TASKS/PlotEfficiency.C(\\\"\$FileO2\\\", \\\"$PARTICLES\\\")\""
  }
  cat << EOF > "$SCRIPT_POSTPROCESS"
#!/bin/bash
FileO2="\$1"
FileAli="\$2"
$POSTEXEC
EOF
}
