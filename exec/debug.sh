#!/bin/bash

# Debug bad jobs.

# Show which jobs did not finish successfully or normally, show their input files.
# Show warnings and errors in the logs of individual jobs.

TYPE="o2" # conversion, ali, o2
BAD=0
UNFINISHED=0
FILES=0
WARNINGS=0
ERRORS=0

# Identify a bad job (without output file or successful end).
IsBadJob() {
  STRING="Dumping"
  [ "$TYPE" == "ali" ] && STRING="long) 0"
  [ -f "$DIRJOB/AnalysisResults.root" ] || return 0
  tail -n 1 "$LOG" | grep -q "$STRING" && return 1 || return 0
}

# Identify an unfinished job (running, hanging, aborted).
IsUnfinishedJob() {
  STRING="Dumping"
  [ "$TYPE" == "ali" ] && STRING="long) 0"
  tail -n 2 "$LOG" | grep -q "$STRING" && return 1 || return 0
}

# Print warnings in the log.
PrintWarnings() {
  if [ "$TYPE" == "o2" ]; then
    grep -q "\\[WARN\\]" "$LOG" && {
      [ "$PRINTDIR" -eq 1 ] || { echo -e "\n$DIRJOB"; PRINTDIR=1; }
      grep "\\[WARN\\]" "$LOG" | sort -u
    }
  elif [ "$TYPE" == "ali" ]; then
    grep -q -e '^'"W-" -e '^'"Warning" "$LOG" && {
      [ "$PRINTDIR" -eq 1 ] || { echo -e "\n$DIRJOB"; PRINTDIR=1; }
      grep -e '^'"W-" -e '^'"Warning" "$LOG" | sort -u
    }
  fi
}

# Print errors in the log.
PrintErrors() {
  if [ "$TYPE" == "o2" ]; then
    grep -q -e "\\[ERROR\\]" -e "segmentation" "$LOG" && {
      [ "$PRINTDIR" -eq 1 ] || { echo -e "\n$DIRJOB"; PRINTDIR=1; }
      grep -e "\\[ERROR\\]" -e "segmentation" "$LOG" | sort -u
    }
  elif [ "$TYPE" == "ali" ]; then
    grep -q -e '^'"E-" -e '^'"Error" -e '^'"F-" -e '^'"Fatal" -e "segmentation" "$LOG" && {
      [ "$PRINTDIR" -eq 1 ] || { echo -e "\n$DIRJOB"; PRINTDIR=1; }
      grep -e '^'"E-" -e '^'"Error" -e '^'"F-" -e '^'"Fatal" -e "segmentation" "$LOG" | sort -u
    }
  fi
}

# Print out syntax.
Help() {
  echo "Usage: bash [<path>/]$(basename "$0") [-h] [-t TYPE] [-b [-u]] [-f] [-w] [-e]"
  echo "TYPE: conversion, ali, o2 (o2 by default)"
  echo "-b  Show bad jobs (without output file or successful end)."
  echo "-u  Mark unfinished jobs (running, hanging, aborted). (Requires -b.)"
  echo "-f  Show input files of bad jobs."
  echo "-w  Show warnings (for all jobs)."
  echo "-e  Show errors (for all jobs)."
}

####################################################################################################

# Parse command line options.
while getopts ":ht:bufwe" opt; do
  case ${opt} in
    h)
      Help; exit 0;;
    t)
      TYPE="$OPTARG";;
    b)
      BAD=1;;
    u)
      UNFINISHED=1;;
    f)
      FILES=1;;
    w)
      WARNINGS=1;;
    e)
      ERRORS=1;;
    \?)
      echo "Error: Invalid option: $OPTARG" 1>&2; Help; exit 1;;
    :)
      echo "Error: Invalid option: $OPTARG requires an argument." 1>&2; Help; exit 1;;
  esac
done

[[ "$TYPE" != "o2" && "$TYPE" != "ali" && "$TYPE" != "conversion" ]] && { echo "Error: Unsupported type: $TYPE" 1>&2; exit 1; }

if ! [[ $BAD -eq 1 || $FILES -eq 1 || $WARNINGS -eq 1 || $ERRORS -eq 1 ]]; then
  echo "Nothing to do"; Help; exit 0
fi

DIR="output_$TYPE"
[ -d "$DIR" ] || { echo "Error: Directory $DIR does not exist." 1>&2; exit 1; }

[ "$TYPE" == "conversion" ] && TYPE="ali"

# Loop over log files
for LOG in "$DIR"/*/log_*.log; do
  DIRJOB="$(dirname "$LOG")"
  PRINTDIR=0
  IsBadJob && {
    [ $BAD -eq 1 ] && {
      TEXT="$DIRJOB"
      [ $UNFINISHED -eq 1 ] && { IsUnfinishedJob && TEXT+=" unfinished"; }
      if ! [[ $FILES -eq 1 || $WARNINGS -eq 1 || $ERRORS -eq 1 ]]; then
        echo "$TEXT"
      else
        echo -e "\n$TEXT"
      fi
      PRINTDIR=1
    }
    [ $FILES -eq 1 ] && cat "$DIRJOB"/list_*.txt
  }
  [ $WARNINGS -eq 1 ] && PrintWarnings
  [ $ERRORS -eq 1 ] && PrintErrors
done

exit 0
