#!/bin/bash

SCRIPT="$1"
FILEIN="$2"
JSON="$3"
LOGFILE="$4"

# Run the script.
bash "$SCRIPT" "$FILEIN" "$JSON" > "$LOGFILE" 2>&1
ExitCode=$?

# Show warnings and errors in the log file.
grep -e "\\[WARN\\]" -e "\\[ERROR\\]" "$LOGFILE" | sort -u

# Clean.
pid=$(tail -n 2 "$LOGFILE" | grep "is exiting" | cut -d " " -f 3) # Get the process ID from the O2 log.
find /tmp -maxdepth 1 -type s -group "$USER" -name "localhost${pid}_*" -delete 2> /dev/null # Delete the process sockets.

exit $ExitCode
