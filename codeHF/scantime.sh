#!/bin/bash

#INPUTDIR="/data/Run3data/output" #K0* MC injected 

CASE=0
DORUN3=1
DO3PRONG=1
TRIGGERBITRUN3=-1 #FIXME
#AOD3NAME=/home/ginnocen/Run3Analysisvalidation/AO2D_mc_HIJING_PbPb_LHC15k1a3.root #35 
AOD3NAME=/home/ginnocen/Run3Analysisvalidation/AO2D_mc_pp_D2H_LHC18a4a2_cent.root #3141 

if [ $DORUN3 -eq 1 ]; then
  rm AnalysisResults_0.root
  for ptmin in 0.0
  do
  rm test.txt
  echo "----------------"
  echo $ptmin
  time o2-analysis-hftrackindexskimscreator --aod-file $AOD3NAME  -b --triggerindex=$TRIGGERBITRUN3 --ptmintrack=$ptmin --do3prong=$DO3PRONG  --configuration json://$PWD/dpl-config_std.json --keep dangling>> test.txt
  #time o2-analysis-hftrackindexskimscreator --aod-file $AOD3NAME  -b --triggerindex=$TRIGGERBITRUN3 --ptmintrack=$ptmin --do3prong=$DO3PRONG >> test.txt
  grep "real  " test.txt
  grep "totalevents" test.txt  
  du -h AnalysisResults_0.root
  done
fi 
