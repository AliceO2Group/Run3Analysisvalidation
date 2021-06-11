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
    format_list = [".pdf"]
    for fileFormat in format_list:
        canvas.SaveAs(title + fileFormat)


def kinematic_plots(var, particle, detector, hp):
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
    cres = TCanvas("cres", "resolution distribution")
    cres.SetCanvasSize(1600, 1000)
    cres.cd()
    num = fileo2.Get(
        "qa-tracking-rejection-%s/tracking%ssel%s/%seta" % (particle, detector, hp, var)
    )
    den = fileo2.Get("qa-tracking-rejection-%s/tracking/%seta" % (particle, var))
    # gPad.SetLogz()
    num.Divide(den)
    num.Draw("coltz")
    num.GetYaxis().SetTitle("#eta")
    num.GetYaxis().SetTitleOffset(1.0)
    num.GetZaxis().SetRangeUser(0.01, 1)
    num.SetTitle("Fraction of %s selected by %s as %s" % (particle, detector, hp))
    saveCanvas(
        cres, "contamination/%seta_%sSelfrom%sas%s" % (var, particle, detector, hp)
    )


# kinematic_plots("p", "pion", "MID", "Muon")
# kinematic_plots("p", "mu", "MID", "Muon")
kinematic_plots("p", "pion", "TOF", "Electron")
kinematic_plots("p", "pion", "RICH", "Electron")
kinematic_plots("pt", "pion", "RICH", "Electron")
kinematic_plots("p", "kaon", "RICH", "Electron")
kinematic_plots("pt", "kaon", "RICH", "Electron")
# kinematic_plots("p", "pion", "TOF", "Kaon")
# kinematic_plots("p", "pion", "RICH", "Kaon")
