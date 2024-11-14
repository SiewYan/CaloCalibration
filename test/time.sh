#!/bin/bash

#./bin/time -f /home/siew/gm2/df-spectrum/data/spacer/fit-result/fit_data-spacer.root -t 60 -t 600
#./bin/time -f /home/siew/gm2/df-spectrum/data/spacer/fit-result/fit_data-reference.root -t 60 -t 600
#./bin/time -f /home/siew/gm2/df-spectrum/data/spacer/fit-result -t 0 -t 500

# run4
./bin/time -f /home/siew/gm2/df-spectrum/data/materials/fit-result/fit_run4F_rw.root -t 60 -t 600
./bin/time -f /home/siew/gm2/df-spectrum/data/materials/fit-result/fit_run4O_rw.root -t 60 -t 600

# run5
./bin/time -f /home/siew/gm2/df-spectrum/data/materials/fit-result/fit_run5A_rw.root -t 60 -t 600
./bin/time -f /home/siew/gm2/df-spectrum/data/materials/fit-result/fit_run5BC_rw.root -t 60 -t 600
./bin/time -f /home/siew/gm2/df-spectrum/data/materials/fit-result/fit_run5P_rw.root -t 60 -t 600

# mc
./bin/time -f /home/siew/gm2/df-spectrum/data/materials/fit-result/fit_gasgun.root -t 0 -t 500


# RE
./bin/time -f /home/siew/gm2/df-spectrum/data/materials/fit-result/fit_run4F_rw.root -t 60 -t 600
./bin/time -f /home/siew/gm2/df-spectrum/data/materials/fit-result/fit_run5A_rw.root -t 60 -t 600
