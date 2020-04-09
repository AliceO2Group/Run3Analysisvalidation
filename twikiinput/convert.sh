#!/bin/bash

rm event_stat.root outputs_valid EventStat_temp.root *.txt


rm *AO2D*
cp ../codeHF/convertAO2D.C .

rm AO2D_PbPbsmall_v4.root
LISTPBPB=AliESDs_PbPbsmall.txt
ls AliESDs_PbPbsmall.root >> $LISTPBPB
root -q -l "convertAO2D.C(\"$LISTPBPB\", 0)"  
mv AO2D.root AO2D_PbPbsmall_v4.root

rm AO2D_ppK0starToyMC_v4.root
LISTPP=AliESDs_ppK0starToyMC.txt
ls AliESDs_ppK0starToyMC.root >> $LISTPP
root -q -l "convertAO2D.C(\"$LISTPP\", 0)"  
mv AO2D.root AO2D_ppK0starToyMC_v4.root

rm event_stat.root outputs_valid EventStat_temp.root *.txt

rm AnalysisResults.root

o2-analysis-vertexing-hf --aod-file AO2D_PbPbsmall_v4.root -b
mv AnalysisResults.root AnalysisResults_PbPbsmall_v4.root
o2-analysis-vertexing-hf --aod-file AO2D_ppK0starToyMC_v4.root -b
mv AnalysisResults.root AnalysisResults_ppK0starToyMC_v4.root
rm  *localhost*
