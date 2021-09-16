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

if [ -f $nameSetupFile ]
then
    rm $nameSetupFile
fi

touch $nameSetupFile


### Starting creating setup file
echo "/random/setSeeds " $seedr " " $seedr  "   " >> $nameSetupFile

echo "/process/optical/verbose 0           " >> $nameSetupFile

echo "/detector/world/dimensions 6. 6. 6. m" >> $nameSetupFile

echo "### beam pipe                        " >> $nameSetupFile
echo "/detector/pipe/radius 1.6 cm         " >> $nameSetupFile
echo "/detector/pipe/length 200. cm	   " >> $nameSetupFile
echo "/detector/pipe/thickness 500 um      " >> $nameSetupFile
echo "#                     " >> $nameSetupFile


echo "/detector/enable ABSO								                                " >> $nameSetupFile
echo "###SENSITIVE LAYERS                     " >> $nameSetupFile
echo "#			   rmin	rmax runit length	length_unit	material add_sd	                      " >> $nameSetupFile
echo "# First sensitive layer to register what comes out					                                " >> $nameSetupFile
echo "/detector/ABSO/addCylinder " $detAfterAbs1Rmin  " "   $detAfterAbs1Rmax  "  cm    600.    cm    G4_Galactic   true  " >> $nameSetupFile
echo "# Second sensitive layer to register what comes out                                                                      " >> $nameSetupFile
echo "/detector/ABSO/addCylinder " $detAfterAbs2Rmin  " "   $detAfterAbs2Rmax  "  cm    600.    cm    G4_Galactic   true  " >> $nameSetupFile
echo "#                     " >> $nameSetupFile
echo "#                     " >> $nameSetupFile
echo "### inner tracker           radius      length          thickness " >> $nameSetupFile
echo "/detector/tracker/addLayer  0.5  cm    16  cm             50. um" >> $nameSetupFile
echo "/detector/tracker/addLayer  1.2  cm    16  cm             50. um" >> $nameSetupFile
echo "/detector/tracker/addLayer  2.5  cm    16  cm             50. um" >> $nameSetupFile
echo "/detector/tracker/addLayer  3.75 cm    16  cm             50. um" >> $nameSetupFile
echo "#                     " >> $nameSetupFile

echo "### outer tracker           radius      length          thickness" >> $nameSetupFile
echo "/detector/tracker/addLayer   7. cm      40   cm       500. um" >> $nameSetupFile
echo "/detector/tracker/addLayer  12. cm      60.  cm       500. um" >> $nameSetupFile
echo "/detector/tracker/addLayer  20. cm      80.  cm       500. um" >> $nameSetupFile
echo "/detector/tracker/addLayer  30. cm     120.  cm       500. um" >> $nameSetupFile
echo "/detector/tracker/addLayer  45. cm     180.  cm       500. um" >> $nameSetupFile
echo "/detector/tracker/addLayer  60. cm     240.  cm       500. um" >> $nameSetupFile
echo "/detector/tracker/addLayer  80. cm     320.  cm       500. um" >> $nameSetupFile
echo "/detector/tracker/addLayer 100. cm     400.  cm       500. um" >> $nameSetupFile
echo "#                     " >> $nameSetupFile
echo "#                     " >> $nameSetupFile


echo "### EMCAL                                                                                                          " >> $nameSetupFile

echo "#											                                " >> $nameSetupFile
echo "#                                                                                                    " >> $nameSetupFile


echo "#			   rmin	rmax runit length	length_unit	material add_sd	    x y z                          " >> $nameSetupFile

if [ ! -f EmCalThicknessVsZ.txt ]; then
  root -q -b -l GetEmCalThicknessVsZ.C\($emcalRmin,${emcalThicknessEta0}\)
  echo "File EmCalThicknessVsZ.txt did not exit, I created it !"
fi

while read z emcalThickness; do

    emcalRmaxVsZ=$(($emcalRmin+$emcalThickness))
    echo "/detector/ABSO/addCylinder " $emcalRmin " " $emcalRmaxVsZ "  cm     5     cm       G4_PbWO4  false 0 0" $z " " >> $nameSetupFile

done <EmCalThicknessVsZ.txt


echo "#			   rmin	rmax runit length	length_unit	material add_sd	                      " >> $nameSetupFile
echo "# ABSORBER								                                " >> $nameSetupFile

if [ ! -f AbsoThicknessVsZ.txt ]; then
  root -q -b -l GetAbsoThicknessVsZ.C\($absRmin,${abs}\)
  echo "File AbsoThicknessVsZ.txt did not exit, I created it !"
fi

while read zabso absoThickness halflength; do

    absoRmaxVsZ=$(($absRmin+$absoThickness))
    echo "/detector/ABSO/addCylinder " $absRmin " " $absoRmaxVsZ "  cm    " " $halflength  " "  cm       G4_Fe  false 0 0" $zabso " " >> $nameSetupFile

done <AbsoThicknessVsZ.txt

echo "### initialize                          " >> $nameSetupFile
echo "/run/initialize			      " >> $nameSetupFile
echo "					      " >> $nameSetupFile
echo "### magnetic field		      " >> $nameSetupFile
echo "/globalField/verbose 1		      " >> $nameSetupFile
echo "/globalField/setValue 0. 0. 0.5 tesla   " >> $nameSetupFile
echo "					      " >> $nameSetupFile
echo "### transport			      " >> $nameSetupFile
echo "/stacking/transport all		      " >> $nameSetupFile
echo "/tracking/verbose 0                     " >> $nameSetupFile


echo "#######################      " >> $nameSetupFile
echo "### generator GUN		   " >> $nameSetupFile
echo "/generator/select gun	   " >> $nameSetupFile
echo "/gun/particle " $partName "  " >> $nameSetupFile
echo "/gun/momentumAmp " $en " GeV " >> $nameSetupFile
echo "/gun/position 0 0 0 cm	   " >> $nameSetupFile
echo "/gun/direction 1 0 " $pzOverpt "    " >> $nameSetupFile
echo "/gun/number 1		   " >> $nameSetupFile
echo "# z is set in order to match a specific eta value when x=1 and y=0 " >> $nameSetupFile
echo "# specific (eta,z) corresponding values: " >> $nameSetupFile
echo "# (0,0), (0.5,0.52), (1,1.18), (1.5,2.13), (2,3.63) " >> $nameSetupFile
echo "				   " >> $nameSetupFile
echo "### io			   " >> $nameSetupFile
echo "/io/prefix " $label "	   " >> $nameSetupFile
echo "/io/saveParticles false	   " >> $nameSetupFile
echo "#######################	   " >> $nameSetupFile
echo "				   " >> $nameSetupFile
echo "### run			   " >> $nameSetupFile
echo "/run/beamOn " $nEvents "     " >> $nameSetupFile

