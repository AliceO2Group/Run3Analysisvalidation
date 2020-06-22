#!/bin/bash

#INPUTDIR="/data/Run3data/output" #K0* MC injected 

DOCONVERT=0
DORUN3=1
INPUTDIR="/data/Run3data/alice_sim_2015_LHC15k1a3_246391/246391"
ISMC=0
LISTNAME="listprodhfrun3_mc_HIJING_PbPb_LHC15k1a3.txt"
AOD3NAME=test_AO2D_mc_HIJING_PbPb_LHC15k1a3.root
STRING="00*/AliESDs.root"
NMAX=5

if [ $DOCONVERT -eq 1 ]; then
  rm *.root
  rm *.txt
  rm $LISTNAME
  ls $INPUTDIR/$STRING >> $LISTNAME
  echo $LISTNAME
  root -q -l "convertAO2D.C(\"$LISTNAME\", $ISMC, $NMAX)"  
  mv AO2D.root $AOD3NAME
fi

if [ $DORUN3 -eq 1 ]; then
  rm AnalysisResults_0.root
  for ptmin in 10.0
  do
  rm test.txt
  echo "----------------"
  #o2-analysis-hftrackindexskimscreator --shm-segment-size 16000000000 --configuration json://$PWD/dpl-config_std.json -b
  time o2-analysis-hftrackindexskimscreator --shm-segment-size 16000000000 --configuration json://$PWD/dpl-config_std.json -b >> test.txt
  #o2-analysis-hftrackindexskimscreator --shm-segment-size 16000000000 --configuration json://$PWD/dpl-config_std.json -b
  grep "real  " test.txt
  grep "totalevents" test.txt  
  du -h AnalysisResults_0.root
  done
fi 
