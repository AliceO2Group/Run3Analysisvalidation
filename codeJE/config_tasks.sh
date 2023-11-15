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
DOO2_TRIGSEL=1      # event-selection
# QA
DOO2_QA_EVTRK=0     # qa-event-track
# User tasks
DOO2_TASK_JETFINDER=1   # jet-finder
DOO2_TASK_JETQA=1   # jetqa
# Converters
DOO2_MCCONV=0       # mc-converter
DOO2_FDDCONV=0      # fdd-converter
DOO2_COLLCONV=0     # collision-converter
DOO2_ZDC=1          # zdc-converter
DOO2_TRACKSELE=1    # trackselection, dca, propagation
DOO2_TRACKEXTRA=1   # track-extra-converter
DOO2_BC=1           # BC
DOO2_DERIVED=1      # jet-deriveddata-producer

SAVETREES=0         # Save O2 tables to trees.
USEO2VERTEXER=0     # Use the O2 vertexer in AliPhysics.
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
    rm "$DATABASE_O2_EDIT" || ErrExit "Failed to rm $DATABASE_O2_EDIT."
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
  if [ "$INPUT_IS_MC" -eq 1 ]; then
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

  # timestamp-task
  if [[ "$INPUT_IS_MC" -eq 1 && "$INPUT_RUN" -eq 2 ]]; then
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

}

# Generate the O2 script containing the full workflow specification.
function MakeScriptO2 {
  # Suffix to distinguish versions of the same workflow for different runs in the workflow database
  SUFFIX_RUN_MASK="_runX" # suffix mask to be replaced in the workflow names
  SUFFIX_RUN="_run${INPUT_RUN}" # the actual suffix to be used instead of the mask

  WORKFLOWS=""
  # Trigger selection
  [ $DOO2_TRIGSEL -eq 1 ] && WORKFLOWS+=" o2-analysis-event-selection"
  # QA
  [ $DOO2_QA_EVTRK -eq 1 ] && WORKFLOWS+=" o2-analysis-qa-event-track"
  # User tasks
  [ $DOO2_TASK_JETFINDER -eq 1 ] && WORKFLOWS+=" o2-analysis-je-jet-finder"
  [ $DOO2_TASK_JETQA -eq 1 ] && WORKFLOWS+=" o2-analysis-je-jet-validation-qa"
  # Converters
  [ $DOO2_MCCONV -eq 1 ] && WORKFLOWS+=" o2-analysis-mc-converter"
  [ $DOO2_FDDCONV -eq 1 ] && WORKFLOWS+=" o2-analysis-fdd-converter"
  [ $DOO2_COLLCONV -eq 1 ] && WORKFLOWS+=" o2-analysis-collision-converter"
  [ $DOO2_ZDC -eq 1 ] && WORKFLOWS+=" o2-analysis-zdc-converter"
  [ $DOO2_TRACKEXTRA -eq 1 ] && WORKFLOWS+=" o2-analysis-tracks-extra-converter"
  [ $DOO2_BC -eq 1 ] && WORKFLOWS+=" o2-analysis-bc-converter"
  [ $DOO2_DERIVED -eq 1 ] && WORKFLOWS+=" o2-analysis-je-jet-deriveddata-producer"

  # Translate options into arguments of the generating script.
  OPT_MAKECMD=""
  [ "$INPUT_IS_MC" -eq 1 ] && OPT_MAKECMD+=" --mc"
  [ "$DEBUG" -eq 1 ] && OPT_MAKECMD+=" -d"
  [ $SAVETREES -eq 1 ] && OPT_MAKECMD+=" -t"
  [ $MAKE_GRAPH -eq 1 ] && OPT_MAKECMD+=" -g"

  # Make a copy of the default workflow database file before modifying it.
  DATABASE_O2_EDIT="${DATABASE_O2/.yml/_edit.yml}"
  cp "$DATABASE_O2" "$DATABASE_O2_EDIT" || ErrExit "Failed to cp $DATABASE_O2 $DATABASE_O2_EDIT."
  DATABASE_O2="$DATABASE_O2_EDIT"

  # Replace the workflow version masks with the actual values in the workflow database.
  ReplaceString "$SUFFIX_RUN_MASK" "$SUFFIX_RUN" "$DATABASE_O2" || ErrExit "Failed to edit $DATABASE_O2."

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
  ALIEXEC="root -b -q -l \"$DIR_TASKS/RunJetTaskLocal.C(\\\"\$FileIn\\\", \\\"\$JSON\\\", $INPUT_IS_MC, $USEO2VERTEXER, $USEALIEVCUTS)\""
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
  #[[ $DOALI -eq 1 && $DOO2 -eq 1 ]] && {
   [[ $DOPOSTPROCESS -eq 1 ]] && {
    OPT_COMPARE=""
    [ $DOO2_TASK_JETQA -eq 1 ] && OPT_COMPARE+=" jets "
    [ "$OPT_COMPARE" ] && POSTEXEC+=" && root -b -q -l \"$DIR_TASKS/Compare.C(\\\"\$FileO2\\\", \\\"\$FileAli\\\", \\\"$OPT_COMPARE\\\", $DORATIO)\""
  }
  cat << EOF > "$SCRIPT_POSTPROCESS"
#!/bin/bash
FileO2="\$1"
FileAli="\$2"
$POSTEXEC
EOF
}
