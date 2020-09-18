#!/bin/bash

echo -e "\nCleaning"

rm -f AO2D*.root AnalysisResults.root AnalysisResultsQA.root EventStat_temp.root Vertices2prong-ITS1*.root event_stat.root
rm -f comparison_histos.pdf comparison_ratios.pdf
rm -f *.log
rm -rf output_*
rm -f output*
rm -f *_sel.json
rm -f localhost*
rm -f dpl-config.json
