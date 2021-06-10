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
MC_d0_eff.pdf MC_d0_pT.pdf \
MC_dplus_eff.pdf MC_dplus_pT.pdf \
MC_lc_eff.pdf MC_lc_pT.pdf \
MC_xic_eff.pdf MC_xic_pT.pdf \
MC_jpsi_eff.pdf MC_jpsi_pT.pdf \
MC_lc-tok0sP_eff.pdf MC_lc-tok0sP_pT.pdf \
./*.log \
output_* \
|| { echo "Error: Failed to delete files."; exit 1; }

exit 0
