#!/bin/bash

# Script to delete created files

rm -rf \
AnalysisResults_ALI.root AnalysisResults_O2.root AnalysisResults_trees_O2.root \
./*.log \
output_* \
|| { echo "Error: Failed to delete files."; exit 1; }

exit 0
