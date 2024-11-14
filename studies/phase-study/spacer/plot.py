import ROOT
from ROOT import gROOT , gStyle, TLegend, TCanvas, gPad, gSystem, TLatex, gStyle, TGaxis, TH1
import os, sys
from array import array
import math

TH1.SetDefaultSumw2()
gROOT.SetBatch(True)
gStyle.SetOptStat(0)
gStyle.SetPaintTextFormat(".5f")

def setBotStyle(h, r=4, fixRange=True):
    h.GetXaxis().SetLabelSize(h.GetXaxis().GetLabelSize()*(r-1));
    h.GetXaxis().SetLabelOffset(h.GetXaxis().GetLabelOffset()*(r-1));
    h.GetXaxis().SetTitleSize(h.GetXaxis().GetTitleSize()*(r-1));
    h.GetYaxis().SetLabelSize(h.GetYaxis().GetLabelSize()*(r-1));
    h.GetYaxis().SetNdivisions(505);
    h.GetYaxis().SetTitleSize(h.GetYaxis().GetTitleSize()*(r-1));
    h.GetYaxis().SetTitleOffset(h.GetYaxis().GetTitleOffset()/(r-1));
    if fixRange:
        #h.GetYaxis().SetRangeUser(0., 2.)
        h.GetYaxis().SetRangeUser(0.3, 1.7)
        for i in range(1, h.GetNbinsX()+1):
            if h.GetBinContent(i)<1.e-6:
                h.SetBinContent(i, -1.e-6)
pass

def drawRatio(data, bkg):
    errData = array('d', [1.0])
    errBkg = array('d', [1.0])
    intData = data.IntegralAndError(1, data.GetNbinsX(), errData)
    intBkg = bkg.IntegralAndError(1, bkg.GetNbinsX(), errBkg)
    ratio = intData / intBkg if intBkg!=0 else 0.
    error = math.hypot(errData[0]*ratio/intData,  errBkg[0]*ratio/intBkg) if intData>0 and intBkg>0 else 0
    latex = TLatex()
    latex.SetNDC()
    latex.SetTextColor(1)
    latex.SetTextFont(62)
    latex.SetTextSize(0.08)
    #latex.DrawLatex(0.25, 0.85, "Data/Bkg = %.3f #pm %.3f" % (ratio, error))
    latex.DrawLatex(0.15, 0.85, "Data/Bkg = %.3f #pm %.3f" % (ratio, error))
    print ("  Ratio:\t%.3f +- %.3f" % (ratio, error))
    return [ratio, error]
pass

def en_angle_compare(file_,outfile_):
    
    f = ROOT.TFile.Open( file_ , "READ" )
    
    en_0d = f.Get("energy0d")
    en_90d = f.Get("energy90d")
    en_180d = f.Get("energy180d")
    
    en_0d.SetDirectory(0)
    en_90d.SetDirectory(0)
    en_180d.SetDirectory(0)
    
    # blue : 0deg ; red : 90deg ; black : 180deg
    en_0d.SetLineColor(ROOT.kBlue)
    en_0d.SetLineWidth(2)
    
    en_90d.SetLineColor(ROOT.kRed)
    en_90d.SetLineWidth(2)
        
    en_180d.SetLineColor(ROOT.kBlack)
    en_180d.SetLineWidth(2)

    c1 = TCanvas("c1","c1",800, 800)

    # mc
    c1.cd()
    en_0d.Draw()
    en_0d.SetTitle("E(#theta)")
    en_90d.Draw("same")
    en_180d.Draw("same")

    en_0d.Scale(1./en_0d.Integral())
    en_90d.Scale(1./en_90d.Integral())
    en_180d.Scale(1./en_180d.Integral())    
    en_0d.SetMaximum( (en_0d.GetBinContent(en_0d.GetMaximumBin())+en_0d.GetBinError(en_0d.GetMaximumBin()))*1.5 )

    legend = ROOT.TLegend(0.65 ,0.65 ,0.84 ,0.77)
    #legend = ROOT.TLegend(0.7377477,0.7315783,0.8879041,0.8167505);                                                                                                                        
    legend.SetLineWidth(0)
    #legend = ROOT.TLegend(0.7 ,0.8 ,0.85 ,0.75)                                                                                                                                            
    legend.AddEntry( en_0d , "#theta = 0^{#circ}" )
    legend.AddEntry( en_90d , "#theta = 90^{#circ}" )
    legend.AddEntry( en_180d , "#theta = 180^{#circ}" )
    legend.SetTextFont(42)
    legend.Draw("same")
    
    c1.Print("%s.png" %outfile_)
    
pass

if __name__ == '__main__':

    en_angle_compare("data-reference_phase.root","data_reference_phase")
    en_angle_compare("data-spacer_phase.root","data_spacer_phase")
    #en_angle_compare("mc_phase.root","mc_phase")
    
