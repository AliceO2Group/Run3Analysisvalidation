#!/bin/bash
FileIn="$1"
JSON="$2"
mkdir sockets && \
o2-analysis-hf-track-index-skims-creator -b --configuration json://$JSON --aod-memory-rate-limit 2000000000 --shm-segment-size 16000000000 --min-failure-level error | \
o2-analysis-hf-candidate-creator-2prong --doMC -b --configuration json://$JSON --aod-memory-rate-limit 2000000000 --shm-segment-size 16000000000 --min-failure-level error | \
o2-analysis-hf-d0-candidate-selector -b --configuration json://$JSON --aod-memory-rate-limit 2000000000 --shm-segment-size 16000000000 --min-failure-level error | \
o2-analysis-hf-task-d0 --doMC -b --configuration json://$JSON --aod-memory-rate-limit 2000000000 --shm-segment-size 16000000000 --min-failure-level error | \
o2-analysis-pp-qa-event-track -b --configuration json://$JSON --aod-memory-rate-limit 2000000000 --shm-segment-size 16000000000 --min-failure-level error | \
o2-analysis-pid-tpc-full -b --configuration json://$JSON --aod-memory-rate-limit 2000000000 --shm-segment-size 16000000000 --min-failure-level error | \
o2-analysis-alice3-pid-tof -b --configuration json://$JSON --aod-memory-rate-limit 2000000000 --shm-segment-size 16000000000 --min-failure-level error  --fairmq-ipc-prefix sockets && \
rm -r sockets
