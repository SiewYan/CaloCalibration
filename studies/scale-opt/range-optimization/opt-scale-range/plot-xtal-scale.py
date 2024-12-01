from ROOT import TH2D, TCanvas, gROOT, gStyle, TLine
import ROOT
from array import array as arr
import sys

gROOT.SetBatch(True)
gStyle.SetOptStat(0)
gStyle.SetPaintTextFormat("4.2f")
gStyle.SetHistMinimumZero()
gStyle.SetNumberContours(256)
gStyle.SetPalette(ROOT.kRainbow)
#gStyle.SetPalette(ROOT.kVisibleSpectrum)
gStyle.SetPadRightMargin(0.2)
gStyle.SetPadBottomMargin(0.13)
gStyle.SetPadLeftMargin(0.1)


def percalo_scale(filename_):

    file1 = open( filename_ , 'r' )
    Lines = file1.readlines()
    isRW = True if 'rw' in filename_ else False

    COI=filename_.split('_')[1].strip('calo')

    dd={}
    cc={}
    recon="Recon West" if isRW else "Recon East"
    for icalo in range(1,25):
        if icalo != int(COI): continue
        dd[icalo] = TH2D(
            "%s" %icalo ,
            'Calo; Xtal; Ytal',
            9, 0., 9.,
            6, 0., 6
        )
    
        cc[icalo] = TCanvas( "%s" %icalo , 'scale' , 1200 , 800 )
    
        for line in Lines:
            line = line.strip().split(' ')
            calo=str(line[0])
            if str(icalo) != calo : continue
            row=int(line[1]) // 9
            col=int(line[1]) % 9
            scale=float(line[4])
        
            #print("X :", 9-(col+1), " ; Y : ", (row) , " ; content : ", scale ) 
            dd[icalo].SetBinContent( 10-(col+1) , (row+1) , scale )
            dd[icalo].Fill( str(9-(col+1)) , str(row) , 0 )

        # Cross check
        #dd[icalo].SetBinContent( 10-(col+1) , (row+1) , int(line[1])+1 )
        #dd[icalo].Fill( str(9-(col+1)) , str(row) , 0 )
        
        cc[icalo].cd()
        cc[icalo].SetGrid()
    
        #h2d.GetZaxis().SetNdivisions(20);
        #h2d.GetXaxis().SetNdivisions(53);
        dd[icalo].GetXaxis().SetTickLength(0.02)
        dd[icalo].GetYaxis().SetTickLength(0.01)
        dd[icalo].SetMaximum(1.2)
        dd[icalo].SetMinimum(0.5)
    
        dd[icalo].GetZaxis().SetTitle("Energy Scale")
        dd[icalo].SetNdivisions(510, "X")
        if 'mean' in filename_:
            dd[icalo].SetTitle( "(%s) Calorimeter %s (mean scale)" %(recon,icalo) )
        else:
            dd[icalo].SetTitle( "(%s) Calorimeter %s" %(recon,icalo) )
        #dd[icalo].SetTitle( "Xtal index" )
    
        dd[icalo].Draw("Colz TEXT45")
        #dd[icalo].Draw("TEXT45")

        cc[icalo].Print( filename.replace('.txt','.png') )
    #del c

pass

def global_scale(filename_):

    file1 = open( filename_ , 'r' )
    Lines = file1.readlines()
    isRW = True if 'rw' in filename_ else False

    recon="Recon West" if isRW else "Recon East"

    h2d = TH2D(
        "h2d" ,
        'Calo; Xtal; Ytal',
        9, 0., 9.,
        6, 0., 6
    )

    c2d = TCanvas( "c2d" , '%s global scale' %recon , 1200 , 800 )
    
    for line in Lines:
        line = line.strip().split(' ')
        row=int(line[0]) // 9
        col=int(line[0]) % 9
        scale=float(line[3])
        
        #print("X :", 9-(col+1), " ; Y : ", (row) , " ; content : ", scale ) 
        h2d.SetBinContent( 10-(col+1) , (row+1) , scale )
        h2d.Fill( str(9-(col+1)) , str(row) , 0 )

        # Cross check
        #dd[icalo].SetBinContent( 10-(col+1) , (row+1) , int(line[1])+1 )
        #dd[icalo].Fill( str(9-(col+1)) , str(row) , 0 )
        
    c2d.cd()
    c2d.SetGrid()
    
    #h2d.GetZaxis().SetNdivisions(20);
    #h2d.GetXaxis().SetNdivisions(53);
    h2d.GetXaxis().SetTickLength(0.02)
    h2d.GetYaxis().SetTickLength(0.01)
    h2d.SetMaximum(1.5)
    h2d.SetMinimum(0.5)
    
    #h2d.GetZaxis().SetTitle("Energy Scale")
    h2d.SetNdivisions(510, "X")
    #h2d.SetTitle( "(%s) Global Scale" %recon )

    #h2d.SetMarkerSize(1.8)
    #h2d.SetMarkerColor(ROOT.kRed)
    h2d.SetMinimum(0.0)
    h2d.Draw("Colz TEXT45")
 
    c2d.Print( filename_.replace('.txt','.png') )
    #del c

pass

if __name__ == '__main__':

    #filename='/home/shoh/Works/gm2/study/result-calibrator/calibrator_v1/global-ks-scale.dat'
    filename = sys.argv[1]

    if 'global' in filename:
        print('global detected')
        global_scale(filename)
    else:
        print('per calo')
        percalo_scale(filename)
