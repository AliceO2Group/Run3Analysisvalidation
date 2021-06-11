#!/usr/bin/env python
from ROOT import TCanvas, TFile, gPad, gStyle

gStyle.SetOptStat(0)
gStyle.SetErrorX(0)
gStyle.SetFrameLineWidth(1)
gStyle.SetTitleSize(0.045, "x")
gStyle.SetTitleSize(0.045, "y")
gStyle.SetMarkerSize(1)
gStyle.SetLabelOffset(0.015, "x")
gStyle.SetLabelOffset(0.02, "y")
gStyle.SetTickLength(-0.02, "x")
gStyle.SetTickLength(-0.02, "y")
gStyle.SetTitleOffset(1.1, "x")
gStyle.SetTitleOffset(1.0, "y")


def saveCanvas(canvas, title):
    format_list = ["png", ".pdf", ".root"]
    for fileFormat in format_list:
        canvas.SaveAs(title + fileFormat)


def kinematic_plots(var1):
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
    cres = TCanvas("cres", "resolution distribution")
    cres.SetCanvasSize(1600, 1200)
    cres.Divide(1, 3)
    sig = fileo2.Get("hf-task-jpsi-mc/h%sSig" % var1)
    bkg = fileo2.Get("hf-task-jpsi-mc/h%sBg" % var1)
    gen = fileo2.Get("hf-task-jpsi-mc/h%sGen" % var1)
    cres.cd(1)
    gPad.SetLogz()
    sig.Draw("coltz")
    sig.SetTitle("%s Signal distribution(Rec. Level)" % var1)
    cres.cd(2)
    gPad.SetLogz()
    bkg.Draw("coltz")
    bkg.SetTitle("%s Background distribution(Rec. Level)" % var1)
    cres.cd(3)
    gPad.SetLogz()
    gen.Draw("coltz")
    gen.SetTitle("%s Gen distribution(Gen. Level)" % var1)
    saveCanvas(cres, "%s" % var1)
    ceff = TCanvas("ceff", "2D efficiencies")
    ceff.SetCanvasSize(1600, 1200)
    ceff.cd()
    eff = sig.Clone("eff")
    den = gen.Clone("den")
    eff.Divide(den)
    eff.Draw("COLZ")
    eff.SetTitle("%s Jpsi reco and sel. efficiency (Reco pt / Gen pt)" % var1)
    saveCanvas(ceff, "efficiencyYpt")


var_list = ["Y"]

for var in var_list:
    kinematic_plots(var)
