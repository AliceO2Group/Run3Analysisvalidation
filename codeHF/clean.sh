#!/bin/bash

echo "Cleaning"

rm -f *.root
rm -f *.txt
rm -f output*
rm -f *.pdf
rm -f localhost*
git checkout cv_K0star.pdf
git checkout cvprimary_K0star.pdf
rm -f dpl-config.json

