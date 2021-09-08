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

# O2 database
DATABASE_O2="workflows.yml"
MAKE_GRAPH=0        # Make topology graph.

# Activation of O2 workflows
# Event selection
DOO2_EVSEL=0        # event-selection and timestamp
# QA
DOO2_REJ_ALICE3=0   # qa-rejection
DOO2_QA_EFF=0       # qa-efficiency
DOO2_QA_EVTRK=0     # qa-event-track
DOO2_MC_VALID=0     # hf-mc-validation
# PID
DOO2_PID_TPC=0      # pid-tpc-full
DOO2_PID_TOF=0      # pid-tof-full
DOO2_PID_TOF_QA=0   # pid-tof-qa-mc
# Vertexing
DOO2_SKIM=0         # hf-track-index-skims-creator
DOO2_CAND_2PRONG=0  # hf-candidate-creator-2prong
DOO2_CAND_3PRONG=0  # hf-candidate-creator-3prong
DOO2_CAND_CASC=0    # hf-candidate-creator-cascade
DOO2_CAND_X=0       # hf-candidate-creator-x
DOO2_CAND_XICC=0    # hf-candidate-creator-xicc
# Selectors
DOO2_SEL_D0=0       # hf-d0-candidate-selector
DOO2_SEL_DPLUS=0    # hf-dplus-topikpi-candidate-selector
DOO2_SEL_LC=0       # hf-lc-candidate-selector
DOO2_SEL_XIC=0      # hf-xic-topkpi-candidate-selector
DOO2_SEL_JPSI=0     # hf-jpsi-candidate-selector
DOO2_SEL_X=0        # hf-xic-topkpi-candidate-selector
DOO2_SEL_LCK0SP=0   # hf-lc-tok0sp-candidate-selector
DOO2_SEL_XICC=0     # hf-xicc-topkpipi-candidate-selector
# User tasks
DOO2_TASK_D0=1      # hf-task-d0
DOO2_TASK_DPLUS=0   # hf-task-dplus
DOO2_TASK_LC=0      # hf-task-lc
DOO2_TASK_XIC=0     # hf-task-xic
DOO2_TASK_JPSI=0    # hf-task-jpsi
DOO2_TASK_BPLUS=0   # hf-task-bplus
DOO2_TASK_X=0       # hf-task-x
DOO2_TASK_LCK0SP=0  # hf-task-lc-tok0sp
DOO2_TASK_XICC=0    # hf-task-xicc
# Tree creators
DOO2_TREE_D0=0      # hf-tree-creator-d0-tokpi
DOO2_TREE_LC=0      # hf-tree-creator-lc-topkpi
DOO2_TREE_X=0       # hf-tree-creator-x-tojpsipipi
DOO2_TREE_XICC=0    # hf-tree-creator-xicc-topkpipi
# Correlations
DOO2_D0D0BAR_DATA=0      # hf-correlator-d0d0bar
DOO2_D0D0BAR_MCREC=0     # hf-correlator-d0d0bar-mc-rec
DOO2_D0D0BAR_MCGEN=0     # hf-correlator-d0d0bar-mc-gen
DOO2_DPLUSDMINUS_DATA=0  # hf-correlator-dplusdminus
DOO2_DPLUSDMINUS_MCREC=0 # hf-correlator-dplusdminus-mc-rec
DOO2_DPLUSDMINUS_MCGEN=0 # hf-correlator-dplusdminus-mc-gen

# Selection cuts
APPLYCUTS_D0=0      # Apply D0 selection cuts.
APPLYCUTS_DPLUS=0   # Apply D+ selection cuts.
APPLYCUTS_LC=0      # Apply Λc selection cuts.
APPLYCUTS_XIC=0     # Apply Ξc selection cuts.
APPLYCUTS_JPSI=0    # Apply J/ψ selection cuts.
APPLYCUTS_X=0       # Apply X selection cuts.
APPLYCUTS_LCK0SP=0  # Apply Λc → K0S p selection cuts.
APPLYCUTS_XICC=0    # Apply Ξcc selection cuts.

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
    [ "$DATABASE_O2_EDIT" ] && { rm "$DATABASE_O2_EDIT" || ErrExit "Failed to rm $DATABASE_O2_EDIT."; }
  }

  return 0
}

