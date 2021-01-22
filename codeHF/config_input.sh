#!/bin/bash

# Input specification for runtest.sh
# (Modifies input parameters.)

INPUT_CASE=10  # Input case
NFILESMAX=-0   # Maximum number of processed input files. (Set to -0 to process all; to -N to process all but the last N files.)

JSONRUN3="dpl-config_run3.json"  # Run 3 tasks parameters
JSONRUN5="dpl-config_run5.json"  # Run 5 tasks parameters
JSON="$JSONRUN3"

INPUT_FILES="*/AliESDs.root"  # Input file pattern

case $INPUT_CASE in
  0) # FIXME: missing input files
    INPUT_LABEL="ppK0starToyMC"
    INPUT_DIR="../twikiinput"
    INPUT_FILES="AliESDs_ppK0starToyMC.root";;
  1) # FIXME: no read permission
    INPUT_LABEL="Pb-Pb real LHC15o"
    INPUT_DIR="/mnt/temp/Run3data/data/LHC15o_246751/pass1"
    TRIGGERSTRINGRUN2="CV0L7-B-NOPF-CENT"
    TRIGGERBITRUN3=5;; #FIXME
  2) # FIXME: conversion fails
    INPUT_LABEL="Pb-Pb MC LHC15o"
    INPUT_DIR="/data/Run3data/alice_sim_2015_LHC15k1a3_246391/246391"
    ISMC=1;;
  3) # FIXME: conversion fails
    INPUT_LABEL="?"
    INPUT_DIR="/data/Run3data/output";;
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
  7)
    INPUT_LABEL="Pb-Pb real LHC15o, converted (AliHyperloop LHC15o_test sample)"
    INPUT_DIR="/mnt/temp/Run3data_Vit/LHC15o_converted/alice/data/2015/LHC15o/000244918/pass5_lowIR/PWGZZ/Run3_Conversion/96_20201013-1346_child_1"
    INPUT_FILES="*/AO2D.root"
    ISINPUTO2=1;;
  8)
    INPUT_LABEL="Run 5, p-p MC 14 TeV CCBAR, Scenario 3"
    INPUT_DIR="/data/Run5data/CCbar_5Mev_scenario3_18012021"
    INPUT_FILES="AODRun5.*.root"
    JSON="$JSONRUN5"
    ISINPUTO2=1
    ISMC=1;;

esac
