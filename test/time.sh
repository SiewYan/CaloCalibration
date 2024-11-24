#!/bin/bash

# DATA
./bin/time -f /home/siew/CaloCalibration/data/fit-results/reference_run.root -t 0 -t 500 

# MC
./bin/time -f /home/siew/CaloCalibration/data/fit-results/offset-0p00mm.root -t 60 -t 600

