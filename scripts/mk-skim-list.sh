#!/bin/bash

#ls /lustre/collider/siew/pruned_datasets/MC/*.root > /home/siew/gm2/df-spectrum/data/skim/mc.txt
#ls /lustre/collider/siew/pruned_datasets/RW_Run4F/*.root > /home/siew/gm2/df-spectrum/data/skim/rw_run4f.txt
#ls /lustre/collider/siew/pruned_datasets/RE_Run4F/*.root > /home/siew/gm2/df-spectrum/data/skim/re_run4f.txt

CWD=`pwd`

TEST=$(echo $CWD | awk -F "/" '{print $NF}')
echo $TEST
if [[ $TEST == "scripts" ]]; then
    echo please run the script from the main directory of CaloCalibration
    exit
fi

# make MC
for ioffset in offset-0p00mm  offset-12p5mm  offset-25p0mm  offset-37p5mm  offset-50p0mm  offset-62p5mm  offset-75p0mm
do
    echo creating list ${CWD}/data/${ioffset}.txt
    ls /lustre/collider/siew/study_v2/spacer/art-skim/${ioffset}/*/data/*.root > ${CWD}/data/${ioffset}.txt
done

# make Data
for irun in run60787  run60788  run60789  run60791  run60866  run60867  run60868  run60869  run60870  run60871
do
    echo creating list ${CWD}/data/${irun}.txt
    ls /lustre/collider/siew/study_v2/spacer/data/${irun}/*/*/data/*.root > ${CWD}/data/${irun}.txt
done
