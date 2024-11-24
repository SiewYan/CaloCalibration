#!/bin/bash

#for ilog in /lustre/collider/siew/pruned_datasets/MC /lustre/collider/siew/pruned_datasets/RW_Run4F /lustre/collider/siew/pruned_datasets/RE_Run4F
for ilog in /lustre/collider/siew/study_v2/spacer/skim/offset-0p00mm
do
    for ierr in $(find $ilog/*.err -type f -size +0)
    do
	ls -trlh $ierr
	OUT=${ierr//.err}    
	ECHO=$(cat $OUT.out | grep bin/skim)
	COMMAND=`cat $OUT.out | grep bin/skim`
	echo $ECHO
	#$COMMAND
    done
done
