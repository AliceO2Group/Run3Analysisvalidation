#!/bin/bash

# Find tabs in text files

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
    echo "Found some tabs in $f:" > /dev/stderr
    # print out where the tabs are
    grep -P -n -C 1 "\t" "$f" > /dev/stderr
  fi
  # find trailing whitespaces in file
  if grep -q " $" "$f"; then
    status_trail=2
    echo "Found some trailing whitespaces in $f:" > /dev/stderr
    # print out where the tabs are
    grep -n -C 1 " $" "$f" > /dev/stderr
  fi
done < <(find . -type f -not -path "./.git/*")
exit $((status_tab + status_trail))
