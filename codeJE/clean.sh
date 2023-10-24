#!/bin/bash

# Script to delete created files

rm -rf \
AnalysisResults_ALI.root AnalysisResults_O2.root \
comparison_histos_jets.* comparison_ratios_jets.* \
./*.log \
output_* \
|| { echo "Error: Failed to delete files."; exit 1; }

exit 0
