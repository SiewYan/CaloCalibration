#!/bin/bash

#ls /lustre/collider/siew/pruned_datasets/MC/*.root > /home/siew/gm2/df-spectrum/data/skim/mc.txt
#ls /lustre/collider/siew/pruned_datasets/RW_Run4F/*.root > /home/siew/gm2/df-spectrum/data/skim/rw_run4f.txt
#ls /lustre/collider/siew/pruned_datasets/RE_Run4F/*.root > /home/siew/gm2/df-spectrum/data/skim/re_run4f.txt

# skim file for spacer
# /lustre/collider/siew/study_v2/spacer
DIRECTORY="/lustre/collider/siew/study_v2/spacer/skim"

CWD=`pwd`

TEST=$(echo $CWD | awk -F "/" '{print $NF}')
echo $TEST
if [[ $TEST == "scripts" ]]; then
    echo please run the script from the main directory of CaloCalibration
    exit
fi

for ioffset in offset-0p00mm  offset-12p5mm  offset-25p0mm  offset-37p5mm  offset-50p0mm  offset-62p5mm  offset-75p0mm reference_run spacer_run
do
    echo creating list ${CWD}/data/skim-files/${ioffset}.txt
    find ${DIRECTORY}/${ioffset}/ -type f -name "*.root" -size +1M > ${CWD}/data/skim-files/${ioffset}.txt
done
