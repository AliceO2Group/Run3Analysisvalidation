#!/bin/bash

echo -e "\nCleaning"

rm -f AO2D.root AnalysisResults.root AnalysisResultsQA.root Vertices2prong-ITS1.root
rm -f comparison_histos_2prong.pdf comparison_ratios_2prong.pdf comparison_histos_3prong.pdf comparison_ratios_3prong.pdf
rm -f *.log
rm -rf output_*
