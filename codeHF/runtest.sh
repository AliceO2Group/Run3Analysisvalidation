#!/bin/bash

bash clean.sh

CASE=4
PARALLELISE=0
RUN5=0      # Use Run 5 input

DOCONVERT=1 # Convert AliESDs.root to AO2D.root.
DOQA=0      # Run the QA task with O2.
DORUN1=1    # Run the heavy-flavour tasks with AliPhysics.
DORUN3=1    # Run the heavy-flavour tasks with O2.
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
  LISTNAME="listprodhfrun3_mc_pp_D2H_LHC18a4a2_cent.txt"
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
  if [ $DOQA -eq 1 ]; then
    echo "Setting MC mode ON."
    ISMC=1
  fi
  echo "Input files taken from: $LISTNAME"
  $ENVALI $CMDROOT "convertAO2D.C(\"$LISTNAME\", $ISMC, $NMAX)" > $LOGFILE 2>&1
  if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
  rm -f $FILEOUTO2
  #mv AO2D.root $AOD3NAME
fi

# Perform simple QA studies with O2.
if [ $DOQA -eq 1 ]; then
  LOGFILE="log_o2_qa.log"
  echo -e "\nRunning the QA task with O2... (logfile: $LOGFILE)"
  rm -f $FILEOUTO2 $FILEOUTQA
#  if [ ! -f "$AOD3NAME" ]; then
#    echo "Error: File $AOD3NAME does not exist."
#    exit 1
#  fi
  O2INPUT="$AOD3NAME"
  if [ $RUN5 -eq 1 ]; then
    O2INPUT="@listrun5.txt"
    echo "Using Run 5 input"
    echo "Input files: $(cat listrun5.txt | wc -l)"
  fi
  O2ARGS="--aod-file $O2INPUT"
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
  $ENVO2 bash $TMPSCRIPT > $LOGFILE 2>&1 # Run the script in the O2 environment.
  if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
  grep WARN $LOGFILE | sort -u
  rm -f $TMPSCRIPT
  mv $FILEOUTO2 $FILEOUTQA
fi

# Run the heavy-flavour tasks with AliPhysics.
if [ $DORUN1 -eq 1 ]; then
  LOGFILE="log_ali_hf.log"
  echo -e "\nRunning the HF tasks with AliPhysics... (logfile: $LOGFILE)"
  rm -f Vertices2prong-ITS1_*.root
  FilesToMerge="outputlist.txt"
  rm -f $FilesToMerge
  rm -f $LOGFILE

  index=0
  while read F  ; do
    FileOutTmp="Vertices2prong-ITS1_$index.root"
    rm -f "$FileOutTmp"
    echo $FileOutTmp >> "$FilesToMerge"
    echo "Index $index"
    echo "Input file: $F"
    echo "Output file: $FileOutTmp"
    $ENVALI $CMDROOT "ComputeVerticesRun1.C(\"$F\",\"$FileOutTmp\",\"$JSON\")" >> $LOGFILE 2>&1
    if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
    index=$((index+1))
  done <"$LISTNAME"
  rm -f $FILEOUTALI
  echo "Merging output files..."
  $ENVALI hadd $FILEOUTALI @"$FilesToMerge" >> $LOGFILE 2>&1
  if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
fi

# Run the heavy-flavour tasks with O2.
if [ $DORUN3 -eq 1 ]; then
  LOGFILE="log_o2_hf.log"
  echo -e "\nRunning the HF tasks with O2... (logfile: $LOGFILE)"
  rm -f $FILEOUTO2
#  if [ ! -f "$AOD3NAME" ]; then
#    echo "Error: File $AOD3NAME does not exist."
#    exit 1
#  fi
  O2JSON="$PWD/dpl-config_std.json"
  if [ $RUN5 -eq 1 ]; then
    O2JSON="$PWD/dpl-config_run5.json"
    echo "Using Run 5 input"
  fi
  # Option --configuration has priority over --aod-file.
#  O2ARGS="--shm-segment-size 16000000000 --configuration json://$PWD/dpl-config_std.json --aod-file $AOD3NAME"
  O2ARGS="--shm-segment-size 16000000000 --configuration json://$O2JSON"
  O2ARGS_SKIM="$O2ARGS"
  O2ARGS_CAND="$O2ARGS"
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
  O2EXEC_TASK="o2-analysis-hf-task-d0 $O2ARGS_TASK"
  O2EXEC="$O2EXEC_SKIM | $O2EXEC_CAND | $O2EXEC_TASK -b"
  TMPSCRIPT="tmpscript.sh"
  cat << EOF > $TMPSCRIPT # Create a temporary script with the full O2 commands.
#!/bin/bash
$O2EXEC
EOF
  $ENVO2 bash $TMPSCRIPT > $LOGFILE 2>&1 # Run the script in the O2 environment.
  if [ ! $? -eq 0 ]; then echo "Error"; exit 1; fi # Exit if error.
  grep WARN $LOGFILE | sort -u
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
