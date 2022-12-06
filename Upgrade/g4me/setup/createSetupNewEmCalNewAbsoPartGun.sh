#!/bin/bash

partName=$1
en=$2
abs=$3
seedr=$4
nEvents=$5
emcalThicknessEta0=30
pzOverpt=$6

emcalRmin=130
emcalRmax=$((emcalRmin+emcalThicknessEta0))

absRmin=$((emcalRmax+2))
absRmax=$((absRmin+abs))

detAfterAbs1Rmin=$((absRmax+5)) # inner radius of the first muonId active layer
detAfterAbs1Rmax=$((absRmax+6)) # outer radius

detAfterAbs2Rmin=$((detAfterAbs1Rmax+15)) # inner radius of the second muonId active layer
detAfterAbs2Rmax=$((detAfterAbs1Rmax+16)) # outer radius

# to avoid "+" and "-" symbols in the filename
partNameLabel="${partName/+/Plus}"
partNameLabel="${partNameLabel/-/Minus}"

label=ParticleGun_${partNameLabel}_${en}GeV_abs_${abs}cm_pzOverpt_${pzOverpt}_seed_${seedr}
nameSetupFile=setup$label.mac

### Starting creating setup file
if [ -f "$nameSetupFile" ]; then
  rm "$nameSetupFile"
fi
{
  echo "/random/setSeeds $seedr $seedr"
  echo "/process/optical/verbose 0"
  echo "/detector/world/dimensions 6. 6. 6. m"
  echo "### beam pipe"
  echo "/detector/pipe/radius 1.6 cm"
  echo "/detector/pipe/length 200. cm"
  echo "/detector/pipe/thickness 500 um"
  echo "#"
  echo "/detector/enable ABSO"
  echo "###SENSITIVE LAYERS"
  echo "#         rmin  rmax runit length  length_unit  material add_sd"
  echo "# First sensitive layer to register what comes out"
  echo "/detector/ABSO/addCylinder $detAfterAbs1Rmin $detAfterAbs1Rmax    cm    600.    cm    G4_Galactic   true"
  echo "# Second sensitive layer to register what comes out"
  echo "/detector/ABSO/addCylinder $detAfterAbs2Rmin $detAfterAbs2Rmax    cm    600.    cm    G4_Galactic   true"
  echo "#"
  echo "#"
  echo "### inner tracker           radius      length          thickness"
  echo "/detector/tracker/addLayer  0.5  cm    16  cm             50. um"
  echo "/detector/tracker/addLayer  1.2  cm    16  cm             50. um"
  echo "/detector/tracker/addLayer  2.5  cm    16  cm             50. um"
  echo "/detector/tracker/addLayer  3.75 cm    16  cm             50. um"
  echo "#"
  echo "### outer tracker           radius      length          thickness"
  echo "/detector/tracker/addLayer   7. cm      40   cm       500. um"
  echo "/detector/tracker/addLayer  12. cm      60.  cm       500. um"
  echo "/detector/tracker/addLayer  20. cm      80.  cm       500. um"
  echo "/detector/tracker/addLayer  30. cm     120.  cm       500. um"
  echo "/detector/tracker/addLayer  45. cm     180.  cm       500. um"
  echo "/detector/tracker/addLayer  60. cm     240.  cm       500. um"
  echo "/detector/tracker/addLayer  80. cm     320.  cm       500. um"
  echo "/detector/tracker/addLayer 100. cm     400.  cm       500. um"
  echo "#"
  echo "#"
  echo "### EMCAL"
  echo "#"
  echo "#"
  echo "#         rmin  rmax runit length  length_unit  material add_sd      x y z"
} >> "$nameSetupFile"

if [ ! -f EmCalThicknessVsZ.txt ]; then
  root -q -b -l GetEmCalThicknessVsZ.C\($emcalRmin,${emcalThicknessEta0}\)
  echo "File EmCalThicknessVsZ.txt did not exit, I created it !"
fi

while read -r z emcalThickness; do
  emcalRmaxVsZ=$((emcalRmin + emcalThickness))
  echo "/detector/ABSO/addCylinder $emcalRmin $emcalRmaxVsZ   cm     5     cm       G4_PbWO4  false 0 0 $z" >> "$nameSetupFile"
done <EmCalThicknessVsZ.txt

{
  echo "#         rmin  rmax runit length  length_unit  material add_sd"
  echo "# ABSORBER"
} >> "$nameSetupFile"

if [ ! -f AbsoThicknessVsZ.txt ]; then
  root -q -b -l GetAbsoThicknessVsZ.C\("$absRmin","$abs"\)
  echo "File AbsoThicknessVsZ.txt did not exit, I created it !"
fi

while read -r zabso absoThickness halflength; do
  absoRmaxVsZ=$((absRmin + absoThickness))
  echo "/detector/ABSO/addCylinder $absRmin $absoRmaxVsZ   cm     $halflength    cm       G4_Fe  false 0 0 $zabso" >> "$nameSetupFile"
done <AbsoThicknessVsZ.txt

{
  echo "### initialize"
  echo "/run/initialize"
  echo ""
  echo "### magnetic field"
  echo "/globalField/verbose 1"
  echo "/globalField/setValue 0. 0. 0.5 tesla"
  echo ""
  echo "### transport"
  echo "/stacking/transport all"
  echo "/tracking/verbose 0"
  echo "#######################"
  echo "### generator GUN"
  echo "/generator/select gun"
  echo "/gun/particle $partName"
  echo "/gun/momentumAmp  $en GeV"
  echo "/gun/position 0 0 0 cm"
  echo "/gun/direction 1 0 $pzOverpt"
  echo "/gun/number 1"
  echo "# z is set in order to match a specific eta value when x=1 and y=0"
  echo "# specific (eta,z) corresponding values:"
  echo "# (0,0), (0.5,0.52), (1,1.18), (1.5,2.13), (2,3.63)"
  echo ""
  echo "### io"
  echo "/io/prefix $label"
  echo "/io/saveParticles false"
  echo "#######################"
  echo ""
  echo "### run"
  echo "/run/beamOn $nEvents"
} >> "$nameSetupFile"
