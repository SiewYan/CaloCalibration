#!/bin/bash

source /cvmfs/sft.cern.ch/lcg/views/LCG_102a/x86_64-centos7-gcc11-opt/setup.sh
export LD_LIBRARY_PATH=/home/siew/gm2/df-spectrum/plugins/blinder:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/home/siew/gm2/df-spectrum/plugins/blinder/rlib/include:$LD_LIBRARY_PATH

export HOME=`pwd`
cd ${HOME}

# reconWest
/home/siew/gm2/df-spectrum/bin/phase \
    -f /home/siew/gm2/df-spectrum/data/skim/rw_run4f.txt \
    -o /home/siew/gm2/df-spectrum/studies/phase-study/rw_run4f_phase.root \
    --phi0 4.11563 \
    --t_start 63.9721 \
    --t_end 605.257 \
    --omegaa 1.43938 \
    --ncore 3

# reconEast
/home/siew/gm2/df-spectrum/bin/phase \
    -f /home/siew/gm2/df-spectrum/data/skim/re_run4f.txt \
    -o /home/siew/gm2/df-spectrum/studies/phase-study/re_run4f_phase.root \
    --phi0 4.11378 \
    --t_start 63.9708 \
    --t_end 600.878 \
    --omegaa 1.43938 \
    --ncore 3

# MC
/home/siew/gm2/df-spectrum/bin/phase \
    -f /home/siew/gm2/df-spectrum/data/skim/mc.txt \
    -o /home/siew/gm2/df-spectrum/studies/phase-study/mc_phase.root \
    --phi0 3.16231 \
    --t_start 2.1971 \
    --t_end 504.218 \
    --omegaa 1.43932 \
    --isMC \
    --ncore 3
