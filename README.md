# CaloCalibration
code used for calorimeter calibration for gm2 experiment

## Installation

git pull into your favourite directory ```pwd``` in inpac (needed condor capability)
```
cd `pwd`
git clone git@github.com:SiewYan/CaloCalibration.git
cd CaloCalibration
# install blinder
make init
make install-blinder
```

Note: run ```make init``` every time you run the code.

## Projects

### Energy Re-calibration study

- GEANT4-based model, gm2ringsim detailed the material structure and
particle detectors used in g-2 injection line and the storage ring.
- Particle guns are used to simulate and track muons and decay positrons.
- The material effects are imprinted into the
positron energy spectrum.
- The accuracy of the total material budget can
be studied, by comparing crystal-level energy
spectra between data and simulation.

### Spacer calo study

- The material effects are imprinted into the
positron energy spectrum, together with the geometrical acceptance.
- Simular approach can be used to study the accuracy of the total material budget, together with the effect of geometrical effects (shifts),
- comparing crystal-level energy
spectra between data and simulation.

## Methodology

1. **Time window selection** : Determine the initial phase of the data and simulation, for simulation, uses 5 parameter's fit function; data uses 9 parameter's fit function.
2. **Phase validation** : (optional) check the modulation of energy spectrum according to the angle between the muon spin and momentum.
3. **Energy Calibration V1 strategy** : In the time window, energy threshold (1600 MeV for MC ; 1700 MeV for data), energy range (above 1000 MeV); perform a unidimensional scan using KS test on energy spectra at xtal level.
4. **Energy Calibration V2 strategy** : In the time window, energy threshold (1600 MeV for MC ; 1700 MeV for data), optimized energy range (via quantile style dual-directional scan); perform a unidimensional scan using KS test on energy spectra at xtal level.
5. **Energy Scale validation** : Validates the optimized energy scale, comparing data and MC, with normalization performed on data ( MC/Data(Eenergy Range) ) in the optimized range.

### Goal of the study

The obtained energy scale depicts the difference between data and MC, in which the data loss/gain signifies unaccounted effects. We trust the MC to be the truth.


### Technical walkthrough

The following instruction for a quick walktrough on how to run the analysis. The analysis consists of:
 - skim
 - initial phase determination
 - parameter scan
 - validation of plot

#### Skim

The art format files are available:

 - Data : ```/lustre/collider/siew/study_v2/spacer/data```
 - **Spacer Run** : 60866 , 60867 , 60868 , 60869 , 60870 , 60871
 - **Reference Run** : 60787 , 60788 , 60789 , 60790 , 60791


| Spacer depth    | Calorimeters |
| -------- | ------- |
| 0.0    | 10, 14, 17    |
| 12.5    | 1, 5, 12, 19     |
| 25.0    | 2, 6, 13, 22    |
| 37.5    | 3, 7, 18, 23    |
| 50.0    | 4, 8, 15, 24    |
| 62.5    | 9, 20    |
| 75.0    | 11, 16, 21    |

 - MC : ```/lustre/collider/siew/study_v2/spacer/art-skim```

 - offset-0p00mm,  offset-12p5mm, offset-25p0mm,  offset-37p5mm,  offset-50p0mm,  offset-62p5mm,  offset-75p0mm

The filelists are prepared in ```CaloCalibration/data/``` ; to run the skim

```
make skim
./bin/skim -f /home/siew/gm2/df-spectrum/data/materials/art-skim/test_re.txt \
	   -o /home/siew/gm2/df-spectrum/test_re.root \
	   -t "crystalTreeMaker1EP/ntuple" \
	   -x \
	   -n 3
```
- ```-f``` input list
- ```-o``` output root file
- ```-t``` tree name
-  ```-x``` toggle between Recon East and West
-  ```-n``` number of core used in processing

Source code : ```src/skim.sh```

Once the local test is ok, run the production job:

```
python submission/skim_spacer_prod.py
```

The filelist can be define here : https://github.com/SiewYan/CaloCalibration/blob/main/submission/skim_spacer_prod.py#L100-L106
Make sure the output direction is corrected defined to avoid writing files into other user folder.

#### Initial Phase determination

running the fit, example script is in ```test/fit.sh```. Code compilation is needed.

Source code : ```src/fit.cc```

```
make fit
# execute
    ./bin/fit \
	-f $1  \
	-o /home/siew/gm2/df-spectrum/data/materials/fit-result \
	-m blinded_5params \
	-e 1600 \
	-p 100000 \
	-p 64.4 \
	-p 0.4 \
	-p 0 \
	-p 5 \
	--range 30 \
	--range 500 \
	-n 15 \
	--isMC
```

Once the fit is run, produces a wiggle plot with the fit on, and the list of extracted parameters, 
store in ```data```; The initial phase will be determined.

Note:
 - Data uses 9 parameter fit , ```-m blinded_9params```
 - MC uses 5 parameter fit, ```-m blinded_5params```

Next, do the validation of the phase (optional)

Source code : ```src/phaseval.cc```

```
make phase
# execute
/home/siew/gm2/df-spectrum/bin/phase \
    -f /home/siew/gm2/df-spectrum/data/skim/rw_run4f.txt \
    -o /home/siew/gm2/df-spectrum/studies/phase-study/rw_run4f_phase.root \
    --phi0 4.11563 \
    --t_start 63.9721 \
    --t_end 605.257 \
    --omegaa 1.43938 \
    --ncore 3
```

...

#### Run scale optimization

change the outoput directory here: ```https://github.com/SiewYan/CaloCalibration/blob/main/src/scale-optimizer.cc#L37```

```
make scale-opt
# execute
./bin/scale-opt \
    -d /home/siew/gm2/df-spectrum/data/materials/skim/run4f_test.txt \
    -d /home/siew/gm2/df-spectrum/data/materials/fit-result/fit_run4F_rw_cond.txt \
    -m /home/siew/gm2/df-spectrum/data/materials/skim/gasgun_test.txt \
    -m /home/siew/gm2/df-spectrum/data/materials/fit-result/fit_gasgun_cond.txt \
    -c 12 \
    -x 23
```
