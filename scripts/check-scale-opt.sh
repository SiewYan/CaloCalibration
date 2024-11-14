#!/bin/bash

LOG="/lustre/collider/siew/scale-opt"

for ierr in $(find $LOG/*.out -type f -size +0)
do
    #ls -trlh $ierr
    OUT=${ierr//.out}
    ECHO=$(cat $OUT.out | grep DONE)
    #COMMAND=`cat $OUT.out | grep ./bin/skim-re`
    echo $OUT
    echo $ECHO
    #$COMMAND
done
