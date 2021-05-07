#!/bin/bash

# Find tabs in text files

status=0
# loop over files
while IFS= read -r f; do
  # ignore binary files
  file -i "$f" | grep -q "charset=binary" && continue
  # find tabs in file
  echo "Scanning file: $f"
  if grep -q -P "\t" "$f"; then
    status=1
    echo "Found some tabs in $f:" > /dev/stderr
    # print out where the tabs are
    grep -P -n -C 1 "\t" "$f" > /dev/stderr
  fi
done < <(find . -type f -not -path "./.git/*")
exit $status
