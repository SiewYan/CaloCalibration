#!/bin/bash

#plot-xtal-scale.py scale_calo10-xtal36_rw_run4f.txt

DIR=/home/siew/gm2/df-spectrum/data/scale-opt
RECON="rw"

#for icalo in 3 6 9 11 13 15 17 19 21 24
for icalo in 17
do
    printf -v fn "%02d" $icalo
    count=0
    for ixtal in $(seq 0 53)
    do
	printf -v fnn "%02d" $ixtal
	#echo "python plot-scale-opt.py ${DIR}/scale_calo${fn}-xtal${fnn}_rw_run4f.txt /home/siew/gm2/df-spectrum/studies/scale-opt/scale-rw"
	#python plot-scale-opt.py ${DIR}/scale_calo${fn}-xtal${fnn}_rw_run4f.txt /home/siew/gm2/df-spectrum/studies/scale-opt/scale-rw
	echo plotting calo ${fn} xtal ${fnn}
	if [[ ${count} == 0 ]];
	then
	    XMEAN=$(python plot-scale-opt.py ./txts/scale_calo${fn}-xtal${fnn}_${RECON}_run4f.txt ./plots)
	    echo "$count ${XMEAN}" > ${RECON}_calo-${fn}_all_xtal-mean.txt
	else
	    XMEAN=$(python plot-scale-opt.py ./txts/scale_calo${fn}-xtal${fnn}_${RECON}_run4f.txt ./plots)
	    echo "$count ${XMEAN}" >> ${RECON}_calo-${fn}_all_xtal-mean.txt
	fi
	((count++))
    done
    scp ${RECON}_calo-${fn}_all_xtal-mean.txt ${RECON}_calo-${fn}_all_xtal-mean_in.txt
    MEAN=$(python mean.py ${RECON}_calo-${fn}_all_xtal-mean_in.txt)
    rm ${RECON}_calo-${fn}_all_xtal-mean_in.txt
    echo "99 ${MEAN}" >> ${RECON}_calo-${fn}_all_xtal-mean.txt
done
