#!/bin/bash

FILEIN="$1"
ISMC=$2
USEALIEVCUTS=$3
LOGFILE="$4"

# This directory
DIR_THIS="$(dirname "$(realpath "$0")")"

# Run the macro.
root -b -q -l "$DIR_THIS/convertAO2D.C(\"$FILEIN\", $ISMC, $USEALIEVCUTS)" > "$LOGFILE" 2>&1
ExitCode=$?

# Show warnings, errors and fatals in the log file.
grep -e '^'"W-" -e '^'"Warning" -e '^'"E-" -e '^'"Error" -e '^'"F-" -e '^'"Fatal" -e "segmentation" -e "Segmentation" "$LOGFILE" | sort -u

exit $ExitCode
