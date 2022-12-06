#!/usr/bin/env python
from ROOT import TH1F, TCanvas, TEfficiency, TFile, TLegend, gPad, gStyle


def saveCanvas(canvas, title):
    format_list = [".png", ".pdf", ".root"]
    for fileFormat in format_list:
        canvas.SaveAs(title + fileFormat)


def efficiencytracking(var):
    # plots the efficiency vs pT, eta and phi for all the species(it extracts the
    # Efficiency from qa - tracking - efficiency if you have ran with-- make - eff)
    hadron_list = ["pion", "proton", "kaon", "electron", "muon"]
    color_list = [1, 2, 4, 6, 8]
    marker_list = [20, 21, 22, 34, 45]
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")

    c1 = TCanvas("c1", "Efficiency")
    gStyle.SetOptStat(0)
    gStyle.SetErrorX(0)
    gStyle.SetFrameLineWidth(2)
    gStyle.SetTitleSize(0.045, "x")
    gStyle.SetTitleSize(0.045, "y")
    gStyle.SetMarkerSize(1)
    gStyle.SetLabelOffset(0.015, "x")
    gStyle.SetLabelOffset(0.02, "y")
    gStyle.SetTickLength(-0.02, "x")
    gStyle.SetTickLength(-0.02, "y")
    gStyle.SetTitleOffset(1.1, "x")
    gStyle.SetTitleOffset(1.0, "y")

    c1.SetCanvasSize(800, 600)
    c1.cd()
    c1.SetGridy()
    c1.SetGridx()
    eff_list = []

    if var == "Pt":
        hempty = TH1F("hempty", ";Transverse Momentum(GeV/c);Efficiency", 100, 0.05, 10)
        gPad.SetLogx()
    elif var == "Eta":
        hempty = TH1F("hempty", ";Pseudorapidity;Efficiency", 100, -4.0, 4.0)
    elif var == "Phi":
        hempty = TH1F("hempty", ";Azimuthal angle(rad);Efficiency", 100, 0.0, 6.0)

    hempty.GetYaxis().CenterTitle()
    hempty.GetXaxis().CenterTitle()
    hempty.GetXaxis().SetNoExponent()
    hempty.GetXaxis().SetMoreLogLabels(1)
    hempty.Draw()
    leg = TLegend(0.55, 0.15, 0.89, 0.35, "P")
    leg.SetNColumns(2)
    leg.SetHeader("Minimum bias KrKr #sqrt{s} = 6.46TeV", "C")
    leg.SetFillColor(0)

    for i, had in enumerate(hadron_list):
        leff = fileo2.Get("qa-tracking-efficiency-%s/Efficiency" % had)
        if var == "Pt":
            eff = leff.At(0)
        elif var == "Eta":
            eff = leff.At(1)
        elif var == "Phi":
            eff = leff.At(2)
        gPad.Update()
        eff.Paint("p")
        gr = eff.GetPaintedGraph().Clone()
        for j in range(0, gr.GetN()):
            gr.GetEXlow()[j] = 0
            gr.GetEXhigh()[j] = 0

        gr.SetLineColor(color_list[i])
        gr.SetMarkerColor(color_list[i])
        gr.SetMarkerStyle(marker_list[i])
        eff_list.append(gr)
        gr.Draw(" same p")
        leg.AddEntry(eff_list[i], had, "p")
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
    saveCanvas(ceffhf, "efficiency_hfcand%s%s" % (had, var))


var_list = ["Pt", "Eta", "Phi"]
hfhadron_list = ["d0", "dplus", "lc", "xic", "jpsi"]

for var in var_list:
    efficiencytracking(var)

for had in hfhadron_list:
    efficiencyhadron(had, "Pt")
