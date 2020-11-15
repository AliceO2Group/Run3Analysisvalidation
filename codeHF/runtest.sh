#!/bin/bash

##############################################################################
# User settings

# Steps
DOCLEAN=1           # Delete created files (before and after running tasks).
DOCONVERT=1         # Convert AliESDs.root to AO2D.root.
DOALI=1             # Run AliPhysics tasks.
DOO2=1              # Run O2 tasks.
DOPOSTPROCESS=1     # Run output postprocessing. (Compare AliPhysics and O2 output.)

NFILESMAX=1         # Maximum number of processed input files. (Set to -0 to process all; to -N to process all but the last N files.)
SAVETREES=0         # Save O2 tables to trees.
DEBUG=0             # Print out more information.
##############################################################################

# Default settings
CONFIG_INPUT="config_input.sh"  # Input specification
CONFIG_TASKS="config_tasks.sh"  # Tasks configuration; provides functions: AdjustJson, MakeScriptO2
INPUT_CASE=-1                   # Input case
INPUT_LABEL="nothing"           # Input description
INPUT_DIR=""                    # Input directory
INPUT_FILES="*/AliESDs.root"    # Input file pattern
JSON=""                         # Tasks parameters
ISINPUTO2=0                     # Input files are in O2 format.
ISMC=0                          # Switch for MC input
TRIGGERSTRINGRUN2=""            # Run 2 trigger (not used)
TRIGGERBITRUN3=-1               # Run 3 trigger (not used)

# Load message formatting.
source messages.sh || { MsgErr "Error: Failed to load message formatting."; exit 1; }

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

# Step scripts
SCRIPT_O2="script_o2.sh"
SCRIPT_ALI="script_ali.sh"
SCRIPT_POSTPROCESS="script_postprocess.sh"

# Load input specification.
source "$CONFIG_INPUT" || { MsgErr "Error: Failed to load input specification."; exit 1; }

# Load tasks configuration.
source "$CONFIG_TASKS" || { MsgErr "Error: Failed to load tasks configuration."; exit 1; }

########## END OF CONFIGURATION ##########

########## START OF EXECUTION ##########

# Print out input description.
MsgStep "Processing case $INPUT_CASE: $INPUT_LABEL"

# Delete created files.
[ $DOCLEAN -eq 1 ] && { bash clean.sh || exit 1; }

# Generate list of input files.
[ $ISINPUTO2 -eq 1 ] && LISTFILES=$LISTFILES_O2 || LISTFILES=$LISTFILES_ALI
ls $INPUT_DIR/$INPUT_FILES | head -n $NFILESMAX > $LISTFILES
[[ ${PIPESTATUS[0]} -eq 0 || ${PIPESTATUS[0]} -eq 141 ]] || { MsgErr "Error: Failed to make a list of input files."; exit 1; }

# Modify the JSON file.
AdjustJson || { MsgErr "Error"; exit 1; }

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
  MsgStep "Running AliPhysics tasks... ($(cat $LISTFILES_ALI | wc -l) files)"
  rm -f $FILEOUT $FILEOUT_ALI || { MsgErr "Error"; exit 1; }
  MakeScriptAli || { MsgErr "Error"; exit 1; }
  $ENVALI bash ali_batch.sh $LISTFILES_ALI $JSON $SCRIPT_ALI $DEBUG || exit 1 # Run the batch script in the ALI environment.
  #$ENVALIO2 bash ali_batch.sh $LISTFILES_ALI $JSON $SCRIPT_ALI $DEBUG || exit 1 # Run the batch script in the ALI+O2 environment.
  mv $FILEOUT $FILEOUT_ALI || { MsgErr "Error"; exit 1; }
fi

# Run O2 tasks.
if [ $DOO2 -eq 1 ]; then
  [ -f "$LISTFILES_O2" ] || { MsgErr "\nO2 tasks: Error: File $LISTFILES_O2 does not exist."; exit 1; }
  MsgStep "Running O2 tasks... ($(cat $LISTFILES_O2 | wc -l) files)"
  rm -f $FILEOUT $FILEOUT_O2 || { MsgErr "Error"; exit 1; }
  MakeScriptO2 || { MsgErr "Error"; exit 1; }
  [ $SAVETREES -eq 1 ] || FILEOUT_TREES=""
  $ENVO2 bash o2_batch.sh $LISTFILES_O2 $JSON $SCRIPT_O2 $DEBUG $FILEOUT_TREES || exit 1 # Run the batch script in the O2 environment.
  mv $FILEOUT $FILEOUT_O2 || { MsgErr "Error"; exit 1; }
  [[ $SAVETREES -eq 1 && "$FILEOUT_TREES" ]] && { mv $FILEOUT_TREES $FILEOUT_TREES_O2 || { MsgErr "Error"; exit 1; } }
fi

# Run output postprocessing. (Compare AliPhysics and O2 output.)
if [ $DOPOSTPROCESS -eq 1 ]; then
  LOGFILE="log_postprocess.log"
  MsgStep "Postprocessing... (logfile: $LOGFILE)"
  ok=1
  for file in "$FILEOUT_ALI" "$FILEOUT_O2"; do
    [ -f "$file" ] || { MsgErr "Error: File $file does not exist."; ok=0; }
  done
  [ $ok -ne 1 ] && exit 1
  MakeScriptPostprocess || { MsgErr "Error"; exit 1; }
  [ -f "$SCRIPT_POSTPROCESS" ] || { MsgErr "Error: File $SCRIPT_POSTPROCESS does not exist."; exit 1; }
  $ENVALI bash $SCRIPT_POSTPROCESS "$FILEOUT_O2" "$FILEOUT_ALI" > $LOGFILE 2>&1 || { MsgErr "Error"; exit 1; }
fi

# Delete created files.
[ $DOCLEAN -eq 1 ] && {
  rm -f $LISTFILES_ALI && \
  rm -f $LISTFILES_O2 && \
  rm -f $SCRIPT_ALI && \
  rm -f $SCRIPT_O2 && \
  rm -f $SCRIPT_POSTPROCESS || { MsgErr "Error"; exit 1; }
  [ "$JSON_EDIT" ] && { rm "$JSON_EDIT" || { MsgErr "Error"; exit 1; } }
}

MsgStep "Done"

exit 0
