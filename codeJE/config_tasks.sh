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

# Here you can select the AliPhysics and O2Physics Git branches to load. (You need to have them built with aliBuild.)
# BRANCH_ALI="master"
# ENV_ALI="alienv setenv AliPhysics/latest-${BRANCH_ALI}-o2 -c"
# BRANCH_O2="master"
# ENV_O2="alienv setenv O2Physics/latest-${BRANCH_O2}-o2 -c"

# Steps
DOCLEAN=1           # Delete created files (before and after running tasks).
DOCONVERT=1         # Convert AliESDs.root to AO2D.root.
DOALI=1             # Run AliPhysics tasks.
DOO2=1              # Run O2 tasks.
DOPOSTPROCESS=1     # Run output postprocessing. (Comparison plots. Requires DOALI=1 and/or DOO2=1)

# Disable incompatible steps.
[ "$INPUT_IS_O2" -eq 1 ] && { DOCONVERT=0; DOALI=0; }

DATABASE_O2="workflows.yml"  # Workflow specification database
MAKE_GRAPH=0                 # Make topology graph.
MAKE_PERF_STATS=0            # Produce performance profiling stats.

# Activation of O2 workflows
# Table producers
DOO2_JET_DERIVED=0  # je-jet-deriveddata-producer
DOO2_JET_FINDER=0   # je-jet-finder-data-charged
# Analysis tasks
# TODO: Add analysis tasks
# QA
DOO2_QA_EVTRK=0     # qa-event-track
DOO2_JET_VALID=1    # je-jet-validation-qa
# Converters (Consider setting these per input case via INPUT_TASK_CONFIG.)
DOO2_CONV_MC=0      # mc-converter
DOO2_CONV_FDD=0     # fdd-converter
DOO2_CONV_COLL=0    # collision-converter
DOO2_CONV_ZDC=0     # zdc-converter
DOO2_CONV_BC=0      # bc-converter
DOO2_CONV_TRKEX=0   # tracks-extra-converter
DOO2_CONV_V0=0      # v0converter

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

  # Derived AO2D input
  if [ "$INPUT_PARENT_MASK" ]; then
    ReplaceString "PARENT_PATH_MASK" "$INPUT_PARENT_MASK" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

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
    ReplaceString "\"processData\": \"false\"" "\"processData\": \"true\"" "$JSON" || ErrExit "Failed to edit $JSON."
  fi

  # event-selection
  ReplaceString "\"syst\": \"pp\"" "\"syst\": \"$INPUT_SYS\"" "$JSON" || ErrExit "Failed to edit $JSON."

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

  # Suffix to distinguish the workflows that run on derived data with parent access
  SUFFIX_DER_MASK="_derX" # suffix mask to be replaced in the workflow names
  [ "$INPUT_PARENT_MASK" ] && SUFFIX_DER="_derived" || SUFFIX_DER="" # the actual suffix to be used instead of the mask

  WORKFLOWS=""
  # Table producers
  [ $DOO2_JET_DERIVED -eq 1 ] && WORKFLOWS+=" o2-analysis-je-jet-deriveddata-producer"
  [ $DOO2_JET_FINDER -eq 1 ] && WORKFLOWS+=" o2-analysis-je-jet-finder-data-charged"
  # Analysis tasks
  # TODO: Add analysis tasks
  # QA
  [ $DOO2_QA_EVTRK -eq 1 ] && WORKFLOWS+=" o2-analysis-qa-event-track"
  [ $DOO2_JET_VALID -eq 1 ] && WORKFLOWS+=" o2-analysis-je-jet-validation-qa"
  # Converters
  [ $DOO2_CONV_MC -eq 1 ] && WORKFLOWS+=" o2-analysis-mc-converter"
  [ $DOO2_CONV_FDD -eq 1 ] && WORKFLOWS+=" o2-analysis-fdd-converter"
  [ $DOO2_CONV_COLL -eq 1 ] && WORKFLOWS+=" o2-analysis-collision-converter"
  [ $DOO2_CONV_ZDC -eq 1 ] && WORKFLOWS+=" o2-analysis-zdc-converter"
  [ $DOO2_CONV_BC -eq 1 ] && WORKFLOWS+=" o2-analysis-bc-converter"
  [ $DOO2_CONV_TRKEX -eq 1 ] && WORKFLOWS+=" o2-analysis-tracks-extra-converter"
  [ $DOO2_CONV_V0 -eq 1 ] && WORKFLOWS+=" o2-analysis-v0converter"

  # Translate options into arguments of the generating script.
  OPT_MAKECMD=""
  [ "$INPUT_IS_MC" -eq 1 ] && OPT_MAKECMD+=" --mc"
  [ "$DEBUG" -eq 1 ] && OPT_MAKECMD+=" -d"
  [ $SAVETREES -eq 1 ] && OPT_MAKECMD+=" -t"
  [ $MAKE_GRAPH -eq 1 ] && OPT_MAKECMD+=" -g"
  [ $MAKE_PERF_STATS -eq 1 ] && OPT_MAKECMD+=" -p"

  # Make a copy of the default workflow database file before modifying it.
  DATABASE_O2_EDIT="${DATABASE_O2/.yml/_edit.yml}"
  cp "$DATABASE_O2" "$DATABASE_O2_EDIT" || ErrExit "Failed to cp $DATABASE_O2 $DATABASE_O2_EDIT."
  DATABASE_O2="$DATABASE_O2_EDIT"

  # Replace the workflow version masks with the actual values in the workflow database.
  ReplaceString "$SUFFIX_RUN_MASK" "$SUFFIX_RUN" "$DATABASE_O2" || ErrExit "Failed to edit $DATABASE_O2."
  ReplaceString "$SUFFIX_DER_MASK" "$SUFFIX_DER" "$DATABASE_O2" || ErrExit "Failed to edit $DATABASE_O2."

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
  [[ $DOALI -eq 1 && $DOO2 -eq 1 ]] && {
    OPT_COMPARE=""
    [ $DOO2_JET_VALID -eq 1 ] && OPT_COMPARE+=" jets "
    [ $DOO2_JET_VALID -eq 1 ] && OPT_COMPARE+=" events "
    [ "$OPT_COMPARE" ] && POSTEXEC+=" && root -b -q -l \"$DIR_TASKS/Compare.C(\\\"\$FileO2\\\", \\\"\$FileAli\\\", \\\"$OPT_COMPARE\\\", $DORATIO)\""
  }
  cat << EOF > "$SCRIPT_POSTPROCESS"
#!/bin/bash
FileO2="\$1"
FileAli="\$2"
$POSTEXEC
EOF
}
