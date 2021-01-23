#!/bin/bash

# Script to delete created files

rm -rf \
AnalysisResults_ALI.root AnalysisResults_O2.root AnalysisResults_trees_O2.root \
comparison_histos_tracks.pdf comparison_ratios_tracks.pdf \
comparison_histos_skim.pdf comparison_ratios_skim.pdf \
comparison_histos_cand2.pdf comparison_ratios_cand2.pdf \
comparison_histos_cand3.pdf comparison_ratios_cand3.pdf \
comparison_histos_d0.pdf comparison_ratios_d0.pdf \
comparison_histos_dplus.pdf comparison_ratios_dplus.pdf \
comparison_histos_lc.pdf comparison_ratios_lc.pdf \
comparison_histos_jpsi.pdf comparison_ratios_jpsi.pdf \
D0_MC_eff.pdf  D0_MC_pT.pdf \
./*.log \
output_* \
|| { echo "Error: Failed to delete files."; exit 1; }

exit 0
