#!/bin/bash

#ls /lustre/collider/siew/pruned_datasets/MC/*.root > /home/siew/gm2/df-spectrum/data/skim/mc.txt
#ls /lustre/collider/siew/pruned_datasets/RW_Run4F/*.root > /home/siew/gm2/df-spectrum/data/skim/rw_run4f.txt
#ls /lustre/collider/siew/pruned_datasets/RE_Run4F/*.root > /home/siew/gm2/df-spectrum/data/skim/re_run4f.txt

# skim file for spacer
# /lustre/collider/siew/study_v2/spacer
DIRECTORY="/lustre/collider/siew/study_v2/spacer"

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
    echo creating list ${CWD}/data/art-files/${ioffset}.txt
    find ${DIRECTORY}/art-skim/${ioffset}/*/data/ -type f -name "*.root" -size +1M > ${CWD}/data/art-files/${ioffset}.txt
done

# make Data
for irun in run60787  run60788  run60789  run60791  run60866  run60867  run60868  run60869  run60870  run60871
do
    echo creating list ${CWD}/data/art-files/${irun}.txt
    ls ${DIRECTORY}/data/${irun}/*/*/data/*.root > ${CWD}/data/art-files/${irun}.txt
done

# DATA
# spacer run
COUNT=0
echo creating list ${CWD}/data/art-files/spacer_run.txt
for irun in 60866 60867 60868 60869 60870 60871
do
    ((COUNT++))
    if [[ "$COUNT" == 1 ]]; then
	ls ${DIRECTORY}/data/run${irun}/*/*/data/*.root > ${CWD}/data/art-files/spacer_run.txt
    else
	ls ${DIRECTORY}/data/run${irun}/*/*/data/*.root >> ${CWD}/data/art-files/spacer_run.txt
    fi
done


# DATA
# Reference run , missing 60790
COUNT=0
echo creating list ${CWD}/data/art-files/reference_run.txt
for irun in 60787 60788 60789 60791 #60790
do
    ((COUNT++))
    if [[ "$COUNT" == 1 ]]; then
	ls ${DIRECTORY}/data/run${irun}/*/*/data/*.root > ${CWD}/data/art-files/reference_run.txt
    else
	ls ${DIRECTORY}/data/run${irun}/*/*/data/*.root >> ${CWD}/data/art-files/reference_run.txt
    fi
done
