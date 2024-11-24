#!/bin/bash

# single file in
#./bin/fit -f /lustre/collider/siew/calo-study/skim-data/skim_gm2offline_ana_7170070_38904.00034.root -o ./ -m blinded_9params -e 1700 -p 20000 -p 64.4 -p 0.4 -p 0 -p 5 -p 130 -p -0.008 -p 2.5 -p 20

doFit_9param () {
    ./bin/fit \
	-f $1 \
	-o $2 \
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
	-o $2 \
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

# DATA
doFit_9param /home/siew/CaloCalibration/data/skim-files/reference_run.txt /home/siew/CaloCalibration/data/fit-results/reference_run.root
#doFit_9param /home/siew/CaloCalibration/data/skim-files/spacer_run.txt /home/siew/CaloCalibration/data/fit-results/spacer_run.root

#data
#doFit_9param /home/siew/gm2/df-spectrum/data/spacer/pruned/data-spacer.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run4F_rw.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run4O_rw.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run5A_rw.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run5BC_rw.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run5P_rw.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run4F_re.txt
#doFit_9param /home/siew/gm2/df-spectrum/data/materials/skim/run5A_re.txt

# MC
#doFit_5param /home/siew/CaloCalibration/data/skim-files/offset-0p00mm.txt /home/siew/CaloCalibration/data/fit-results/offset-0p00mm.root
#doFit_5param /home/siew/CaloCalibration/data/skim-files/offset-25p0mm.txt /home/siew/CaloCalibration/data/fit-results/offset-25p0mm.txt
#doFit_5param /home/siew/CaloCalibration/data/skim-files/offset-50p0mm.txt /home/siew/CaloCalibration/data/fit-results/offset-50p0mm.txt
#doFit_5param /home/siew/CaloCalibration/data/skim-files/offset-75p0mm.txt /home/siew/CaloCalibration/data/fit-results/offset-75p0mm.txt
#doFit_5param /home/siew/CaloCalibration/data/skim-files/offset-12p5mm.txt /home/siew/CaloCalibration/data/fit-results/offset-12p5mm.txt
#doFit_5param /home/siew/CaloCalibration/data/skim-files/offset-37p5mm.txt /home/siew/CaloCalibration/data/fit-results/offset-37p5mm.txt
#doFit_5param /home/siew/CaloCalibration/data/skim-files/offset-62p5mm.txt /home/siew/CaloCalibration/data/fit-results/offset-62p5mm.txt

