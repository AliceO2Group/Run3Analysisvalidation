#!/bin/bash

bash clean.sh

CASE=4

DOCONVERT=1 # Convert AliESDs.root to AO2D.root.
DOQA=0 # Run the QA task
DORUN1=1 # Run the tasks with AliPhysics.
DORUN3=1 # Run the tasks with O2.
DOCOMPARE=1 # Compare AliPhysics and O2 output.

if [ $CASE -eq 0 ]; then
  INPUTDIR="../twikiinput"
  ISMC=0
  LISTNAME="listprodhfrun3_ppK0starToyMC.txt"
  AOD3NAME=AO2D.root
  MASS=1.0
  STRING="AliESDs_ppK0starToyMC.root"
  TRIGGERSTRINGRUN2=""
  TRIGGERBITRUN3=-1
  NMAX=-1
  JSON=dpl-config_std.json
fi

if [ $CASE -eq 1 ]; then
  INPUTDIR="/mnt/temp/Run3data/data/LHC15o_246751/pass1"
  ISMC=0
  LISTNAME="listprodhfrun3_PbPb_data_LHC15o_246751.txt"
  AOD3NAME=AO2D.root
  MASS=1.8
  STRING="15000246751019.110/AliESDs.root"
  TRIGGERSTRINGRUN2="CV0L7-B-NOPF-CENT"
  TRIGGERBITRUN3=5 #FIXME
  NMAX=5
  JSON=dpl-config_std.json
fi

if [ $CASE -eq 2 ]; then
  INPUTDIR="/data/Run3data/alice_sim_2015_LHC15k1a3_246391/246391"
  ISMC=1
  LISTNAME="listprodhfrun3_mc_HIJING_PbPb_LHC15k1a3.txt"
  AOD3NAME=AO2D.root
  MASS=1.8
  STRING="00*/AliESDs.root"
  TRIGGERSTRINGRUN2=""
  TRIGGERBITRUN3=-1
  NMAX=1
  JSON=dpl-config_std.json
fi

if [ $CASE -eq 3 ]; then
  INPUTDIR="/data/Run3data/output"
  ISMC=0
  LISTNAME="listprodhfrun3_K0sMC.txt"
  AOD3NAME=AO2D.root
  MASS=1.0
  STRING="00*/AliESDs.root"
  TRIGGERSTRINGRUN2=""
  TRIGGERBITRUN3=-1
  NMAX=-1
  JSON=dpl-config_std.json
fi

if [ $CASE -eq 4 ]; then
  INPUTDIR="/data/Run3data/alice_sim_2018_LHC18a4a2_cent/282099"
  ISMC=0
  LISTNAME="listprodhfrun3_mc_PbPb_D2H_LHC18a4a2_cent.txt"
  AOD3NAME=AO2D.root
  MASS=1.8
  STRING="001/AliESDs.root"
  TRIGGERSTRINGRUN2=""
  TRIGGERBITRUN3=-1
  JSON=dpl-config_std.json
  NMAX=-1
fi

#INPUTDIR="/data/Run3data/output" #K0* MC injected
#INPUTDIR="/data/Run3data/alice_sim_2018_LHC18a4a2_cent/282099" #D2H MC sample
#INPUTDIR="/data/Run3data/alice_sim_2015_LHC15k1a3_246391/246391" #HIJING MC PbPb

# List of input files
ls $INPUTDIR/$STRING > $LISTNAME

# Output files
FILEOUTALI="Vertices2prong-ITS1.root"
FILEOUTO2="AnalysisResults.root"
FILEOUTQA="AnalysisResultsQA.root"

# Steering commands
ENVALI="alienv setenv AliPhysics/latest -c"
ENVO2="alienv setenv O2/latest -c"
CMDROOT="root -b -q -l"

