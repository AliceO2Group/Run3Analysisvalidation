#!/bin/bash

#INPUTDIR="/data/Run3data/output"
INPUTDIR="/data/Run3data/alice_sim_2018_LHC18a4a2_cent/282099"
LISTNAME="listprodhfrun3.txt"
MASS=1.8
DOCONVERT=1
DORUN1=1
DORUN3=1
DOCOMPARE=1

DORUN3ONAOD=0

APPLYPRIMVERTEXSELRUN1=1
APPLYTRACKCUTRUN1=1
APPLYSECVERTEXSELRUN1=0

rm *.root
rm *.txt
if [ $DOCONVERT -eq 1 ]; then
  rm $LISTNAME
  ls ../inputESD/AliESDs_20200201_v0.root >> $LISTNAME
  #ls $INPUTDIR/00*/AliESDs.root >> $LISTNAME
  echo $LISTNAME
  root -q -l "convertAO2D.C(\"$LISTNAME\")"  
fi

if [ $DORUN1 -eq 1 ]; then
  rm Vertices2prong-ITS1_*.root 
  fileouttxt="outputlist.txt"
  rm $fileouttxt
  
  index=0
  while read F  ; do
    fileout="Vertices2prong-ITS1_$index.root"
    rm "$fileout"
    echo $fileout >> "$fileouttxt"
    echo "$F"
    echo "$fileout" 
    root -q -l "ComputeVerticesRun1.C(\"$F\",\"$fileout\", $APPLYPRIMVERTEXSELRUN1, $APPLYTRACKCUTRUN1, $APPLYSECVERTEXSELRUN1)"
    index=$((index+1))
    echo $index
  done <"$LISTNAME"
  rm "Vertices2prong-ITS1.root" 
  hadd Vertices2prong-ITS1.root @"$fileouttxt"

  
fi

if [ $DORUN3 -eq 1 ]; then
  rm AnalysisResults.root
  o2-analysis-vertexing-hf --aod-file AO2D.root  -b
fi 

if [ $DOCOMPARE -eq 1 ]; then
  root -q -l "Compare.C(\"AnalysisResults.root\",\"Vertices2prong-ITS1.root\", $MASS)"
fi 

if [ $DORUN3ONAOD -eq 1 ]; then
  o2-analysis-vertexing-hf --aod-file /data/Run3data/5_20200131-0902/0001/AO2D.root  --readers 80 -b
fi 

