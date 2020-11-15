#!/bin/bash

# Load message formatting.
source messages.sh || { MsgErr "Error: Failed to load message formatting."; exit 1; }

MsgStep "Cleaning"

rm -f AnalysisResults_ALI.root AnalysisResults_O2.root AnalysisResults_trees_O2.root && \
rm -f comparison_histos_2prong.pdf comparison_ratios_2prong.pdf comparison_histos_3prong.pdf comparison_ratios_3prong.pdf && \
rm -f *.log && \
rm -rf output_* || { MsgErr "Error"; exit 1; }

exit 0