# Convert AliESDs.root to AO2D.root.
if [ $DOCONVERT -eq 1 ]; then
  LOGFILE="log_convert.log"
  echo -e "\nConverting... (logfile: $LOGFILE)"
  echo "Input files taken from: $LISTNAME"
  $ENVALI $CMDROOT "convertAO2D.C(\"$LISTNAME\", $ISMC, $NMAX)" > $LOGFILE 2>&1
  if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
  rm -f $FILEOUTO2
  #mv AO2D.root $AOD3NAME
fi

# Perform simple QA studies
if [ $DOQA -eq 1 ]; then
  LOGFILE="log_QA.log"
  rm -f $FILEOUTOMC
  if [ ! -f "$AOD3NAME" ]; then
    echo "Error: File $AOD3NAME does not exist."
    exit 1
  fi
  O2EXEC="o2-analysis-qatask --aod-file $AOD3NAME -b"
  TMPSCRIPT="tmpscript.sh"
  cat << EOF > $TMPSCRIPT # Create a temporary script with the full O2 commands.
#!/bin/bash
$O2EXEC
EOF
  $ENVO2 bash $TMPSCRIPT > $LOGFILE 2>&1 # Run the script in the O2 environment.
  if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
  rm -f $TMPSCRIPT
fi
mv AnalysisResults.root $FILEOUTQA

# Run the tasks with AliPhysics.
if [ $DORUN1 -eq 1 ]; then
  LOGFILE="log_ali.log"
  echo -e "\nRunning the tasks with AliPhysics... (logfile: $LOGFILE)"
  rm -f Vertices2prong-ITS1_*.root
  fileouttxt="outputlist.txt"
  rm -f $fileouttxt
  rm -f $LOGFILE

  index=0
  while read F  ; do
    fileout="Vertices2prong-ITS1_$index.root"
    rm -f "$fileout"
    echo $fileout >> "$fileouttxt"
    echo "Index $index"
    echo "Input file: $F"
    echo "Output file: $fileout"
    $ENVALI $CMDROOT "ComputeVerticesRun1.C(\"$F\",\"$fileout\",\"$JSON\")" >> $LOGFILE 2>&1
    if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
    index=$((index+1))
  done <"$LISTNAME"
  rm -f $FILEOUTALI
  echo "Merging output files..."
  $ENVALI hadd $FILEOUTALI @"$fileouttxt" >> $LOGFILE 2>&1
  if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
fi

# Run the tasks with O2.
if [ $DORUN3 -eq 1 ]; then
  LOGFILE="log_o2.log"
  echo -e "\nRunning the tasks with O2... (logfile: $LOGFILE)"
  rm -f $FILEOUTO2
  if [ ! -f "$AOD3NAME" ]; then
    echo "Error: File $AOD3NAME does not exist."
    exit 1
  fi
  O2ARGS="--shm-segment-size 16000000000 --configuration json://$PWD/dpl-config_std.json --aod-file $AOD3NAME"
  O2EXEC="o2-analysis-hftrackindexskimscreator $O2ARGS | o2-analysis-hfcandidatecreator2prong $O2ARGS | o2-analysis-taskdzero $O2ARGS -b"
  TMPSCRIPT="tmpscript.sh"
  cat << EOF > $TMPSCRIPT # Create a temporary script with the full O2 commands.
#!/bin/bash
$O2EXEC
EOF
  $ENVO2 bash $TMPSCRIPT > $LOGFILE 2>&1 # Run the script in the O2 environment.
  if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
  rm -f $TMPSCRIPT
fi

# Compare AliPhysics and O2 output.
if [ $DOCOMPARE -eq 1 ]; then
  LOGFILE="log_compare.log"
  echo -e "\nComparing... (logfile: $LOGFILE)"
  ok=1
  for file in "$FILEOUTALI" "$FILEOUTO2"; do
    if [ ! -f "$file" ]; then
      echo "Error: File $file does not exist."
      ok=0
    fi
  done
  if [ ! $ok -eq 1 ]; then exit 1; fi
  $ENVALI $CMDROOT "Compare.C(\"$FILEOUTO2\",\"$FILEOUTALI\", $MASS)" > $LOGFILE 2>&1
  if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
fi

echo -e "\nDone"
exit 0

