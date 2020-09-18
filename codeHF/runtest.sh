#!/bin/bash

bash clean.sh

CASE=4

DOCONVERT=1   # Convert AliESDs.root to AO2D.root.
DOQA=0        # Run the QA task with O2.
DORUN1=1      # Run the heavy-flavour tasks with AliPhysics.
DORUN3=1      # Run the heavy-flavour tasks with O2.
DOCOMPARE=1   # Compare AliPhysics and O2 output.

RUN5=0        # Use Run 5 input.
CONVSEP=1     # Convert ESD files separately.
PARALLELISE=0 # Parallelise O2 tasks.
TWOPRONGSEL=0 # Apply D0 selection cuts.

# Default settings
JSON="$PWD/dpl-config_std.json"
ISMC=0
MASS=1.8
TRIGGERSTRINGRUN2=""
TRIGGERBITRUN3=-1
NMAX=-1

if [ $TWOPRONGSEL -eq 1 ]; then
  JSON="$PWD/dpl-config_selection.json"
fi

if [ $CASE -eq 0 ]; then
  INPUTDIR="../twikiinput"
  MASS=1.0
  STRING="AliESDs_ppK0starToyMC.root"
fi

if [ $CASE -eq 1 ]; then
  INPUTDIR="/mnt/temp/Run3data/data/LHC15o_246751/pass1"
  STRING="15000246751019.110/AliESDs.root"
  TRIGGERSTRINGRUN2="CV0L7-B-NOPF-CENT"
  TRIGGERBITRUN3=5 #FIXME
  NMAX=5
fi

if [ $CASE -eq 2 ]; then
  INPUTDIR="/data/Run3data/alice_sim_2015_LHC15k1a3_246391/246391"
  ISMC=1
  STRING="00*/AliESDs.root"
  NMAX=1
fi

if [ $CASE -eq 3 ]; then
  INPUTDIR="/data/Run3data/output"
  MASS=1.0
  STRING="00*/AliESDs.root"
fi

if [ $CASE -eq 4 ]; then
  INPUTDIR="/data/Run3data/alice_sim_2018_LHC18a4a2_cent/282099"
  STRING="001/AliESDs.root"
fi

#INPUTDIR="/data/Run3data/output" #K0* MC injected
#INPUTDIR="/data/Run3data/alice_sim_2018_LHC18a4a2_cent/282099" #D2H MC sample
#INPUTDIR="/data/Run3data/alice_sim_2015_LHC15k1a3_246391/246391" #HIJING MC PbPb

# List of input files
ls $INPUTDIR/$STRING > $LISTNAME
LISTFILESO2="listrun3.txt"
LISTFILESO2RUN5="listrun5.txt"

# Output files
FILEOUTALI="Vertices2prong-ITS1.root"
FILEOUTO2="AnalysisResults.root"
FILEOUTQA="AnalysisResultsQA.root"

# Steering commands
ENVALI="alienv setenv AliPhysics/latest -c"
ENVO2="alienv setenv O2/latest -c"
ENVALIO2="alienv setenv AliPhysics/latest,O2/latest -c"
CMDROOT="root -b -q -l"

# Convert AliESDs.root to AO2D.root.
if [ $DOCONVERT -eq 1 ]; then
  echo -e "\nConverting..."
  if [ $DOQA -eq 1 ]; then
    echo "Setting MC mode ON."
    ISMC=1
  fi
  echo "Input files taken from: $LISTNAME ($(cat $LISTNAME | wc -l))"
  if [ $CONVSEP -eq 1 ]; then
    echo "Converting files separately"
    $ENVALI bash convert_batch.sh $LISTNAME $LISTFILESO2 $ISMC # Run the batch script in the ALI environment.
    if [ $? -ne 0 ]; then exit 1; fi # Exit if error.
  else
    LOGFILE="log_convert.log"
    rm -f $LOGFILE
    echo "logfile: $LOGFILE"
    $ENVALI $CMDROOT "convertAO2D.C(\"$LISTNAME\", $ISMC, $NMAX)" > $LOGFILE 2>&1
    if [ $? -ne 0 ]; then echo "Error"; exit 1; fi # Exit if error.
    echo "$PWD/AO2D.root" > $LISTFILESO2
    rm -f $FILEOUTO2
  fi
fi

# Perform simple QA studies with O2.
if [ $DOQA -eq 1 ]; then
  #LOGFILE="log_o2_qa.log"
  echo -e "\nRunning the QA task with O2..."
  rm -f $FILEOUTO2 $FILEOUTQA
  O2INPUT=$LISTFILESO2
  O2JSON="$PWD/dpl-config_std.json"
  if [ $RUN5 -eq 1 ]; then
    O2INPUT=$LISTFILESO2RUN5
    O2JSON="$PWD/dpl-config_run5.json"
    echo "Using Run 5 input"
  fi
  echo "Input files: $(cat $O2INPUT | wc -l)"
  O2ARGS="--shm-segment-size 16000000000 --configuration json://$O2JSON"
  if [ $PARALLELISE -eq 1 ]; then
    NPROC=3
    echo "Parallelisation ON ($NPROC)"
    O2ARGS="$O2ARGS --pipeline qa-tracking-kine:$NPROC,qa-tracking-resolution:$NPROC"
  fi
  O2EXEC="o2-analysis-qatask $O2ARGS -b"
  TMPSCRIPT="tmpscript.sh"
  cat << EOF > $TMPSCRIPT # Create a temporary script with the full O2 commands.
