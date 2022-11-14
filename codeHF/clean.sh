#!/bin/bash

# Script to delete created files

rm -rf \
AnalysisResults_ALI.root AnalysisResults_O2.root AnalysisResults_trees_O2.root \
comparison_histos_tracks.* comparison_ratios_tracks.* \
comparison_histos_skim.* comparison_ratios_skim.* \
comparison_histos_cand2.* comparison_ratios_cand2.* \
comparison_histos_cand3.* comparison_ratios_cand3.* \
comparison_histos_d0.* comparison_ratios_d0.* \
comparison_histos_d0-mc.* comparison_ratios_d0-mc.* \
comparison_histos_dplus.* comparison_ratios_dplus.* \
comparison_histos_lc.* comparison_ratios_lc.* \
comparison_histos_lc-mc-pt.* comparison_ratios_lc-mc-pt.* \
comparison_histos_lc-mc-eta.* comparison_ratios_lc-mc-eta.* \
comparison_histos_lc-mc-phi.* comparison_ratios_lc-mc-phi.* \
comparison_histos_lc-mc-prompt.* comparison_ratios_lc-mc-prompt.* \
comparison_histos_lc-mc-nonprompt.* comparison_ratios_lc-mc-nonprompt.* \
comparison_histos_jpsi.* comparison_ratios_jpsi.* \
MC_d0_eff.* MC_d0_pT.* \
MC_dplus_eff.* MC_dplus_pT.* \
MC_lc_eff.* MC_lc_pT.* \
MC_xic_eff.* MC_xic_pT.* \
MC_xicc_eff.* MC_xicc_pT.* \
MC_jpsi_eff.* MC_jpsi_pT.* \
MC_lc-tok0sP_eff.* MC_lc-tok0sP_pT.* \
./*.log \
output_* \
|| { echo "Error: Failed to delete files."; exit 1; }

exit 0
