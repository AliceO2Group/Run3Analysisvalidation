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

DEBUG=1             # Print out more information.

####################################################################################################

# Clean before (argument=1) and after (argument=2) running.
function Clean {
  # Cleanup before running
  [ $1 -eq 1 ] && MsgWarn "Before"

  # Cleanup after running
  [ $1 -eq 2 ] && MsgWarn "After"

  return 0
}

# Modify the JSON file.
function AdjustJson {
  MsgWarn "Running AdjustJson"
}

# Generate the O2 script containing the full workflow specification.
function MakeScriptO2 {
  MsgWarn "Running MakeScriptO2"
  O2EXEC="echo \"O2\""
  O2EXEC+=" && cp \"\$FileIn\" $FILEOUT"
  # Create the script with the full O2 command.
  cat << EOF > $SCRIPT_O2
#!/bin/bash
FileIn="\$1"
JSON="\$2"
$O2EXEC
EOF
}

function MakeScriptAli {
  MsgWarn "Running MakeScriptAli"
  ALIEXEC="echo \"Ali\""
  ALIEXEC+=" && cp \"\$FileIn\" $FILEOUT"
  cat << EOF > $SCRIPT_ALI
#!/bin/bash
FileIn="\$1"
JSON="\$2"
$ALIEXEC
EOF
}

function MakeScriptPostprocess {
  MsgWarn "Running MakeScriptPostprocess"
  POSTEXEC="echo \"Postprocessing\""
  cat << EOF > $SCRIPT_POSTPROCESS
#!/bin/bash
FileO2="\$1"
FileAli="\$2"
$POSTEXEC
EOF
}
