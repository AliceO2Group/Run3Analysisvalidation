#!/bin/bash

INPUTDIR="/home/ginnocen/outputProdHFRun3"
LISTNAME="listprodhfrun3.txt"

DOCONVERT=0
DORUN1=0
DORUN3=0
DOCOMPARE=0

DORUN3ONAOD=1

if [ $DOCONVERT -eq 1 ]; then
  rm $LISTNAME
  #ls $INPUTDIR/*/AliESDs.root >> $LISTNAME
  ls ../inputESD/AliESDs_20200201_v0.root >> $LISTNAME
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
    root -q -l "ComputeVerticesRun1.C(\"$F\",\"$fileout\")"
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
  root -q -l "Compare.C(\"AnalysisResults.root\",\"Vertices2prong-ITS1.root\")"
fi 

if [ $DORUN3ONAOD -eq 1 ]; then
  o2-analysis-vertexing-hf --aod-file /data/Run3data/5_20200131-0902/0001/AO2D.root  --readers 80 -b
fi 

