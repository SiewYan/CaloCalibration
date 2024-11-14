#!/bin/bash

set -e

source /cvmfs/sft.cern.ch/lcg/views/LCG_102a/x86_64-centos7-gcc11-opt/setup.sh
export LD_LIBRARY_PATH=/home/siew/gm2/df-spectrum/plugins/blinder:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/home/siew/gm2/df-spectrum/plugins/blinder/rlib/include:$LD_LIBRARY_PATH

export HOME=`pwd`
cd ${HOME}

make fit

# MC
function fitmc(){
    ./bin/fit \
	-f $1  \
	-o $2 \
	-m blinded_5params \
	-e 1600 \
	-p 300000 \
	-p 64.4 \
	-p 0.4 \
	-p 0 \
	-p 5 \
	--range 30 \
	--range 500 \
	-n 15 \
	--isMC
}

function fitrw(){
    ./bin/fit \
	-f $1 \
	-o $2 \
	-m blinded_9params \
	-e 1700 \
	-p 2500000 \
	-p 64.4 \
	-p 0.4 \
	-p 0 \
	-p 5 \
	-p 130 \
	-p -0.008 \
	-p 2.5 \
	-p 20 \
	-n 15 \
	--range 60 \
	--range 600
}

function fitre(){
    ./bin/fit \
	-f $1 \
	-o $2 \
	-m blinded_9params \
	-e 1700 \
	-p 2500000 \
	-p 64.4 \
	-p 0.4 \
	-p 0 \
	-p 5 \
	-p 130 \
	-p -0.008 \
	-p 2.5 \
	-p 20 \
	-n 15 \
	--range 60 \
	--range 600
}

#fitmc /home/siew/gm2/df-spectrum/data/skim/mc.txt /home/siew/gm2/df-spectrum/data/fit-result/mc.root
fitmc /home/siew/gm2/df-spectrum/data/skim/mc_smear.txt /home/siew/gm2/df-spectrum/data/fit-result/mc_smear.root
#fitrw /home/siew/gm2/df-spectrum/data/skim/rw_run4f.txt /home/siew/gm2/df-spectrum/data/fit-result/rw_run4f.root
#fitre /home/siew/gm2/df-spectrum/data/skim/re_run4f.txt /home/siew/gm2/df-spectrum/data/fit-result/re_run4f.root
