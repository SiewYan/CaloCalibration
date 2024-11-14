#!/bin/bash

#for ilog in /lustre/collider/siew/pruned_datasets/MC /lustre/collider/siew/pruned_datasets/RW_Run4F /lustre/collider/siew/pruned_datasets/RE_Run4F
for ilog in /lustre/collider/siew/pruned_datasets/MC_v2
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
