#!/bin/bash

# Steering script to run Run 2 to Run 3 conversion, AliPhysics tasks, O2 tasks, and postprocessing

####################################################################################################
# Default settings

# Steps
DOCLEAN=1        # Delete created files (before and after running tasks).
DOCONVERT=1      # Convert AliESDs.root to AO2D.root.
DOALI=1          # Run AliPhysics tasks.
DOO2=1           # Run O2 tasks.
DOPOSTPROCESS=1  # Run output postprocessing. (Compare AliPhysics and O2 output.)

# Configuration scripts
CONFIG_INPUT="config_input.sh"  # Input specification (Modifies input parameters.)
CONFIG_TASKS="config_tasks.sh"  # Tasks configuration (Cleans directory, modifies step activation, modifies JSON and generates step scripts via functions Clean, AdjustJson, MakeScriptAli, MakeScriptO2, MakeScriptPostprocess.)

# Input parameters
INPUT_CASE=-1                   # Input case
INPUT_LABEL="nothing"           # Input description
INPUT_DIR="$PWD"                # Input directory
INPUT_FILES="*/AliESDs.root"    # Input file pattern
JSON="dpl-config.json"          # Tasks parameters
ISINPUTO2=0                     # Input files are in O2 format.
ISMC=0                          # Input files are MC data.
TRIGGERSTRINGRUN2=""            # Run 2 trigger (not used)
TRIGGERBITRUN3=-1               # Run 3 trigger (not used)
NFILESMAX=1                     # Maximum number of processed input files. (Set to -0 to process all; to -N to process all but the last N files.)

# Other options
SAVETREES=0                     # Save O2 tables to trees.
DEBUG=0                         # Print out more information.

# This directory
DIR_EXEC="$(dirname $(realpath $0))"

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

# Step scripts
SCRIPT_O2="script_o2.sh"
SCRIPT_ALI="script_ali.sh"
SCRIPT_POSTPROCESS="script_postprocess.sh"

# Load utilities.
source "$DIR_EXEC/utilities.sh" || { echo "Error: Failed to load utilities."; exit 1; }

# Parse command line options.
function Help { echo "Usage: bash $0 [-i <input config>] [-t <task config>] "; }
while getopts ":hi:t:" opt; do
  case ${opt} in
    h)
      Help; exit 0;;
    i)
      CONFIG_INPUT="$OPTARG";;
    t)
      CONFIG_TASKS="$OPTARG";;
    \?)
      MsgErr "Invalid option: $OPTARG" 1>&2; Help; exit 1;;
    :)
      MsgErr "Invalid option: $OPTARG requires an argument." 1>&2; Help; exit 1;;
  esac
done

# Load input specification.
source "$CONFIG_INPUT" || ErrExit "Failed to load input specification."

# Load tasks configuration.
source "$CONFIG_TASKS" || ErrExit "Failed to load tasks configuration."
DIR_TASKS="$(dirname $(realpath $CONFIG_TASKS))"

########## END OF CONFIGURATION ##########

####################################################################################################

########## START OF EXECUTION ##########

[ $DEBUG -eq 1 ] && { echo "$0"; echo "Input specification: $CONFIG_INPUT"; echo "Tasks configuration: $CONFIG_TASKS"; }

# Print out input description.
MsgStep "Processing case $INPUT_CASE: $INPUT_LABEL"

# Clean before running.
if [ $DOCLEAN -eq 1 ]; then
  MsgStep "Cleaning..."
  Clean 1 || ErrExit "Clean failed."
fi

# Generate list of input files.
MsgStep "Generating list of input files..."
[ $ISINPUTO2 -eq 1 ] && LISTFILES=$LISTFILES_O2 || LISTFILES=$LISTFILES_ALI
ls $INPUT_DIR/$INPUT_FILES | head -n $NFILESMAX > $LISTFILES
[[ ${PIPESTATUS[0]} -eq 0 || ${PIPESTATUS[0]} -eq 141 ]] || ErrExit "Failed to make a list of input files."

# Modify the JSON file.
MsgStep "Modifying JSON file..."
CheckFile "$JSON"
AdjustJson || ErrExit "AdjustJson failed."
CheckFile "$JSON"

