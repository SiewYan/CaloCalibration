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

based on spacer calo study

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

