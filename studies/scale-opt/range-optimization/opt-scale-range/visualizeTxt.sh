#!/bin/bash

# RW
for itxt in $(ls ./opt-baseOn-region/rw/*.txt)
do
    echo python plot-xtal-scale.py $itxt
    python plot-xtal-scale.py $itxt
done

for itxt in $(ls ./opt-baseOn-xtal/rw/*.txt)
do
    echo python plot-xtal-scale.py $itxt
    python plot-xtal-scale.py $itxt
done


# RE
#for itxt in $(ls ./opt-baseOn-region/re/*.txt)
#do
#    echo python plot-xtal-scale.py $itxt
#    python plot-xtal-scale.py $itxt
#done

#for itxt in $(ls ./opt-baseOn-xtal/re/*.txt)
#do
#    echo python plot-xtal-scale.py $itxt
#    python plot-xtal-scale.py $itxt
#done
