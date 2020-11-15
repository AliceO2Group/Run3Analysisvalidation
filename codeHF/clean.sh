#!/bin/bash

# Load utilities.
source utilities.sh || ErrExit "Failed to load utilities."

MsgStep "Cleaning"

rm -f AnalysisResults_ALI.root AnalysisResults_O2.root AnalysisResults_trees_O2.root && \
rm -f comparison_histos_2prong.pdf comparison_ratios_2prong.pdf comparison_histos_3prong.pdf comparison_ratios_3prong.pdf && \
rm -f *.log && \
rm -rf output_* || ErrExit

exit 0
