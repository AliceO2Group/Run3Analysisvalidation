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
DATABASE_O2="workflows_dummy.yml"
MAKE_GRAPH=0        # Make topology graph.

# Activation of O2 workflows
# Trigger selection
DOO2_EVTSEL=1       # event-selection
DOO2_TRACKSEL=1     # trackselection

SAVETREES=0         # Save O2 tables to trees.

####################################################################################################

# Clean before (argument=1) and after (argument=2) running.
function Clean {
  # Cleanup before running
  [ "$1" -eq 1 ] && { bash "$DIR_TASKS/clean_dummy.sh" || ErrExit; }

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
  MsgWarn "Running AdjustJson"
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
}

# Generate the O2 script containing the full workflow specification.
function MakeScriptO2 {
  MsgWarn "Running MakeScriptO2"
 # Suffix to distinguish versions of the same workflow for different runs in the workflow database
  SUFFIX_RUN_MASK="_runX" # suffix mask to be replaced in the workflow names
  SUFFIX_RUN="_run${INPUT_RUN}" # the actual suffix to be used instead of the mask

  WORKFLOWS=""
  [ $DOO2_EVTSEL -eq 1 ] && WORKFLOWS+=" o2-analysis-event-selection"
  [ $DOO2_TRACKSEL -eq 1 ] && WORKFLOWS+=" o2-analysis-trackselection${SUFFIX_RUN}"

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
  MsgWarn "Running MakeScriptAli"
  ALIEXEC="echo \"Ali\""
  ALIEXEC+=" && cp \$(head -n 1 \"\$FileIn\") $FILEOUT"
  cat << EOF > "$SCRIPT_ALI"
#!/bin/bash
FileIn="\$1"
JSON="\$2"
$ALIEXEC
EOF
}

function MakeScriptPostprocess {
  MsgWarn "Running MakeScriptPostprocess"
  POSTEXEC="echo \"Postprocessing\""
  cat << EOF > "$SCRIPT_POSTPROCESS"
#!/bin/bash
FileO2="\$1"
FileAli="\$2"
$POSTEXEC
EOF
}
