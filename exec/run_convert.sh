#!/bin/bash

FILEIN="$1"
ISMC=$2
LOGFILE="$3"

# This directory
DIR_THIS="$(dirname $(realpath $0))"

# Run the macro.
root -b -q -l "$DIR_THIS/convertAO2D.C(\"$FILEIN\", $ISMC)" > "$LOGFILE" 2>&1
ExitCode=$?

exit $ExitCode
