#!/bin/bash
# shellcheck disable=SC1090,SC1091,SC2034 # Ignore non-constant source, not following, unused triggers and DIR_TASKS.

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
INPUT_FILES="AliESDs.root"      # Input file pattern
INPUT_SYS="pp"                  # Collision system
INPUT_RUN=2                     # LHC Run (2, 3, 5)
JSON="dpl-config.json"          # Tasks parameters
ISINPUTO2=0                     # Input files are in O2 format.
ISMC=0                          # Input files are MC data.
ISALICE3=0                      # Input data from the ALICE 3 detectors.
NFILESMAX=1                     # Maximum number of processed input files. (Set to -0 to process all; to -N to process all but the last N files.)
NFILESPERJOB_CONVERT=1          # Number of input files per conversion job
NFILESPERJOB_ALI=1              # Number of input files per AliPhysics job
NFILESPERJOB_O2=1               # Number of input files per O2 job

# Other options
SAVETREES=0                     # Save O2 tables to trees.
DEBUG=0                         # Print out more information.

# Performance
NCORES=$(nproc)                 # Ideal number of used cores
NCORESPERJOB_ALI=1              # Average number of cores used by one AliPhysics job
NCORESPERJOB_O2=1.6             # Average number of cores used by one O2 job
NJOBSPARALLEL_O2=$(nproc)       # Maximum number of simultaneously running O2 jobs

# This directory
DIR_EXEC="$(dirname "$(realpath "$0")")"

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
ENVO2="alienv setenv O2Physics/latest -c"
ENVPOST="alienv setenv ROOT/latest -c"

# Step scripts
SCRIPT_O2="script_o2.sh"
SCRIPT_ALI="script_ali.sh"
SCRIPT_POSTPROCESS="script_postprocess.sh"

# Load utilities.
source "$DIR_EXEC/utilities.sh" || { echo "Error: Failed to load utilities."; exit 1; }

# Parse command line options.
function Help { echo "Usage: bash [<path>/]$(basename "$0") [-h] [-i <input config>] [-t <task config>] [-d]"; }
while getopts ":hi:t:d" opt; do
  case ${opt} in
    h)
      Help; exit 0;;
    i)
      CONFIG_INPUT="$OPTARG";;
    t)
      CONFIG_TASKS="$OPTARG";;
    d)
      DEBUG=1;;
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
DIR_TASKS="$(dirname "$(realpath "$CONFIG_TASKS")")"

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
[ $ISINPUTO2 -eq 1 ] && LISTFILES="$LISTFILES_O2" || LISTFILES="$LISTFILES_ALI"
INPUT_DIR="$(realpath "$INPUT_DIR")"
[ $DEBUG -eq 1 ] && { echo "Searching for $INPUT_FILES in $INPUT_DIR"; }
find "$INPUT_DIR" -name "$INPUT_FILES" | sort | head -n $NFILESMAX > "$LISTFILES"
[[ ${PIPESTATUS[0]} -eq 0 || ${PIPESTATUS[0]} -eq 141 ]] || ErrExit "Failed to make a list of input files."
[ "$(wc -l < "$LISTFILES")" -eq 0 ] && { ErrExit "No input files!"; }

# Modify the JSON file.
MsgStep "Modifying JSON file..."
CheckFile "$JSON"
AdjustJson || ErrExit "AdjustJson failed."
CheckFile "$JSON"

# Convert AliESDs.root to AO2D.root.
if [ $DOCONVERT -eq 1 ]; then
  CheckFile "$LISTFILES_ALI"
  NFILES=$(wc -l < "$LISTFILES_ALI")
  [ "$NFILES" -eq 0 ] && { ErrExit "No input conversion files!"; }
  NFILESPERJOB_CONVERT=$(python3 -c "n = $NFILESPERJOB_CONVERT; print(n if n > 0 else max(1, round($NFILES * $NCORESPERJOB_ALI / $NCORES)))")
  MsgStep "Converting... ($NFILES files)"
  [ $ISMC -eq 1 ] && MsgWarn "Using MC mode"
  [ $DEBUG -eq 1 ] && echo "Loading AliPhysics..."
  # Run the batch script in the ALI environment.
  [ "$O2_ROOT" ] && { MsgWarn "O2 environment is loaded - expect errors!"; }
  [ "$ALICE_PHYSICS" ] && { MsgWarn "AliPhysics environment is already loaded."; ENVALI=""; }
  $ENVALI bash "$DIR_EXEC/batch_convert.sh" "$LISTFILES_ALI" "$LISTFILES_O2" $ISMC $DEBUG "$NFILESPERJOB_CONVERT" || exit 1
fi

