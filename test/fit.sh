#!/bin/bash

# single file in
#./bin/fit -f /lustre/collider/siew/calo-study/skim-data/skim_gm2offline_ana_7170070_38904.00034.root -o ./ -m blinded_9params -e 1700 -p 20000 -p 64.4 -p 0.4 -p 0 -p 5 -p 130 -p -0.008 -p 2.5 -p 20

doFit_9param () {
    ./bin/fit \
	-f $1 \
	-o /home/siew/gm2/df-spectrum/data/materials/fit-result \
	-m blinded_9params \
	-e 1700 \
	-p 150000 \
	-p 64.4 \
	-p 0.4 \
	-p 0 \
	-p 5 \
	-p 130 \
	-p -0.008 \
	-p 2.5 \
	-p 20 \
	--range 60 \
	--range 600 \
	-n 15
}

doFit_5param () {
    ./bin/fit \
	-f $1  \
	-o /home/siew/gm2/df-spectrum/data/materials/fit-result \
	-m blinded_5params \
	-e 1600 \
	-p 100000 \
	-p 64.4 \
	-p 0.4 \
	-p 0 \
	-p 5 \
	--range 30 \
	--range 500 \
	-n 15 \
	--isMC
}

#data
#doFit_9param /home/siew/gm2/df-spectrum/data/spacer/pruned/data-spacer.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run4F_rw.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run4O_rw.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run5A_rw.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run5BC_rw.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run5P_rw.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run4F_re.txt
doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run5A_re.txt

#mc 
#doFit_5param /home/siew/gm2/df-spectrum/data/spacer/pruned/offset-0p00mm.txt 
#doFit_5param /home/siew/gm2/df-spectrum/data/materials/skim/gasgun.txt
