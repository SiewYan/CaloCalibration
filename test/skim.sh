#!/bin/bash

#./bin/skim -f /home/siew/gm2/df-spectrum/data/materials/art-skim/test_run.txt \
#	   -o /home/siew/gm2/df-spectrum/data-test.root \
#	   -t "xtalTree/xtal_info" \
#	   -n 4

#./bin/skim -f /home/siew/gm2/df-spectrum/data/materials/art-skim/test_gasgun.txt \
#           -o /home/siew/gm2/df-spectrum/gasgun-test.root \
#           -t "gAnalyzer/g2phase" \
#           -n 4

# RE
./bin/skim -f /home/siew/gm2/df-spectrum/data/materials/art-skim/test_re.txt \
	   -o /home/siew/gm2/df-spectrum/test_re.root \
	   -t "crystalTreeMaker1EP/ntuple" \
	   -x \
	   -n 3