# Run AliPhysics tasks.
if [ $DOALI -eq 1 ]; then
  CheckFile "$LISTFILES_ALI"
  NFILES=$(wc -l < "$LISTFILES_ALI")
  [ "$NFILES" -eq 0 ] && { ErrExit "No input AliPhysics files!"; }
  NFILESPERJOB_ALI=$(python3 -c "n = $NFILESPERJOB_ALI; print(n if n > 0 else max(1, round($NFILES * $NCORESPERJOB_ALI / $NCORES)))")
  MsgStep "Running AliPhysics tasks... ($NFILES files)"
  rm -f "$FILEOUT" "$FILEOUT_ALI" || ErrExit "Failed to rm $FILEOUT $FILEOUT_ALI."
  MakeScriptAli || ErrExit "MakeScriptAli failed."
  CheckFile "$SCRIPT_ALI"
  [ $DEBUG -eq 1 ] && echo "Loading AliPhysics..."
  # Run the batch script in the ALI environment.
  [ "$O2_ROOT" ] && { MsgWarn "O2 environment is loaded - expect errors!"; }
  [ "$ALICE_PHYSICS" ] && { MsgWarn "AliPhysics environment is already loaded."; ENVALI=""; }
  $ENVALI bash "$DIR_EXEC/batch_ali.sh" "$LISTFILES_ALI" "$JSON" "$SCRIPT_ALI" $DEBUG "$NFILESPERJOB_ALI" || exit 1
  mv "$FILEOUT" "$FILEOUT_ALI" || ErrExit "Failed to mv $FILEOUT $FILEOUT_ALI."
fi

# Run O2 tasks.
if [ $DOO2 -eq 1 ]; then
  CheckFile "$LISTFILES_O2"
  NFILES=$(wc -l < "$LISTFILES_O2")
  [ "$NFILES" -eq 0 ] && { ErrExit "No input O2 files!"; }
  NFILESPERJOB_O2=$(python3 -c "n = $NFILESPERJOB_O2; print(n if n > 0 else min(16, max(1, round($NFILES * $NCORESPERJOB_O2 / $NCORES))))") # FIXME: Jobs with more than 16 files per job lose data.
  MsgStep "Running O2 tasks... ($NFILES files)"
  rm -f "$FILEOUT" "$FILEOUT_O2" || ErrExit "Failed to rm $FILEOUT $FILEOUT_O2."
  MakeScriptO2 || ErrExit "MakeScriptO2 failed."
  CheckFile "$SCRIPT_O2"
  [ $SAVETREES -eq 1 ] || FILEOUT_TREES=""
  [ $DEBUG -eq 1 ] && echo "Loading O2Physics..."
  # Run the batch script in the O2 environment.
  [ "$ALICE_PHYSICS" ] && { MsgWarn "AliPhysics environment is loaded - expect errors!"; }
  [ "$O2_ROOT" ] && { MsgWarn "O2 environment is already loaded."; ENVO2=""; }
  $ENVO2 bash "$DIR_EXEC/batch_o2.sh" "$LISTFILES_O2" "$JSON" "$SCRIPT_O2" $DEBUG "$NFILESPERJOB_O2" "$FILEOUT_TREES" "$NJOBSPARALLEL_O2" || exit 1
  mv "$FILEOUT" "$FILEOUT_O2" || ErrExit "Failed to mv $FILEOUT $FILEOUT_O2."
  [[ $SAVETREES -eq 1 && "$FILEOUT_TREES" ]] && { mv "$FILEOUT_TREES" "$FILEOUT_TREES_O2" || ErrExit "Failed to mv $FILEOUT_TREES $FILEOUT_TREES_O2."; }
fi

# Run output postprocessing. (Compare AliPhysics and O2 output.)
if [ $DOPOSTPROCESS -eq 1 ]; then
  LogFile="log_postprocess.log"
  MsgStep "Postprocessing... (logfile: $LogFile)"
  MakeScriptPostprocess || ErrExit "MakeScriptPostprocess failed."
  CheckFile "$SCRIPT_POSTPROCESS"
  [ $DEBUG -eq 1 ] && echo "Loading ROOT..."
  # Run the batch script in the postprocessing environment.
  [ "$ROOTSYS" ] && { MsgWarn "ROOT environment is already loaded."; ENVPOST=""; }
  $ENVPOST bash "$SCRIPT_POSTPROCESS" "$FILEOUT_O2" "$FILEOUT_ALI" > $LogFile 2>&1 || ErrExit "\nCheck $(realpath $LogFile)"
  grep -q -e '^'"W-" -e '^'"Warning" -e "warning" "$LogFile" && MsgWarn "There were warnings!\nCheck $(realpath $LogFile)"
  grep -q -e '^'"E-" -e '^'"Error" "$LogFile" && MsgErr "There were errors!\nCheck $(realpath $LogFile)"
  grep -q -e '^'"F-" -e '^'"Fatal" -e "segmentation" -e "Segmentation" "$LogFile" && ErrExit "There were fatal errors!\nCheck $(realpath $LogFile)"
fi

# Clean after running.
if [ $DOCLEAN -eq 1 ]; then
  MsgStep "Cleaning..."
  Clean 2 || ErrExit "Clean failed."
fi

MsgStep "Done"

exit 0
