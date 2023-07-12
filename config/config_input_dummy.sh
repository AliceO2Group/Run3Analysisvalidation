#!/bin/bash
# shellcheck disable=SC2034 # Ignore unused parameters.

# Input specification for runtest.sh
# (Modifies input parameters.)

INPUT_CASE=1            # Input case

NFILESMAX=1             # Maximum number of processed input files. (Set to -0 to process all; to -N to process all but the last N files.)

# Number of input files per job (Automatic optimisation on if < 1.)
NFILESPERJOB_CONVERT=0  # Conversion
NFILESPERJOB_ALI=0      # AliPhysics
NFILESPERJOB_O2=1       # O2

# Maximum number of simultaneously running O2 jobs
NJOBSPARALLEL_O2=$(python3 -c "print(min(10, round($(nproc) / 2)))")

# Default settings:
# INPUT_FILES="AliESDs.root"
# INPUT_SYS="pp"
# INPUT_RUN=2
# INPUT_IS_O2=0
# INPUT_IS_MC=0
# JSON="dpl-config.json"

INPUT_BASE="/data"

case $INPUT_CASE in
  1)
    INPUT_LABEL="Run 2, p-p 5.02 TeV LHC17p, real"
    INPUT_DIR="$INPUT_BASE/Run2/pp_5.02TeV/real/LHC17p_pass1_CENT_woSDD"
    ;;
  esac