# Modify the JSON file.
function AdjustJson {
  # Make a copy of the default JSON file to modify it.
  JSON_EDIT=""
  if [[ $APPLYCUTS_D0 -eq 1 || $APPLYCUTS_DPLUS -eq 1 || $APPLYCUTS_LC -eq 1 || $APPLYCUTS_XIC -eq 1 || $APPLYCUTS_JPSI -eq 1 || $APPLYCUTS_LCK0SP -eq 1 || $APPLYCUTS_X -eq 1 || $APPLYCUTS_XICC -eq 1 ]]; then
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

  # Enable D+ selection.
  if [ $APPLYCUTS_DPLUS -eq 1 ]; then
    MsgWarn "\nUsing D+ selection cuts"
    ReplaceString "\"d_selectionFlagDPlus\": \"0\"" "\"d_selectionFlagDPlus\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable Λc selection.
  if [ $APPLYCUTS_LC -eq 1 ]; then
    MsgWarn "\nUsing Λc selection cuts"
    ReplaceString "\"d_selectionFlagLc\": \"0\"" "\"d_selectionFlagLc\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable Ξc selection.
  if [ $APPLYCUTS_XIC -eq 1 ]; then
    MsgWarn "\nUsing Ξc selection cuts"
    ReplaceString "\"d_selectionFlagXic\": \"0\"" "\"d_selectionFlagXic\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable J/ψ selection.
  if [ $APPLYCUTS_JPSI -eq 1 ]; then
    MsgWarn "\nUsing J/ψ selection cuts"
    ReplaceString "\"d_selectionFlagJpsi\": \"0\"" "\"d_selectionFlagJpsi\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable X(3872) selection.
  if [ $APPLYCUTS_X -eq 1 ]; then
    MsgWarn "\nUsing X(3872) selection cuts"
    ReplaceString "\"d_selectionFlagX\": \"0\"" "\"d_selectionFlagX\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable Λc → K0S p selection.
  if [ $APPLYCUTS_LCK0SP -eq 1 ]; then
    MsgWarn "\nUsing Λc → K0S p selection cuts"
    ReplaceString "\"selectionFlagLcK0sp\": \"0\"" "\"selectionFlagLcK0sp\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # Enable Ξcc selection.
  if [ $APPLYCUTS_XICC -eq 1 ]; then
    MsgWarn "\nUsing Ξcc selection cuts"
    ReplaceString "\"d_selectionFlagXicc\": \"0\"" "\"d_selectionFlagXicc\": \"1\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi
}