#!/bin/bash
$O2EXEC
EOF
  #$ENVO2 bash $TMPSCRIPT > $LOGFILE 2>&1 # Run the script in the O2 environment.
  #if [ $? -ne 0 ]; then echo "Error"; exit 1; fi # Exit if error.
  #grep WARN $LOGFILE | sort -u
  $ENVO2 bash o2_batch.sh $O2INPUT $O2JSON $TMPSCRIPT # Run the batch script in the O2 environment.
  if [ $? -ne 0 ]; then exit 1; fi # Exit if error.
  rm -f $TMPSCRIPT
  mv $FILEOUTO2 $FILEOUTQA
  mv output_o2 output_o2_qa
  mv log_o2.log log_o2_qa.log
fi

# Run the heavy-flavour tasks with AliPhysics.
if [ $DORUN1 -eq 1 ]; then
  echo -e "\nRunning the HF tasks with AliPhysics..."
  #$ENVALI bash ali_batch.sh $LISTNAME $JSON $FILEOUTALI $TWOPRONGSEL # Run the batch script in the ALI environment.
  $ENVALIO2 bash ali_batch.sh $LISTNAME $JSON $FILEOUTALI $TWOPRONGSEL # Run the batch script in the ALI+O2 environment.
  if [ $? -ne 0 ]; then exit 1; fi # Exit if error.
fi

# Run the heavy-flavour tasks with O2.
if [ $DORUN3 -eq 1 ]; then
  #LOGFILE="log_o2_hf.log"
  echo -e "\nRunning the HF tasks with O2..."
  rm -f $FILEOUTO2
  O2INPUT=$LISTFILESO2
  O2JSON="$PWD/dpl-config_std.json"
  if [ $TWOPRONGSEL -eq 1 ]; then
    O2JSON="$PWD/dpl-config_selection.json"
  fi
  if [ $RUN5 -eq 1 ]; then
    O2INPUT=$LISTFILESO2RUN5
    O2JSON="$PWD/dpl-config_run5.json"
    echo "Using Run 5 input"
  fi
  # Option --configuration has priority over --aod-file.
#  O2ARGS="--shm-segment-size 16000000000 --configuration json://$PWD/dpl-config_std.json --aod-file $AOD3NAME"
  O2ARGS="--shm-segment-size 16000000000 --configuration json://$O2JSON"
  O2ARGS_SKIM="$O2ARGS"
  O2ARGS_CAND="$O2ARGS"
  O2ARGS_PIDTPC="$O2ARGS"
  O2ARGS_PIDTOF="$O2ARGS"
  O2ARGS_SEL="$O2ARGS"
  O2ARGS_TASK="$O2ARGS"
  if [ $PARALLELISE -eq 1 ]; then
    NPROC=3
    echo "Parallelisation ON ($NPROC)"
    O2ARGS_SKIM="$O2ARGS_SKIM --pipeline hf-produce-sel-track:$NPROC,hf-track-index-skims-creator:$NPROC"
    O2ARGS_CAND="$O2ARGS_CAND --pipeline hf-cand-creator-2prong:$NPROC,hf-cand-creator-2prong-expressions:$NPROC"
    O2ARGS_TASK="$O2ARGS_TASK --pipeline hf-task-d0:$NPROC"
  fi
  O2EXEC_SKIM="o2-analysis-hf-track-index-skims-creator $O2ARGS_SKIM"
  O2EXEC_CAND="o2-analysis-hf-candidate-creator-2prong $O2ARGS_CAND"
  O2EXEC_PIDTPC="o2-analysis-pid-tpc $O2ARGS_PIDTPC"
  O2EXEC_PIDTOF="o2-analysis-pid-tof $O2ARGS_PIDTOF"
  O2EXEC_SEL="o2-analysis-hf-d0-candidate-selector $O2ARGS_SEL"
  O2EXEC_TASK="o2-analysis-hf-task-d0 $O2ARGS_TASK"
  O2EXEC="$O2EXEC_SKIM | $O2EXEC_PIDTPC | $O2EXEC_PIDTOF | $O2EXEC_CAND | $O2EXEC_SEL | $O2EXEC_TASK -b"
  TMPSCRIPT="tmpscript.sh"
  cat << EOF > $TMPSCRIPT # Create a temporary script with the full O2 commands.
#!/bin/bash
$O2EXEC
EOF
  #$ENVO2 bash $TMPSCRIPT > $LOGFILE 2>&1 # Run the script in the O2 environment.
  #if [ $? -ne 0 ]; then echo "Error"; exit 1; fi # Exit if error.
  #grep WARN $LOGFILE | sort -u
  $ENVO2 bash o2_batch.sh $O2INPUT $O2JSON $TMPSCRIPT # Run the batch script in the O2 environment.
  if [ $? -ne 0 ]; then exit 1; fi # Exit if error.
  rm -f $TMPSCRIPT
  mv output_o2 output_o2_hf
  mv log_o2.log log_o2_hf.log
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
  if [ $ok -ne 1 ]; then exit 1; fi
  $ENVALI $CMDROOT "Compare.C(\"$FILEOUTO2\",\"$FILEOUTALI\", $MASS)" > $LOGFILE 2>&1
  if [ $? -ne 0 ]; then echo "Error"; exit 1; fi # Exit if error.
fi

echo -e "\nDone"
exit 0
