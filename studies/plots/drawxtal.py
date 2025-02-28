from array import array
import math
from ROOT import gPad, TCanvas, TH2D, TH1D, TFile, THStack, TLegend, gSystem, TLatex, TH1, gStyle, TGaxis, kOrange, kBlue, kRed
import os, sys

#import CMS_lumi

#import tdrstyle
#TGaxis.SetMaxDigits(2)
TH1.SetDefaultSumw2()
gStyle.SetPaintTextFormat(".5f")

label = {
    "wiggle"         : "wiggle",
    "wiggle_3prd"    : "wiggle n_{period} = 3",
    "wiggle_ph"      : "wiggle phase",
    "wiggle_ph_mod"  : "wiggle modulus",
    "energy0d"       : "E(#phi=0 deg)",
    "energy90d"      : "E(#phi=90 deg)",
    "energy180d"     : "E(#phi=180 deg)",
    "energy0"        : "Inclusive",
    "energy1"        : "Phase Bin 1",
    "energy2"        : "Phase Bin 2",
    "energy3"        : "Phase Bin 3",
    "energy4"        : "Phase Bin 4",
    "energy5"        : "Phase Bin 5",
    "energy6"        : "Phase Bin 6",
    "energy7"        : "Phase Bin 7",
    "energy8"        : "Phase Bin 8",
    "energy9"        : "Phase Bin 9",
    "energy10"       : "Phase Bin 10"
}

def setBotStyle(h, r=4, fixRange=True):
    h.GetXaxis().SetLabelSize(h.GetXaxis().GetLabelSize()*(r-1));
    h.GetXaxis().SetLabelOffset(h.GetXaxis().GetLabelOffset()*(r-1));
    h.GetXaxis().SetTitleSize(h.GetXaxis().GetTitleSize()*(r-1));
    
    h.GetYaxis().SetLabelSize(h.GetYaxis().GetLabelSize()*(r-1.5));
    h.GetYaxis().SetNdivisions(505)
    h.GetYaxis().SetTitleSize(h.GetYaxis().GetTitleSize()*(r-2.5));
    #h.GetYaxis().SetTitleOffset(h.GetYaxis().GetTitleOffset()/(r-3));
    h.GetYaxis().SetTitleOffset(h.GetYaxis().GetTitleOffset()*1.075);
    h.GetYaxis().SetTitle("Data / MC")
    if fixRange:
        h.GetYaxis().SetRangeUser(0.3, 1.7)
        for i in range(1, h.GetNbinsX()+1):
            if h.GetBinContent(i)<1.e-6:
                h.SetBinContent(i, -1.e-6)
pass

def drawCMS(lumi, text, onTop=False):
    latex = TLatex()
    latex.SetNDC()
    latex.SetTextSize(0.04)
    latex.SetTextColor(1)
    latex.SetTextFont(42)
    latex.SetTextAlign(33)
    latex.DrawLatex(0.95, 0.985, "%s  (Partial)" %lumi)
    #if (type(lumi) is float or type(lumi) is int) and float(lumi) > 0: latex.DrawLatex(0.95, 0.985, "%.1f fb^{-1}  (13 TeV)" % (float(lumi)/1000.))
    #elif type(lumi) is str: latex.DrawLatex(0.95, 0.985, "%s fb^{-1}  (13 TeV)" % lumi)
    if not onTop: latex.SetTextAlign(11)
    latex.SetTextFont(62)
    latex.SetTextSize(0.05 if len(text)>0 else 0.06)
    if not onTop: latex.DrawLatex(0.15, 0.87 if len(text)>0 else 0.84, "Muon g-2")
    else: latex.DrawLatex(0.20, 0.99, "g-2")
    latex.SetTextSize(0.04)
    latex.SetTextFont(52)
    if not onTop: latex.DrawLatex(0.15, 0.83, text)
    else: latex.DrawLatex(0.40, 0.98, text)
pass

