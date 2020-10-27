#!/bin/bash

. ${O2_ROOT}/share/scripts/jobutils.sh

# chain of algorithms from MC and reco

# default interaction rates in kHz
# intRatePP=400
# intRatePbPb=50

# default collision system
# collSyst="pp"

# generPP="pythia8"
# generPbPb="pythia8hi"

# default sim engine
# engine="TGeant3"

taskwrapper sim.log o2-sim -n 100 --configKeyValue "Diamond.width[2]=6." -g pythia8 -e TGeant3 -j 6 -m PIPE ITS FT0 TPC TOF MFT

taskwrapper digi.log o2-sim-digitizer-workflow -b --run --shm-segment-size 10000000000 --interactionRate 400000 --tpc-lanes 1

taskwrapper tpcreco.log o2-tpc-reco-workflow $gloOpt --tpc-digit-reader \"--infile tpcdigits.root\" --input-type digits --output-type clusters,tracks  --tpc-track-writer \"--treename events --track-branch-name Tracks --trackmc-branch-name TracksMCTruth\"

taskwrapper itsreco.log  o2-its-reco-workflow --trackerCA --async-phase $gloOpt

taskwrapper mftreco.log  o2-mft-reco-workflow  $gloOpt
echo "Return status of mftreco: $?"

echo "Running FT0 reco flow"
#needs FT0 digitized data
taskwrapper ft0reco.log o2-ft0-reco-workflow $gloOpt
echo "Return status of ft0reco: $?"

echo "Running ITS-TPC macthing flow"
#needs results of o2-tpc-reco-workflow, o2-its-reco-workflow and o2-fit-reco-workflow
taskwrapper itstpcMatch.log o2-tpcits-match-workflow $gloOpt --tpc-track-reader \"tpctracks.root\" --tpc-native-cluster-reader \"--infile tpc-native-clusters.root\"
echo "Return status of itstpcMatch: $?"

echo "Running ITSTPC-TOF macthing flow"
#needs results of TOF digitized data and results of o2-tpcits-match-workflow
taskwrapper tofMatch.log o2-tof-reco-workflow $gloOpt
echo "Return status of its-tpc-tof match: $?"

echo "Running primary vertex finding flow"
#needs results of TPC-ITS matching and FIT workflows
taskwrapper pvfinder.log o2-primary-vertexing-workflow $gloOpt
echo "Return status of primary vertexing: $?"

echo "Producing AOD"
#aod producer
#currently writes bc table, collisions, tracks, ft0 information
#note: task also writes tracks that do not have attached vertices, in that case a track has collisionId = -1
taskwrapper aod.log o2-aod-producer-workflow --aod-writer-keep dangling
echo "Return status of AOD production: $?"