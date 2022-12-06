#!/bin/bash

# Find tabs and trailing whitespaces in text files

# This directory
DIR_THIS="$(dirname "$(realpath "$0")")"

# Load utilities.
# shellcheck disable=SC1091 # Ignore not following.
source "$DIR_THIS/utilities.sh" || { echo "Error: Failed to load utilities."; exit 1; }

status_tab=0
status_trail=0
# loop over files
while IFS= read -r f; do
  # ignore binary files
  file -bi "$f" | grep -q "charset=binary" && continue
  echo "Scanning file: $f"
  # find tabs in file
  if grep -q -P "\t" "$f"; then
    status_tab=1
    MsgErr "Found some tabs in $f:" > /dev/stderr
    # print out where the tabs are
    #grep -P -n "\t" "$f" > /dev/stderr
    awk 'i=index($0, "\t") {printf "%i (col. %i): %s\n", NR, i, $0}' "$f" > /dev/stderr
  fi
  # find trailing whitespaces in file
  if grep -q " $" "$f"; then
    status_trail=2
    MsgErr "Found some trailing whitespaces in $f:" > /dev/stderr
    # print out where the tabs are
    grep -n " $" "$f" > /dev/stderr
  fi
done < <(find . -type f -not -path "./.git/*")
status=$((status_tab + status_trail))
if [ "$status" -ne 0 ]; then
  MsgWarn "Command tips:
  - Replace each tab with two spaces: sed -i 's/\\\t/  /g' <files>
  - Remove trailing whitespaces: sed -i 's/[[:space:]]*$//' <files>"
fi
exit $status
