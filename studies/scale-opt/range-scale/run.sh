#!/bin/bash

#plot-xtal-scale.py scale_calo24-xtal45_spacer_run.txt

CWD=`pwd`
DIR="/lustre/collider/junkai.ng/CaloCalibration_output/03_scale_scan/"
RUN="spacer"
RECON="run"

mkdir -p ./plots/${RUN}

for icalo in $(seq 1 24)
do
    printf -v fn "%02d" $icalo

    count=0
    for ixtal in $(seq 0 53)
    do
	printf -v fnn "%02d" $ixtal
	echo plotting calo ${fn} xtal ${fnn} Run ${RUN}
	python plot-scale-range.py ${DIR}/scale_calo${fn}-xtal${fnn}_${RUN}_${RECON}.txt ./plots/${RUN} ${RUN}
	((count++))
    done
done
