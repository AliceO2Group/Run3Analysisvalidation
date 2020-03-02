#!/bin/bash

reset

INPUTDIR="/data/Run3data/output"
INPUTDIR="/data/Run3data/alice_sim_2018_LHC18a4a2_cent/282099"
LISTNAME="listprodhfrun3.txt"

DOCONVERT=0
DORUN1=1
DORUN3=0
DOCOMPARE=1

DORUN3ONAOD=0

APPLYEVTSELRUN1=1

if [ $DOCONVERT -eq 1 ]; then
    # rm $LISTNAME
    # ls $INPUTDIR/*/AliESDs.root >> $LISTNAME
    #ls /data/Run3data/output/001/AliESDs.root >> $LISTNAME
    echo $LISTNAME
    root -q -l "convertAO2D.C(\"$LISTNAME\")"
    echo "Enter to continue"
    read INP
fi

if [ $DORUN1 -eq 1 ]; then
    fileouttxt="outputlist.txt"
    rm $fileouttxt
    index=0
    echo "Reading file list from $LISTNAME"
    cat $LISTNAME
    while read F ; do
        if [[ $F != *".root" ]]; then
            continue
        fi
        fileout="PidSpectra_$index.root"
        rm "$fileout"
        echo $fileout >> "$fileouttxt"
        echo "Reading input file: $F"
        echo "$fileout"
        root -q -l "ComputePidSpectra.C(\"$F\",\"$fileout\", $APPLYEVTSELRUN1)"
        index=$((index+1))
        echo $index
        # break
    done <"$LISTNAME"
    wait
    rm "PidSpectra.root"
    hadd PidSpectra.root @"$fileouttxt"
    rm PidSpectra_*.root
    
    # root -l PidSpectra.root open.C
    # ./plot.py PidSpectra.root filterEl-task
    
    # echo "Enter to continue"
    # read INP
fi

if [ $DORUN3 -eq 1 ]; then
    rm AnalysisResults*.root
    o2-analysis-spectraTOF --aod-file AO2D.root -b
    # ./plot.py AnalysisResults.root filterEl-task
    # echo "Enter to continue"
    # read INP
fi

if [ $DOCOMPARE -eq 1 ]; then
    ./compare.py AnalysisResults.root PidSpectra.root
fi

if [ $DORUN3ONAOD -eq 1 ]; then
    o2-analysis-spectra --aod-file /data/Run3data/5_20200131-0902/0001/AO2D.root --readers 80 -b
    echo "Enter to continue"
    read INP
fi

rm localhost*_*
