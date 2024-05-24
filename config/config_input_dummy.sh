#!/bin/bash
# shellcheck disable=SC2034 # Ignore unused parameters.

# Input specification for runtest.sh
# (Modifies input parameters.)

INPUT_CASE=1            # Input case

NFILESMAX=1             # Maximum number of processed input files. (Set to -0 to process all; to -N to process all but the last N files.)

# Number of input files per job. (Will be automatically optimised if set to 0.)
NFILESPERJOB_CONVERT=0  # Conversion
NFILESPERJOB_ALI=0      # AliPhysics
NFILESPERJOB_O2=1       # O2

# Maximum number of simultaneously running O2 jobs. (Adjust it based on available memory.)
NJOBSPARALLEL_O2=$(python3 -c "print(min(10, round($(nproc) / 2)))")

# Default settings:
# INPUT_LABEL="nothing"           # Input description
# INPUT_DIR="$PWD"                # Input directory
# INPUT_FILES="AliESDs.root"      # Input file pattern
# INPUT_SYS="pp"                  # Collision system ("pp", "PbPb")
# INPUT_RUN=2                     # LHC Run (2, 3, 5)
# INPUT_IS_O2=0                   # Input files are in O2 format.
# INPUT_IS_MC=0                   # Input files are MC data.
# INPUT_PARENT_MASK=""            # Path replacement mask for the input directory of parent files in case of linked derived O2 input. Set to ";" if no replacement needed.
# JSON="dpl-config.json"          # O2 device configuration

INPUT_BASE="/data"

case $INPUT_CASE in
  1)
    INPUT_LABEL="Run 2, p-p 5.02 TeV LHC17p, real"
    INPUT_DIR="$INPUT_BASE/Run2/pp_5.02TeV/real/LHC17p_pass1_CENT_woSDD"
    ;;
  esac
