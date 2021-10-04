#!/bin/bash
# shellcheck disable=SC1090 # Ignore non-constant source.

export CASEFILE=case.sh
source $CASEFILE
rm "$OUTPUTFOLDER/$CASE.root"
rm "../InputsTheory/$CASE.root"
hadd "$OUTPUTFOLDER/$CASE.root" "$OUTPUTFOLDER"/file_*/"$CASE".root
cp "$OUTPUTFOLDER/$CASE.root" "../InputsTheory/$CASE.root"