def setHistStyle(hist, r=1.1):
    hist.GetXaxis().SetTitleSize(hist.GetXaxis().GetTitleSize()*r*r)
    hist.GetYaxis().SetTitleSize(hist.GetYaxis().GetTitleSize()*r*r)
    hist.GetXaxis().SetLabelSize(hist.GetXaxis().GetLabelSize()*r)
    hist.GetYaxis().SetLabelSize(hist.GetYaxis().GetLabelSize()*r)
    hist.GetXaxis().SetLabelOffset(hist.GetXaxis().GetLabelOffset()*r*r*r*r)
    hist.GetXaxis().SetTitleOffset(hist.GetXaxis().GetTitleOffset()*r)
    hist.GetYaxis().SetTitleOffset(hist.GetYaxis().GetTitleOffset())
    #if hist.GetXaxis().GetTitle().find("MeV") != -1: # and not hist.GetXaxis().IsVariableBinSize()
    div = (hist.GetXaxis().GetXmax() - hist.GetXaxis().GetXmin()) / hist.GetXaxis().GetNbins()
    hist.GetYaxis().SetTitle("Events/%d MeV" % int(div))
    hist.SetTitle("")
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
    #print("  Ratio:\t%.3f +- %.3f" % (ratio, error) )
    return [ratio, error]
pass

def drawKolmogorov(data, bkg):
    latex = TLatex()
    latex.SetNDC()
    latex.SetTextColor(1)
    latex.SetTextFont(62)
    latex.SetTextSize(0.08)
    #latex.DrawLatex(0.45, 0.85, "#chi^{2}/ndf = %.2f,   K-S = %.3f" % (data.Chi2Test(bkg, "CHI2/NDF"), data.KolmogorovTest(bkg)))
    latex.DrawLatex(0.45, 0.85, "#chi^{2}/ndf = %.2f" % (data.Chi2Test(bkg, "CHI2/NDF")))
    #print(data.Chi2Test(bkg, "P"))
    print()
pass

def drawRelativeYield(data,bkg):
    latex = TLatex()
    latex.SetNDC()
    latex.SetTextColor(1)
    latex.SetTextFont(62)
    latex.SetTextSize(0.08)
    latex.DrawLatex(0.75, 0.85, "rel. Yield= %.3f" % ((data.Integral()/bkg.Integral())*100) )
pass

def setTopPad(TopPad, r=4):
    TopPad.SetPad("TopPad", "", 0., 1./r, 1.0, 1.0, 0, -1, 0)
    TopPad.SetTopMargin(0.24/r)
    TopPad.SetBottomMargin(0.04/r)
    TopPad.SetRightMargin(0.05)
    TopPad.SetTicks(1, 1)
pass

def setBotPad(BotPad, r=4):
    BotPad.SetPad("BotPad", "", 0., 0., 1.0, 1./r, 0, -1, 0)
    BotPad.SetTopMargin(r/100.)
    BotPad.SetBottomMargin(r/10.)
    BotPad.SetRightMargin(0.05)
    BotPad.SetTicks(1, 1)
pass

def addOverflow(hist, addUnder=True):
    n = hist.GetNbinsX()
    hist.SetBinContent(n, hist.GetBinContent(n) + hist.GetBinContent(n+1))
    hist.SetBinError(n, math.sqrt( hist.GetBinError(n)**2 + hist.GetBinError(n+1)**2 ) )
    hist.SetBinContent(n+1, 0.)
    hist.SetBinError(n+1, 0.)
    if addUnder:
        hist.SetBinContent(1, hist.GetBinContent(0) + hist.GetBinContent(1))
        hist.SetBinError(1, math.sqrt( hist.GetBinError(0)**2 + hist.GetBinError(1)**2 ) )
        hist.SetBinContent(0, 0.)
        hist.SetBinError(0, 0.)
pass

def drawRegion(channel, left=False):

    latex = TLatex()
    latex.SetNDC()
    latex.SetTextFont(72) #52
    latex.SetTextSize(0.035)
    if left: latex.DrawLatex(0.15, 0.75, channel)
    else:
        latex.SetTextAlign(22)
        latex.DrawLatex(0.5, 0.85, channel)
    pass


def print4Scale(channel, left=False):

    latex = TLatex()
    latex.SetNDC()
    latex.SetTextFont(61) #52
    latex.SetTextSize(0.1)
    if left: latex.DrawLatex(0.18, 0.75, channel)
    else:
        latex.SetTextAlign(22)
        latex.SetTextColor(kRed)
        latex.DrawLatex(0.8, 0.5, channel)
    pass

