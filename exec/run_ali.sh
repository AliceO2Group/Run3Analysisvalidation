#!/bin/bash

SCRIPT="$1"
FILEIN="$2"
JSON="$3"
LOGFILE="$4"

# Run the script.
bash "$SCRIPT" "$FILEIN" "$JSON" > "$LOGFILE" 2>&1
ExitCode=$?

# Show warnings, errors and fatals in the log file.
grep -e '^'"W-" -e '^'"Warning" -e '^'"E-" -e '^'"Error" -e '^'"F-" -e '^'"Fatal" -e "segmentation" "$LOGFILE" | sort -u

exit $ExitCode
