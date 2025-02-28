import ROOT
import os, sys
from collections import OrderedDict

from drawxtal import *

import pandas as pd

ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(0)

#df = pd.read_csv("./calib-scale-range/condor_scale_xtal_ks.dat", sep=",",names=["calo","xtal","range1","range2","scale"],header=None)
#print(df)
#tmp=df[(df["calo"]==11)&(df["xtal"]==35)]
#print(float(tmp["scale"])*2)

def getGraph(calibFile, outf, icalo, ixtal):

    fcalib = ROOT.TFile(calibFile,"READ")
    ksgraph = fcalib.Get("Graph")

    c1 = ROOT.TCanvas("c1", "c1", 800 , 600 )
    c1.cd()

    ksgraph.Draw()
    c1.Print( '%s/calibrator_calo-%s_xtal-%s.png' %( outf , icalo, ixtal ) )

pass
        
       
def unscaled(directory, mcdirectory , calopath ,icalo, ixtal, thisrange):

    compare(
        "%s/data-run4F_calo%s_xtal_%s.root" %(directory,icalo,ixtal)
        , "%s/mc-gasgun_calo%s_xtal_%s.root" %(mcdirectory,icalo,ixtal)
        , calopath
        , name.replace(" ","-")
        , "calo %s xtal %s Inclusive no scaled" %(icalo,ixtal)
        , "energy0check"
        , [ thisrange["range1"][ixtal] , thisrange["range2"][ixtal] ]
        , snorm=1
        , ratio=1
        , poisson=False
        , logy=True
        , rw=True
    )
    
pass

#directory="/home/shoh/Works/gm2/study/result-analysis/analysis_smear_mc_rw"
#mcdirectory="/home/shoh/Works/gm2/study/result-analysis/analysis_smear_mc_rw"
#calibdirectory="/home/shoh/Works/gm2/study/calibrator-study/calibrator"
directory="/home/shoh/Works/gm2/study/result-analysis/analyses_re"
mcdirectory="/home/shoh/Works/gm2/study/result-analysis/analyses_re" 
calibdirectory="/home/shoh/Works/gm2/study/result-calibrator/calibrator-re"
cwd = os.getcwd()
outpath="%s/plot-per-xtal-re" %cwd

if not os.path.exists(outpath):
    os.system('mkdir '+outpath)

# calo
for icalo in range(1, 25, 1):   
    print("Running calo :  ", icalo)
    #if icalo != 23 : continue;
    #if icalo == 24 : continue;
    
    calopath = "%s/calo-%s" %(outpath,icalo)
    if not os.path.exists(calopath):
        os.system('mkdir '+calopath)

    # load calibrator
    df = pd.read_csv("%s/calo-%s/condor_scale_xtal_ks.dat" %(calibdirectory,icalo) , sep=",",names=["calo","xtal","range1","range2","scale"],header=None)
        
    # xtal bin
    for ixtal in range(0, 54, 1):
        print("Running xtal : ", ixtal)
        
        calibrator = df[(df["calo"]==icalo)&(df["xtal"]==ixtal)]
        #print(calibrator)
        #if ixtal != 7 : continue; ##################################

        # calibrator
        getGraph(
            "%s/calo-%s/calibrator_calo_%s_xtal_%s.root" %(calibdirectory,icalo,icalo,ixtal)
            , calopath
            , icalo
            , ixtal
        )
        
        ratioplot=OrderedDict()
        # phase bin
        for ibin in range(0, 11, 1):
            
            name = "calo %s xtal %s phase bin %s" %(icalo,ixtal,ibin) if ibin!=0 else "calo %s xtal %s Inclusive" %(icalo,ixtal)
            # unscaled
            #if ibin == 0: unscaled(directory, mcdirectory, calopath , icalo, ixtal, calibrator)
            #if ibin !=0 : exit()
            #continue
            # scaled
            ratioplot['energy%s' %ibin] = compare(
                "%s/calo-%s/data-run4F_calo%s_xtal_%s.root" %(directory,icalo,icalo,ixtal)
                , "%s/calo-%s/mc-gasgun_calo%s_xtal_%s.root" %(mcdirectory,icalo,icalo,ixtal)
                , calopath
                , name.replace(" ","-")
                , name
                , "energy%s" %ibin
                , [ calibrator["range1"][ixtal], calibrator["range2"][ixtal] ]
                , snorm=1
                , ratio=1
                , poisson=False
                , logy=True
                , rw=False
                , printScale="%s" %float(calibrator["scale"][ixtal])
                , saveOnly0Bin=True
            )
        ## ratio
        drawMultiRatio(ratioplot,calopath,icalo,ixtal)

    ## make html
    # crystal spectrum
    os.system( "cp %s/html_template/display.html %s/calo-%s_xtal_spectrum.html" %(cwd, outpath, icalo) )
    os.system( "sed -i -e 's/__XXXX__/calo-%s/g' %s/calo-%s_xtal_spectrum.html" %(icalo ,outpath, icalo) )
    os.system( "sed -i -e 's/__XXX__/%s/g' %s/calo-%s_xtal_spectrum.html" %(icalo ,outpath, icalo) )

    # ratio plot for phase bin.
    os.system( "cp %s/html_template/display1.html %s/calo-%s_phase_bin_ratio.html" %(cwd, outpath, icalo) )
    os.system( "sed -i -e 's/__XXXX__/calo-%s/g' %s/calo-%s_phase_bin_ratio.html" %(icalo ,outpath, icalo) )
    os.system( "sed -i -e 's/__XXX__/%s/g' %s/calo-%s_phase_bin_ratio.html" %(icalo ,outpath, icalo) )

    # calibrator ks distance.
    os.system( "cp %s/html_template/display2.html %s/calo-%s_ks-distance.html" %(cwd, outpath, icalo) )
    os.system( "sed -i -e 's/__XXXX__/calo-%s/g' %s/calo-%s_ks-distance.html" %(icalo ,outpath, icalo) )
    os.system( "sed -i -e 's/__XXX__/%s/g' %s/calo-%s_ks-distance.html" %(icalo ,outpath, icalo) )
    
    # index
    os.system( "cp %s/html_template/index.html %s/index.html" %(cwd, outpath) )

# make global scale
os.system( "scp %s/make-global-scale-list.sh %s" %(cwd, calibdirectory) )
os.system( "cd %s; ./make-global-scale-list.sh; cd %s" %(calibdirectory, cwd) )
os.system( "python plot-xtal-scale.py %s/global-ks-scale.dat %s/global-energy-scale.png" %(calibdirectory,outpath) )
