#!/bin/bash

./bin/ana \
    -d /home/siew/gm2/df-spectrum/data/skim/test/rw_run4f.txt \
    -d /home/siew/gm2/df-spectrum/data/fit-result/rw_run4f_cond.txt \
    -o /home/siew/gm2/df-spectrum/data/analysis \
    -c 12 \
    -x 23 \
    -s 0.9

./bin/ana \
    -d /home/siew/gm2/df-spectrum/data/skim/test/mc.txt \
    -d /home/siew/gm2/df-spectrum/data/fit-result/mc_cond.txt \
    -o /home/siew/gm2/df-spectrum/data/analysis \
    -c 12 \
    -x 23 \
    -s 1 \
    -m


./bin/ana \
    -d /home/siew/gm2/df-spectrum/data/skim/test/re_run4f.txt \
    -d /home/siew/gm2/df-spectrum/data/fit-result/re_run4f_cond.txt \
    -o /home/siew/gm2/df-spectrum/data/analysis \
    -c 12 \
    -x 23 \
    -s 0.5