def compare( fdata , fmc , output , filename , region , var , nrange , snorm=1, ratio=1, poisson=True, logy=False, rw=True, printScale=None, saveOnly0Bin=False ):
    HIST={}
    #OUT={}
    
    ffdata = TFile(fdata,"READ")
    ffmc = TFile(fmc,"READ")

    HIST['MC']   = ffmc.Get(var) 
    HIST['DATA'] = ffdata.Get(var)

    #HIST['MC'].SetDirectory(0)
    #HIST['DATA'].SetDirectory(0)
    
    HIST['BkgSum'] = HIST['DATA'].Clone("BkgSum")
    HIST['BkgSum'].Reset("MICES")
    HIST['BkgSum'].SetFillStyle(3003)
    HIST['BkgSum'].SetFillColor(1)
    HIST['BkgSum'].SetMarkerStyle(0)

    HIST['BkgSum'].Add(HIST['MC'])

    HIST['DATA'].SetMarkerStyle(20)
    HIST['DATA'].SetMarkerSize(1.25)
    HIST['DATA'].SetFillColor(1)
    HIST['DATA'].SetFillStyle(3003)
    HIST['DATA'].SetLineColor(1)
    HIST['DATA'].SetLineStyle(1)
    HIST['DATA'].SetLineWidth(2)

    HIST['MC'].SetFillColor(418) if rw else HIST['MC'].SetFillColor(881)
    HIST['MC'].SetFillStyle(1001)
    HIST['MC'].SetLineColor(418) if rw else HIST['MC'].SetLineColor(881)
    HIST['MC'].SetLineStyle(1)
    HIST['MC'].SetLineWidth(2)

    for i, s in enumerate(HIST): addOverflow(HIST[s], False) # Add overflow

    #Stack
    bkg = THStack('bkg', ";"+HIST['BkgSum'].GetXaxis().GetTitle()+";"+HIST['BkgSum'].GetYaxis().GetTitle())
    bkg.Add(HIST['MC']) # ADD ALL BKG
    
    c1 = TCanvas("c1", list(HIST.values())[-1].GetXaxis().GetTitle(), 800, 800 if ratio else 600 )

    #Ratio pad
    if ratio:
        c1.Divide(1, 2)
        setTopPad(c1.GetPad(1))
        setBotPad(c1.GetPad(2))

    c1.cd(1)
    c1.GetPad(bool(ratio)).SetTopMargin(0.06)
    c1.GetPad(bool(ratio)).SetRightMargin(0.05)
    c1.GetPad(bool(ratio)).SetTicks(1, 1)
    if logy: c1.GetPad(bool(ratio)).SetLogy()

    ######################################################
    ######################################################
    #Draw
    #HIST['MC'].Draw("HIST") # stack
    #HIST['BkgSum'].Draw("SAME, E2") # sum of bkg
    #HIST['DATA'].Draw("SAME, PE") # data
    ## break down to plot in loop
    mcplot     = HIST['MC'].Clone("mcplot")
    bkgsumplot = HIST['BkgSum'].Clone("bkgsumplot")
    dataplot   = HIST['DATA'].Clone("dataplot")
    
    mcplot.Reset("MICES")
    bkgsumplot.Reset("MICES")
    dataplot.Reset("MICES")    

    xlow = nrange[0] if nrange is not None else 0.
    xhigh = nrange[1] if nrange is not None else 3100.
    
    #print("xlow  : ", xlow  )
    #print("xhigh : ", xhigh )
    
    for i in range(0, mcplot.GetNbinsX()+1):
        #if i >= mcplot.GetXaxis().FindFixBin(xlow) and i< mcplot.GetXaxis().FindFixBin(xhigh):
        mcplot.SetBinContent(i,HIST['MC'].GetBinContent(i))
        mcplot.SetBinError(i,HIST['MC'].GetBinError(i))
    for i in range(0, bkgsumplot.GetNbinsX()+1):
        #if i >= bkgsumplot.GetXaxis().FindFixBin(xlow) and i< bkgsumplot.GetXaxis().FindFixBin(xhigh):
        bkgsumplot.SetBinContent(i,HIST['BkgSum'].GetBinContent(i))
        bkgsumplot.SetBinError(i,HIST['BkgSum'].GetBinError(i))
    for i in range(0, dataplot.GetNbinsX()+1):
        #if i >= dataplot.GetXaxis().FindFixBin(xlow) and i< dataplot.GetXaxis().FindFixBin(xhigh):
        dataplot.SetBinContent(i,HIST['DATA'].GetBinContent(i))
        dataplot.SetBinError(i,HIST['DATA'].GetBinError(i))
    mcplot.Draw("HIST")
    bkgsumplot.Draw("SAME, E2")
    dataplot.Draw("SAME, PE")

    # normalize in range
    if snorm:
        if nrange is not None:
            data_scale = dataplot.Integral( dataplot.GetXaxis().FindFixBin(xlow), dataplot.GetXaxis().FindFixBin(xhigh))
            mc_scale = mcplot.Integral(mcplot.GetXaxis().FindFixBin(xlow), mcplot.GetXaxis().FindFixBin(xhigh))
        else:
            data_scale = dataplot.Integral();
            mc_scale = mcplot.Integral();

        print("Integrate from ", xlow  , " MeV to ", xhigh , " MeV" )
        print("Scale MC template with normalization factor : ", data_scale/mc_scale )

        mcplot.Scale( data_scale/mc_scale )
        bkgsumplot.Scale( data_scale/mc_scale )

    setHistStyle(mcplot)
    setHistStyle(bkgsumplot)

    mcplot.GetYaxis().SetTitleOffset(mcplot.GetYaxis().GetTitleOffset()*1.075) #1.075
    mcplot.SetMaximum((100.0 if logy else 1.5)*max(bkg.GetMaximum(), dataplot.GetBinContent(dataplot.GetMaximumBin())+dataplot.GetBinError(dataplot.GetMaximumBin())))
    #HIST['MC'].SetMinimum(max(min(HIST['BkgSum'].GetBinContent(HIST['BkgSum'].GetMinimumBin()), HIST['DATA'].GetMinimum()), 1.)  if logy else 0.) 
    mcplot.SetMinimum(1.)

    ######################################################
    ######################################################
    #Legend
    n=len(HIST)
    leg = TLegend(0.7, 0.9-0.05*n, 0.95, 0.9)
    leg.SetBorderSize(0)
    leg.SetFillStyle(0) #1001                                                                                                                                                               
    leg.SetFillColor(0)
    leg.SetTextSize(0.03)
    leg.AddEntry(dataplot, 'Data [%.1f]' %(dataplot.Integral()), "pl")
    leg.AddEntry(mcplot, 'MC [%.1f]' %(mcplot.Integral()), "f")
    #leg.AddEntry(bkgsumplot, 'BkgSum [%.1f]' %(bkgsumplot.Integral()), "f")

    leg.Draw()

    ##########################
    #rMC = HIST['BkgSum'].Clone("MC_hist;")
    #rData = HIST['DATA'].Clone("Data_hist;")
    rMC = bkgsumplot.Clone("MC_hist;")
    rData = dataplot.Clone("Data_hist;")
    rMC.Reset("MICES")
    rData.Reset("MICES")
    # probe in range
    for i in range(1, bkgsumplot.GetNbinsX()+1):
        if i >= bkgsumplot.GetXaxis().FindFixBin(xlow) and i< bkgsumplot.GetXaxis().FindFixBin(xhigh):
            rMC.SetBinContent(i,bkgsumplot.GetBinContent(i))
            rMC.SetBinError(i,bkgsumplot.GetBinError(i))
    for i in range(1, dataplot.GetNbinsX()+1):
        if i >= dataplot.GetXaxis().FindFixBin(xlow) and i< dataplot.GetXaxis().FindFixBin(xhigh):
            rData.SetBinContent(i,dataplot.GetBinContent(i))
            rData.SetBinError(i,dataplot.GetBinError(i))
    ##########################
    if ratio:
        c1.cd(2)
        err = bkgsumplot.Clone("BkgErr;")
        err1 = bkgsumplot.Clone("BkgErr;")
        err2 = bkgsumplot.Clone("BkgErr;")
        err.SetTitle("")
        for i in range(1, err.GetNbinsX()+1):
            err.SetBinContent(i, 1)
            err1.SetBinContent(i,1.34)
            err2.SetBinContent(i,0.66)
            if bkgsumplot.GetBinContent(i) > 0:
                err.SetBinError(i, bkgsumplot.GetBinError(i)/bkgsumplot.GetBinContent(i))
        err1.SetBinContent(err.GetNbinsX(),1.34)
        err2.SetBinContent(err2.GetNbinsX(),0.66)
        setBotStyle(err)
        errLine = err.Clone("errLine")
        errLine.SetLineWidth(1)
        errLine.SetFillStyle(0)
        errLine.SetLineColor(1)
        err.Draw("E2")
        errLine.Draw("SAME, HIST")

        ## one sigma band
        dashLine1 = err1.Clone("Line1")
        dashLine1.SetLineWidth(1)
        dashLine1.SetFillStyle(3003)
        dashLine1.SetFillColor(418) if rw else dashLine1.SetFillColor(881)
        dashLine1.SetLineColor(418) if rw else dashLine1.SetLineColor(881)
        dashLine1.SetLineStyle(8)
        dashLine1.Draw("SAME, HIST")
        
        dashLine2 = err2.Clone("Line2")
        dashLine2.SetLineWidth(1)
        #dashLine2.SetFillStyle(1001)
        dashLine2.SetFillColor(10)
        dashLine2.SetLineColor(42)
        dashLine2.SetLineStyle(8)
        dashLine2.Draw("SAME, HIST")

        res = dataplot.Clone("Residues")
        res.SetTitle("")
        for i in range(0, res.GetNbinsX()+1):
            if bkgsumplot.GetBinContent(i) > 0:
                # draw point in range
                if i >= res.GetXaxis().FindFixBin(xlow) and i< res.GetXaxis().FindFixBin(xhigh):
                    res.SetBinContent(i, res.GetBinContent(i)/bkgsumplot.GetBinContent(i))
                    res.SetBinError(i, res.GetBinError(i)/bkgsumplot.GetBinContent(i))
        #if outputRatio: res.SetDirectory(0)
        res.SetDirectory(0)
        setBotStyle(res)
        res.Draw("SAME, PE0")
        #print("whole range : ", HIST['DATA'].Chi2Test(HIST['BkgSum'], "CHI2/NDF"))
        #print("subrange : ", rData.Chi2Test(rMC, "CHI2/NDF"))
        drawRatio(rData, rMC)
        drawKolmogorov(rData, rMC)
        #drawRelativeYield(rData,rMC)

    c1.cd(1)
    
    drawCMS("Run4F", "Preliminary")
    drawRegion(region)
    if printScale is not None: print4Scale(printScale)

    c1.Update()

    if saveOnly0Bin and var == "energy0":
        c1.Print( '%s/%s_%s.png' %( output , filename , var ) )
            
    return res
