#!/bin/bash

./bin/scale-opt \
    -d /home/siew/gm2/df-spectrum/data/materials/skim/run4f_test.txt \
    -d /home/siew/gm2/df-spectrum/data/materials/fit-result/fit_run4F_rw_cond.txt \
    -m /home/siew/gm2/df-spectrum/data/materials/skim/gasgun_test.txt \
    -m /home/siew/gm2/df-spectrum/data/materials/fit-result/fit_gasgun_cond.txt \
    -c 12 \
    -x 23

