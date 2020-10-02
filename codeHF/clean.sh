#!/bin/bash

# Message formatting
function MsgStep { echo -e "\n\e[1;32m$@\e[0m"; }
function MsgWarn { echo -e "\e[1;36m$@\e[0m"; }
function MsgErr { echo -e "\e[1;31m$@\e[0m"; }

MsgStep "Cleaning"

rm -f AO2D.root AnalysisResults.root AnalysisResultsQA.root Vertices2prong-ITS1.root
rm -f comparison_histos_2prong.pdf comparison_ratios_2prong.pdf comparison_histos_3prong.pdf comparison_ratios_3prong.pdf
rm -f *.log
rm -rf output_*
