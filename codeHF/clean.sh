#!/bin/bash

echo -e "\nCleaning"

rm -f *.root
rm -f *.txt
rm -f *.pdf
rm -f *.log
rm -f output*
rm -f localhost*
git checkout cv_K0star.pdf
git checkout cvprimary_K0star.pdf
rm -f dpl-config.json