# Generate the O2 script containing the full workflow specification.
function MakeScriptO2 {
  # Enable cascade reconstruction in case of Λc → K0S p tasks
  [[ $DOO2_CAND_CASC -eq 1 || $DOO2_SEL_LCK0SP -eq 1 || $DOO2_TASK_LCK0SP -eq 1 ]] && DOO2_CASC=1 || DOO2_CASC=0
  # Cascade reconstruction
  [ $DOO2_CASC -eq 1 ] && SUFFIX_CASC="-v0" || SUFFIX_CASC=""
  # Event selection
  [ $DOO2_EVSEL -eq 1 ] && SUFFIX_EVSEL="-evsel" || SUFFIX_EVSEL=""
  # ALICE 3 input
  [ "$ISALICE3" -eq 1 ] && SUFFIX_ALICE3="-alice3" || SUFFIX_ALICE3=""

  WORKFLOWS=""
  # QA
  [ $DOO2_REJ_ALICE3 -eq 1 ] && WORKFLOWS+=" o2-analysis-qa-rejection"
  [ $DOO2_QA_EFF -eq 1 ] && WORKFLOWS+=" o2-analysis-qa-efficiency"
  [ $DOO2_QA_EVTRK -eq 1 ] && WORKFLOWS+=" o2-analysis-qa-event-track"
  [ $DOO2_MC_VALID -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-mc-validation"
  # PID
  [ $DOO2_PID_TPC -eq 1 ] && WORKFLOWS+=" o2-analysis-pid-tpc-full"
  [ $DOO2_PID_TOF -eq 1 ] && WORKFLOWS+=" o2-analysis-pid-tof-full"
  [ $DOO2_PID_TOF_QA -eq 1 ] && WORKFLOWS+=" o2-analysis-pid-tof-qa-mc"
  # Vertexing
  WF_SKIM="o2-analysis-hf-track-index-skims-creator"
  [ $DOO2_SKIM -eq 1 ] && WORKFLOWS+=" ${WF_SKIM}${SUFFIX_EVSEL}${SUFFIX_CASC}"
  [ $DOO2_CAND_2PRONG -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-2prong"
  [ $DOO2_CAND_3PRONG -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-3prong"
  [ $DOO2_CAND_X -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-x"
  [ $DOO2_CAND_CASC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-cascade"
  [ $DOO2_CAND_XICC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-candidate-creator-xicc"
  # Selectors
  [ $DOO2_SEL_D0 -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-d0-candidate-selector"
  WF_SEL_JPSI="o2-analysis-hf-jpsi-candidate-selector"
  [ $DOO2_SEL_JPSI -eq 1 ] && WORKFLOWS+=" ${WF_SEL_JPSI}${SUFFIX_ALICE3}"
  [ $DOO2_SEL_DPLUS -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-dplus-topikpi-candidate-selector"
  [ $DOO2_SEL_LC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-lc-candidate-selector"
  [ $DOO2_SEL_XIC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-xic-topkpi-candidate-selector"
  [ $DOO2_SEL_X -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-x-tojpsipipi-candidate-selector"
  [ $DOO2_SEL_LCK0SP -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-lc-tok0sp-candidate-selector"
  [ $DOO2_SEL_XICC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-xicc-topkpipi-candidate-selector"
  # User tasks
  [ $DOO2_TASK_D0 -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-d0"
  [ $DOO2_TASK_JPSI -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-jpsi"
  [ $DOO2_TASK_DPLUS -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-dplus"
  [ $DOO2_TASK_LC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-lc"
  [ $DOO2_TASK_XIC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-xic"
  [ $DOO2_TASK_BPLUS -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-bplus"
  [ $DOO2_TASK_X -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-x"
  [ $DOO2_TASK_LCK0SP -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-lc-tok0sp"
  [ $DOO2_TASK_XICC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-task-xicc"
  WF_CORR=""
  [ $DOO2_D0D0BAR_DATA -eq 1 ] && WF_CORR="o2-analysis-hf-correlator-d0d0bar o2-analysis-hf-task-correlation-ddbar"
  [ $DOO2_D0D0BAR_MCREC -eq 1 ] && WF_CORR="o2-analysis-hf-correlator-d0d0bar-mc-rec o2-analysis-hf-task-correlation-ddbar-mc-rec"
  [ $DOO2_D0D0BAR_MCGEN -eq 1 ] && WF_CORR="o2-analysis-hf-correlator-d0d0bar-mc-gen o2-analysis-hf-task-correlation-ddbar-mc-gen"
  [ $DOO2_DPLUSDMINUS_DATA -eq 1 ] && WF_CORR="o2-analysis-hf-correlator-dplusdminus o2-analysis-hf-task-correlation-ddbar"
  [ $DOO2_DPLUSDMINUS_MCREC -eq 1 ] && WF_CORR="o2-analysis-hf-correlator-dplusdminus-mc-rec o2-analysis-hf-task-correlation-ddbar-mc-rec"
  [ $DOO2_DPLUSDMINUS_MCGEN -eq 1 ] && WF_CORR="o2-analysis-hf-correlator-dplusdminus-mc-gen o2-analysis-hf-task-correlation-ddbar-mc-gen"
  [[ $DOO2_D0D0BAR_DATA -eq 1 || $DOO2_D0D0BAR_MCREC -eq 1 || $DOO2_D0D0BAR_MCGEN -eq 1 || $DOO2_DPLUSDMINUS_DATA -eq 1 || $DOO2_DPLUSDMINUS_MCREC -eq 1 || $DOO2_DPLUSDMINUS_MCGEN -eq 1 ]] && WORKFLOWS+=" ${WF_CORR}"
  # Tree creators
  [ $DOO2_TREE_D0 -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-tree-creator-d0-tokpi"
  [ $DOO2_TREE_LC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-tree-creator-lc-topkpi"
  [ $DOO2_TREE_X -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-tree-creator-x-tojpsipipi"
  [ $DOO2_TREE_XICC -eq 1 ] && WORKFLOWS+=" o2-analysis-hf-tree-creator-xicc-topkpipi"

  # Translate options into arguments of the generating script.
  OPT_MAKECMD=""
  [ "$ISMC" -eq 1 ] && OPT_MAKECMD+=" --mc"
  [ "$DEBUG" -eq 1 ] && OPT_MAKECMD+=" -d"
  [ $SAVETREES -eq 1 ] && OPT_MAKECMD+=" -t"
  [ $MAKE_GRAPH -eq 1 ] && OPT_MAKECMD+=" -g"

  # Make a copy of the default workflow database file before modifying it.
  DATABASE_O2_EDIT=""
  if [[ $DOO2_EVSEL -eq 1 || $DOO2_CASC -eq 1 || "$ISALICE3" -eq 1 ]]; then
    DATABASE_O2_EDIT="${DATABASE_O2/.yml/_edit.yml}"
    cp "$DATABASE_O2" "$DATABASE_O2_EDIT" || ErrExit "Failed to cp $DATABASE_O2 $DATABASE_O2_EDIT."
    DATABASE_O2="$DATABASE_O2_EDIT"

    # Adjust workflow database in case of ALICE 3 input.
    [ "$ISALICE3" -eq 1 ] && {
      ReplaceString "- $WF_SEL_JPSI" "- ${WF_SEL_JPSI}${SUFFIX_ALICE3}" "$DATABASE_O2" || ErrExit "Failed to edit $DATABASE_O2."
    }

    # Adjust workflow database in case of event selection or cascades enabled.
    [[ $DOO2_EVSEL -eq 1 || $DOO2_CASC -eq 1 ]] && {
      ReplaceString "- $WF_SKIM" "- ${WF_SKIM}${SUFFIX_EVSEL}${SUFFIX_CASC}" "$DATABASE_O2" || ErrExit "Failed to edit $DATABASE_O2."
    }
  fi

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
mkdir sockets && \\
$O2EXEC && \\
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
    [ $DOO2_SKIM -eq 1 ] && OPT_COMPARE+=" tracks skim "
    [ $DOO2_CAND_2PRONG -eq 1 ] && OPT_COMPARE+=" cand2 "
    [ $DOO2_CAND_3PRONG -eq 1 ] && OPT_COMPARE+=" cand3 "
    [ $DOO2_TASK_D0 -eq 1 ] && { OPT_COMPARE+=" d0 "; [ "$ISMC" -eq 1 ] && OPT_COMPARE+=" d0-mc "; }
    [ $DOO2_TASK_DPLUS -eq 1 ] && OPT_COMPARE+=" dplus "
    [ $DOO2_TASK_LC -eq 1 ] && { OPT_COMPARE+=" lc "; [ "$ISMC" -eq 1 ] && OPT_COMPARE+=" lc-mc "; }
    [ $DOO2_TASK_XIC -eq 1 ] && OPT_COMPARE+=" xic "
    [ $DOO2_TASK_JPSI -eq 1 ] && OPT_COMPARE+=" jpsi "
    [ "$OPT_COMPARE" ] && POSTEXEC+=" && root -b -q -l \"$DIR_TASKS/Compare.C(\\\"\$FileO2\\\", \\\"\$FileAli\\\", \\\"$OPT_COMPARE\\\", $DORATIO)\""
  }
  # Plot particle reconstruction efficiencies.
  [[ $DOO2 -eq 1 && $ISMC -eq 1 ]] && {
    PARTICLES=""
    [ $DOO2_TASK_D0 -eq 1 ] && PARTICLES+=" d0 "
    [ $DOO2_TASK_DPLUS -eq 1 ] && PARTICLES+=" dplus "
    [ $DOO2_TASK_LC -eq 1 ] && PARTICLES+=" lc "
    [ $DOO2_TASK_XIC -eq 1 ] && PARTICLES+=" xic "
    [ $DOO2_TASK_JPSI -eq 1 ] && PARTICLES+=" jpsi "
    [ $DOO2_TASK_LCK0SP -eq 1 ] && PARTICLES+=" lc-tok0sP "
    [ $DOO2_TASK_XICC -eq 1 ] && PARTICLES+=" xicc "
    [ "$PARTICLES" ] && POSTEXEC+=" && root -b -q -l \"$DIR_TASKS/PlotEfficiency.C(\\\"\$FileO2\\\", \\\"$PARTICLES\\\")\""
  }
  cat << EOF > "$SCRIPT_POSTPROCESS"
#!/bin/bash
FileO2="\$1"
FileAli="\$2"
$POSTEXEC
EOF
}
