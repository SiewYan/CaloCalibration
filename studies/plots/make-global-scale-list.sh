#!/bin/bash

CWD=`pwd`

var=0
for i in $(seq 1 24)
do
    if [ $var -eq 0 ]; then
	cat $CWD/calo-${i}/condor_scale_xtal_ks.dat | awk -F ", " '{print $1" "$2" "$5}' > global-ks-scale.dat
    else
	cat $CWD/calo-${i}/condor_scale_xtal_ks.dat | awk -F ", " '{print $1" "$2" "$5}' >> global-ks-scale.dat
    fi
    ((var++))
done
