#!/bin/bash

for ilog in /lustre/collider/siew/analysis
do
    for ierr in $(find $ilog/*.err -type f -size +0)
    do
	ls -trlh $ierr
	OUT=${ierr//.err}    
	ECHO=$(cat $OUT.out | grep bin/ana)
	COMMAND=`cat $OUT.out | grep bin/ana`
	echo $ECHO
	#$COMMAND
    done
done
