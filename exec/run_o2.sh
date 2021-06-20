#!/bin/bash

SCRIPT="$1"
FILEIN="$2"
JSON="$3"
LOGFILE="$4"

# Run the script.
bash "$SCRIPT" "$FILEIN" "$JSON" > "$LOGFILE" 2>&1
ExitCode=$?

# Show warnings and errors in the log file.
grep -e "\\[WARN\\]" -e "\\[ERROR\\]" -e "\\[FATAL\\]" -e "segmentation" -e "command not found" "$LOGFILE" | sort -u

exit $ExitCode
