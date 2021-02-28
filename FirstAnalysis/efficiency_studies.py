#!/usr/bin/env python
from ROOT import TH1F, TCanvas, TEfficiency, TFile, TLegend, gPad


def saveCanvas(canvas, title):
    format_list = [".png", ".pdf", ".root"]
    for fileFormat in format_list:
        canvas.SaveAs(title + fileFormat)


def efficiencytracking(var):
    # plots the efficiency vs pT, eta and phi for all the species(it extracts the
    # Efficiency from qa - tracking - efficiency if you have ran with-- make - eff)
    hadron_list = [
        "pion",
        "proton",
        "kaon",
        "electron",
        "muon",
    ]
    color_list = [1, 2, 4, 6, 8]
    fileo2 = TFile("/data/Run5data/QA/CCbar_pp_AnalysisResults_O2_500files_HFval.root")

    c1 = TCanvas("c1", "Efficiency")
    c1.SetCanvasSize(1500, 1500)
    c1.cd()
    if var == "pt":
        gPad.SetLogx()
    eff_list = []
    if var == "pt":
        hempty = TH1F("hempty", ";p_{T};efficiency", 100, 0.0015, 15)
    elif var == "eta":
        hempty = TH1F("hempty", ";eta;efficiency", 100, -4.0, 4.0)
    elif var == "phi":
        hempty = TH1F("hempty", ";phi;efficiency", 100, 0.0, 6.0)
    hempty.Draw()
    leg = TLegend(0.1, 0.7, 0.3, 0.9, "")
    leg.SetFillColor(0)

    for i, had in enumerate(hadron_list):
        leff = fileo2.Get("qa-tracking-efficiency-%s/Efficiency" % had)
        if var == "pt":
            eff = leff.At(0)
        elif var == "eta":
            eff = leff.At(1)
        elif var == "phi":
            eff = leff.At(2)
        eff.SetLineColor(color_list[i])
        eff_list.append(eff)
        eff.Draw("same")
        leg.AddEntry(eff_list[i], had)
    leg.Draw()
    saveCanvas(c1, "efficiency_tracking_%s" % var)


def efficiencyhadron(had, var):
    # extract the efficiency vs pT for single species(D0, Lc, Jpsi)
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
    ceffhf = TCanvas("ceffhf", "A Simple Graph Example")
    ceffhf.SetCanvasSize(1500, 700)
    ceffhf.Divide(2, 1)
    gPad.SetLogy()
    # hnum = fileo2.Get("qa-tracking-efficiency-%s/%s/num" % (had, var))
    hnum = fileo2.Get("hf-task-%s-mc/h%sRecSig" % (had, var))
    # hden = fileo2.Get("qa-tracking-efficiency-%s/%s/den" % (had, var))
    hden = fileo2.Get("hf-task-%s-mc/h%sGen" % (had, var))
    hnum.Rebin(4)
    hden.Rebin(4)
    eff = TEfficiency(hnum, hden)
    eff.Draw()
    ceffhf.SaveAs("efficiency_hfcand%s%s.pdf" % (had, var))
    ceffhf.SaveAs("efficiency_hfcand%s%s.root" % (had, var))


var_list = ["pt", "eta", "phi"]
for var in var_list:
    efficiencytracking(var)
