#!/bin/bash

# Script to delete created files

rm -rf \
AnalysisResults_ALI.root AnalysisResults_O2.root AnalysisResults_trees_O2.root \
comparison_histos_2prong.pdf comparison_ratios_2prong.pdf comparison_histos_3prong.pdf comparison_ratios_3prong.pdf \
D0_MC_eff.pdf  D0_MC_pT.pdf \
./*.log \
output_* \
|| { echo "Error: Failed to delete files."; exit 1; }

exit 0
