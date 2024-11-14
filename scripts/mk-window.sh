#!/bin/bash

set -e

make time

/home/siew/gm2/df-spectrum/bin/time -f /home/siew/gm2/df-spectrum/data/fit-result/mc.root -t 0 -t 500
/home/siew/gm2/df-spectrum/bin/time -f /home/siew/gm2/df-spectrum/data/fit-result/rw_run4f.root -t 60 -t 600
/home/siew/gm2/df-spectrum/bin/time -f /home/siew/gm2/df-spectrum/data/fit-result/re_run4f.root -t 60 -t 600