# Convert AliESDs.root to AO2D.root.
if [ $DOCONVERT -eq 1 ]; then
  CheckFile "$LISTFILES_ALI"
  MsgStep "Converting... ($(cat $LISTFILES_ALI | wc -l) files)"
  [ $ISMC -eq 1 ] && MsgWarn "Using MC mode"
  [ $DEBUG -eq 1 ] && echo "Loading AliPhysics..."
  $ENVALI bash "$DIR_EXEC/batch_convert.sh" $LISTFILES_ALI $LISTFILES_O2 $ISMC $DEBUG || exit 1 # Run the batch script in the ALI environment.
fi

# Run AliPhysics tasks.
if [ $DOALI -eq 1 ]; then
  CheckFile "$LISTFILES_ALI"
  MsgStep "Running AliPhysics tasks... ($(cat $LISTFILES_ALI | wc -l) files)"
  rm -f $FILEOUT $FILEOUT_ALI || ErrExit "Failed to rm $FILEOUT $FILEOUT_ALI."
  MakeScriptAli || ErrExit "MakeScriptAli failed."
  CheckFile "$SCRIPT_ALI"
  [ $DEBUG -eq 1 ] && echo "Loading AliPhysics..."
  $ENVALI bash "$DIR_EXEC/batch_ali.sh" $LISTFILES_ALI $JSON $SCRIPT_ALI $DEBUG || exit 1 # Run the batch script in the ALI environment.
  #$ENVALIO2 bash "$DIR_EXEC/batch_ali.sh" $LISTFILES_ALI $JSON $SCRIPT_ALI $DEBUG || exit 1 # Run the batch script in the ALI+O2 environment.
  mv $FILEOUT $FILEOUT_ALI || ErrExit "Failed to mv $FILEOUT $FILEOUT_ALI."
fi

# Run O2 tasks.
if [ $DOO2 -eq 1 ]; then
  CheckFile "$LISTFILES_O2"
  MsgStep "Running O2 tasks... ($(cat $LISTFILES_O2 | wc -l) files)"
  rm -f $FILEOUT $FILEOUT_O2 || ErrExit "Failed to rm $FILEOUT $FILEOUT_O2."
  MakeScriptO2 || ErrExit "MakeScriptO2 failed."
  CheckFile "$SCRIPT_O2"
  [ $SAVETREES -eq 1 ] || FILEOUT_TREES=""
  [ $DEBUG -eq 1 ] && echo "Loading O2..."
  $ENVO2 bash "$DIR_EXEC/batch_o2.sh" $LISTFILES_O2 $JSON $SCRIPT_O2 $DEBUG $FILEOUT_TREES || exit 1 # Run the batch script in the O2 environment.
  mv $FILEOUT $FILEOUT_O2 || ErrExit "Failed to mv $FILEOUT $FILEOUT_O2."
  [[ $SAVETREES -eq 1 && "$FILEOUT_TREES" ]] && { mv $FILEOUT_TREES $FILEOUT_TREES_O2 || ErrExit "Failed to mv $FILEOUT_TREES $FILEOUT_TREES_O2."; }
fi

# Run output postprocessing. (Compare AliPhysics and O2 output.)
if [ $DOPOSTPROCESS -eq 1 ]; then
  LOGFILE="log_postprocess.log"
  MsgStep "Postprocessing... (logfile: $LOGFILE)"
  MakeScriptPostprocess || ErrExit "MakeScriptPostprocess failed."
  CheckFile "$SCRIPT_POSTPROCESS"
  [ $DEBUG -eq 1 ] && echo "Loading AliPhysics..."
  $ENVALI bash "$SCRIPT_POSTPROCESS" "$FILEOUT_O2" "$FILEOUT_ALI" > $LOGFILE 2>&1 || ErrExit "\nCheck $(realpath $LOGFILE)"
fi

# Clean after running.
if [ $DOCLEAN -eq 1 ]; then
  MsgStep "Cleaning..."
  Clean 2 || ErrExit "Clean failed."
fi

MsgStep "Done"

exit 0
