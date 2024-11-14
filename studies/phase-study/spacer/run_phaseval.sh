#!/bin/bash

source /cvmfs/sft.cern.ch/lcg/views/LCG_102a/x86_64-centos7-gcc11-opt/setup.sh
export LD_LIBRARY_PATH=/home/siew/gm2/df-spectrum/plugins/blinder:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/home/siew/gm2/df-spectrum/plugins/blinder/rlib/include:$LD_LIBRARY_PATH

export HOME=`pwd`
cd ${HOME}

# reference
/home/siew/gm2/df-spectrum/bin/phase \
    -f /home/siew/gm2/df-spectrum/data/spacer/pruned/data-reference.txt \
    -o /home/siew/gm2/df-spectrum/studies/phase-study/spacer/data-reference_phase.root \
    --phi0 4.12135 \
    --t_start 63.9742 \
    --t_end 605.243 \
    --omegaa 1.43942 \
    --ncore 3

# spacer
/home/siew/gm2/df-spectrum/bin/phase \
    -f /home/siew/gm2/df-spectrum/data/spacer/pruned/data-spacer.txt \
    -o /home/siew/gm2/df-spectrum/studies/phase-study/spacer/data-spacer_phase.root \
    --phi0 4.12313 \
    --t_start 63.9766 \
    --t_end 605.256 \
    --omegaa 1.4394 \
    --ncore 3

# MC
#/home/siew/gm2/df-spectrum/bin/phase \
#    -f /home/siew/gm2/df-spectrum/data/skim/mc.txt \
#    -o /home/siew/gm2/df-spectrum/studies/phase-study/mc_phase.root \
#    --phi0 3.16231 \
#    --t_start 2.1971 \
#    --t_end 504.218 \
#    --omegaa 1.43932 \
#    --isMC \
#    --ncore 3
