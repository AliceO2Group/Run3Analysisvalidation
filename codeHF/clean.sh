#!/bin/bash

# Script to delete created files

# This directory
DIR_THIS="$(dirname $(realpath $0))"

# Load utilities.
source "$DIR_THIS/utilities.sh" || { echo "Error: Failed to load utilities."; exit 1; }

MsgStep "Cleaning"

rm -f AnalysisResults_ALI.root AnalysisResults_O2.root AnalysisResults_trees_O2.root && \
rm -f comparison_histos_2prong.pdf comparison_ratios_2prong.pdf comparison_histos_3prong.pdf comparison_ratios_3prong.pdf && \
rm -f *.log && \
rm -rf output_* || ErrExit "Failed to delete files."

exit 0
