from ROOT import TH2D, TCanvas, gROOT, gStyle, TLine
import ROOT
from array import array as arr
import sys

gROOT.SetBatch(True)
gStyle.SetOptStat(0)
gStyle.SetPaintTextFormat("4.2f")
#gStyle.SetPalette(ROOT.kArmy)
gStyle.SetPalette(ROOT.kRainbow)
gStyle.SetPadRightMargin(0.2)
gStyle.SetPadBottomMargin(0.13)
gStyle.SetPadLeftMargin(0.1)

#filename='/home/shoh/Works/gm2/study/result-calibrator/calibrator_v1/global-ks-scale.dat'
filename = sys.argv[1]
file1 = open( filename , 'r' )
Lines = file1.readlines()

ncalo = [ i for i in range(1, 25, 1 ) ]
nxtal = [ i for i in range(0, 54, 1 ) ]

h2d = TH2D(
    "h2d" ,
    'KS-optimized energy scale; Xtal number; Calo number',
    53, 0., 53.,
    24, 1, 24
)

count=0
for line in Lines:
    count+=1
    #line = line.strip().split(',')
    line = line.strip().split(' ')
    h2d.Fill( str(line[1]) , str(line[0]), float(line[2]))
        
#
c = TCanvas( 'c' , 'scale' , 1200 , 800 )

#h2d.GetZaxis().SetNdivisions(20);
#h2d.GetXaxis().SetNdivisions(53);
h2d.GetXaxis().SetTickLength(0.02)
h2d.GetYaxis().SetTickLength(0.01)
h2d.SetMaximum(1.5)
h2d.SetMinimum(0.5)

h2d.GetZaxis().SetTitle("Energy Scale")

#h2d.Draw("Colz TEXT45")
h2d.Draw("Colz")

line1=TLine(9,1,9,24)
line1.SetLineColor(ROOT.kRed)
line1.SetLineStyle(9)
line1.SetLineWidth(2)
line1.Draw()

line2=TLine(18,1,18,24)
line2.SetLineColor(ROOT.kRed)
line2.SetLineStyle(9)
line2.SetLineWidth(2)
line2.Draw()

line3=TLine(27,1,27,24)
line3.SetLineColor(ROOT.kRed)
line3.SetLineStyle(9)
line3.SetLineWidth(2)
line3.Draw()

line4=TLine(36,1,36,24)
line4.SetLineColor(ROOT.kRed)
line4.SetLineStyle(9)
line4.SetLineWidth(2)
line4.Draw()

line5=TLine(45,1,45,24)
line5.SetLineColor(ROOT.kRed)
line5.SetLineStyle(9)
line5.SetLineWidth(2)
line5.Draw()


name=  "%s-scale.png" %filename.split('/')[-2] if len(sys.argv) == 2 else sys.argv[2]
c.Print( "%s" %name )
    
