from array import array
import pandas as pd
import numpy as np
from ROOT import TH1F, TFile, TCanvas, TGraph, TLatex, gPad, TEfficiency
from ROOT import TLegend

def efficiencytracking():

    hadron_list = ["pion", "proton", "electron", "muon"]
    color_list = [1, 2, 4, 6]
    nhadrons = len(hadron_list)
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")

    c1 = TCanvas( 'c1', 'A Simple Graph Example')
    c1.SetCanvasSize(1500, 1500)
    c1.cd()
    gPad.SetLogx()
    gPad.SetLogy()
    eff_list = []
    hempty = TH1F("hempty", ";p_{T};efficiency", 100, 0.001, 5.)
    hempty.Draw()
    leg = TLegend(.1,.7,.3,.9,"");
    leg.SetFillColor(0);

    for i, had in enumerate(hadron_list):
        hnum = fileo2.Get("qa-tracking-efficiency-%s/num" % had)
        hden = fileo2.Get("qa-tracking-efficiency-%s/den" % had)
        hnum.Rebin(4)
        hden.Rebin(4)
        eff = TEfficiency(hnum, hden)
        eff.SetLineColor(color_list[i])
        eff_list.append(eff)
        eff.Draw("same")
        leg.AddEntry(eff_list[i], had);
    leg.Draw();
    c1.SaveAs("efficiency_tracking.pdf")

def efficiencyhadron(had, var):
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
    ceffhf = TCanvas( 'ceffhf', 'A Simple Graph Example')
    ceffhf.SetCanvasSize(1500, 700)
    ceffhf.Divide(2,1)
    gPad.SetLogy()
    hnum = fileo2.Get("hf-task-%s-mc/h%sRecSig" % (had, var))
    hden = fileo2.Get("hf-task-%s-mc/h%sGen" % (had, var))
    hnum.Rebin(4)
    hden.Rebin(4)
    eff = TEfficiency(hnum, hden)
    eff.Draw()
    ceffhf.SaveAs("efficiency_hfcand%s.pdf" % var)

efficiencytracking()
efficiencyhadron("lc", "Eta")
efficiencyhadron("lc", "Pt")