pass


def drawMultiRatio(histIn,outpath,calo,xtal):
    var=0
    c1 = TCanvas("c1", list(histIn.values())[-1].GetXaxis().GetTitle(), 800, 800 )

    #### ////
    n=len(histIn)
    # X -> ; Y up ; X <- ; Y down
    leg = TLegend(0.4, 0.8-0.005*n, 0.9, 0.87)
    leg.SetNColumns(3)
    leg.SetBorderSize(0)
    leg.SetFillStyle(0) #1001                                                                                                                                                                  leg.SetFillColor(0)
    leg.SetTextSize(0.02)

    for ikey,ivalue in histIn.items():

        #print("ikey : ", ikey, " ; ivalue : ", ivalue)
        ivalue.SetMarkerStyle(20)
        #ivalue.SetMarkerSize(1.25)
        ivalue.SetMarkerSize(1.1)
        ivalue.SetMarkerColor(kBlue-var)
        ivalue.SetFillColor(418)
        ivalue.SetFillStyle(1001)
        ivalue.SetLineColor(kBlue-var)
        ivalue.SetLineStyle(1)
        ivalue.SetLineWidth(2)

        leg.AddEntry(ivalue, "inclusive" if var==0 else "phase bin %s" %var, "pl")
        if var==0:
            ivalue.Draw("PE0")

            ivalue.SetMarkerColor(kRed)
            ivalue.SetLineColor(kRed)
            
            ivalue.GetXaxis().SetTitleSize(0.03)
            ivalue.GetYaxis().SetTitleSize(0.03)
            
            ivalue.GetXaxis().SetLabelSize(0.03)
            ivalue.GetYaxis().SetLabelSize(0.03)

            ivalue.GetXaxis().SetTitleOffset(1.4)
            ivalue.GetYaxis().SetTitleOffset(1.4)

            ivalue.SetTitle("Calo %s Xtal %s" %(calo,xtal))
            ivalue.GetXaxis().SetTitle("Crystal Energy [MeV]")
            ivalue.GetYaxis().SetTitle("Data/MC")
            ivalue.GetYaxis().SetMaxDigits(3);
            #ivalue.GetYaxis().SetNdivisions(-414)
            ivalue.GetYaxis().SetNdivisions(414)
            ivalue.GetYaxis().SetRangeUser(0.3, 1.7)
            #ivalue.GetYaxis().SetRangeUser(0.75, 1.35)
        else:
            ivalue.Draw("SAME,PE0")
        #setHistStyle(ivalue)
        var+=1
        
    leg.Draw()
    c1.SetGridy()
    c1.Print('%s/multiRatio_calo-%s_xtal-%s.png' %(outpath,calo,xtal))

    
