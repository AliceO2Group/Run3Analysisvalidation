#!/usr/bin/env python
from ROOT import TCanvas, TFile, gStyle, gROOT

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


def ratioparticle(
    var="pt",
    numname="Electron",
    selnum="RICHSelHpElLoose",
    denname="Pion",
    selden="RICHSelHpElLoose",
    label="e/#pi",
):
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
    cres = TCanvas("cres", "resolution distribution")
    cres.SetCanvasSize(1600, 1000)
    cres.cd()
    # cres.SetLogy()
    gStyle.SetOptStat(0)
    gROOT.SetBatch(1)

    num2d = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (numname, selnum, var))
    den2d = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (denname, selden, var))
    num = num2d.ProjectionX("num", 1, num2d.GetXaxis().GetNbins())
    den = den2d.ProjectionX("den", 1, den2d.GetXaxis().GetNbins())
    num.Divide(den)
    num.Draw("coltz")
    num.GetYaxis().SetTitle(label)
    num.GetXaxis().SetTitle("p_{T}")
    num.SetMinimum(0.001)
    num.SetMaximum(2.0)
    num.GetYaxis().SetTitleOffset(1.0)
    # num.GetZaxis().SetRangeUser(0.01, 1)
    # nameresult = "Fraction of %s selected by %s over %s selected by %s" % (num,selnum,den,selden)
    canvas = "Fractionof%s%sOver%s%s" % (numname, selnum, denname, selden)
    # num.SetTitle(nameresult)
    saveCanvas(cres, "rejection/%s" % (canvas))


# kinematic_plots("p", "pion", "MID", "Muon")
# kinematic_plots("p", "mu", "MID", "Muon")
# kinematic_plots("p", "pion", "TOF", "Electron")
# kinematic_plots("p", "pion", "RICH", "Electron")
# kinematic_plots("pt", "pion", "RICH", "Electron")
# kinematic_plots("p", "kaon", "RICH", "Electron")
# kinematic_plots("pt", "kaon", "RICH", "Electron")
# kinematic_plots("p", "pion", "TOF", "Kaon")
# kinematic_plots("p", "pion", "RICH", "Kaon")
ratioparticle(
    "pt", "Electron", "RICHSelHpElTight", "Electron", "NoSel", "e/e RICHSelHpElTight"
)
ratioparticle("pt", "Electron", "NoSel", "Pion", "NoSel", "e/#pi No cuts")
ratioparticle(
    "pt", "Electron", "RICHSelHpElTight", "Pion", "RICHSelHpElTight", "Tight e/#pi "
)
ratioparticle("pt", "Muon", "MID", "Pion", "MID", "MIDSel")
ratioparticle("pt", "Pion", "RICHSelHpElTight", "Pion", "NoSel", "Contamination")
ratioparticle("pt", "Pion", "MID", "Pion", "NoSel", "Contamination MID")