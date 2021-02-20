#!/bin/bash
# shellcheck disable=SC2034 # Ignore unused parameters.

# Input specification for runtest.sh
# (Modifies input parameters.)

INPUT_CASE=4            # Input case

NFILESMAX=1             # Maximum number of processed input files. (Set to -0 to process all; to -N to process all but the last N files.)

# Number of input files per job (Automatic optimisation on if < 1.)
NFILESPERJOB_CONVERT=0  # Conversion
NFILESPERJOB_ALI=0      # AliPhysics
NFILESPERJOB_O2=0       # O2

JSONRUN3="dpl-config_run3.json"  # Run 3 tasks parameters
JSONRUN5="dpl-config_run5.json"  # Run 5 tasks parameters
JSON="$JSONRUN3"

INPUT_FILES="AliESDs.root"  # Input file pattern

case $INPUT_CASE in
  1) # FIXME: no read permission
    INPUT_LABEL="Pb-Pb real LHC15o"
    INPUT_DIR="/mnt/temp/Run3data/data/LHC15o_246751/pass1"
    TRIGGERSTRINGRUN2="CV0L7-B-NOPF-CENT"
    TRIGGERBITRUN3=5;; #FIXME
  2) # FIXME: vertices outside beam pipe
    INPUT_LABEL="Pb-Pb MC LHC15o"
    INPUT_DIR="/data/Run3data/alice_sim_2015_LHC15k1a3_246391/246391"
    ISMC=1;;
  3) # FIXME: nothing selected, missing O2 histograms
    INPUT_LABEL="?"
    INPUT_DIR="/data/Run3data/output"
    ISMC=1;;
  4)
    INPUT_LABEL="p-p MC LHC17p"
    INPUT_DIR="/data/Run3data/alice_sim_2018_LHC18a4a2_cent/282099"
    ISMC=1;;
  5)
    INPUT_LABEL="p-p MC LHC17p"
    INPUT_DIR="/mnt/temp/Run3data_Vit/LHC18a4a2_cent/282341"
    ISMC=1;;
  6)
    INPUT_LABEL="p-p real LHC17p"
    INPUT_DIR="/mnt/temp/Run3data_Vit/LHC17p_pass1_CENT_woSDD/282341";;
  7) # FIXME: hanging
    INPUT_LABEL="Pb-Pb real LHC15o, converted (AliHyperloop LHC15o_test sample)"
    INPUT_DIR="/mnt/temp/Run3data_Vit/LHC15o_converted/alice/data/2015/LHC15o/000244918/pass5_lowIR/PWGZZ/Run3_Conversion/138_20210129-0800_child_1"
    INPUT_FILES="AO2D.root"
    ISINPUTO2=1;;
  8)
    INPUT_LABEL="Run 5, p-p MC 14 TeV CCBAR, Scenario 2"
    INPUT_DIR="/data/Run5data/CCBAR_scenario2_latest"
    INPUT_FILES="AODRun5.*.root"
    JSON="$JSONRUN5"
    ISINPUTO2=1
    ISMC=1;;
  9)
    INPUT_LABEL="Run 5, p-p MC 14 TeV ONIA, Scenario 2"
    INPUT_DIR="/data/Run5data/ONIA_scenario2_latest"
    INPUT_FILES="AODRun5.*.root"
    JSON="$JSONRUN5"
    ISINPUTO2=1
    ISMC=1;;
  10)
    INPUT_LABEL="Run 5, p-p MC 14 TeV MB, Scenario 2"
    INPUT_DIR="/data/Run5data/MB_scenario2_latest"
    INPUT_FILES="AODRun5.*.root"
    JSON="$JSONRUN5"
    ISINPUTO2=1
    ISMC=1;;
esac
