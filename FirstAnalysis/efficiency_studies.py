#!/usr/bin/env python
from ROOT import TH1F, TCanvas, TEfficiency, TFile, TLegend, gPad, gStyle, gROOT

import argparse

def saveCanvas(canvas, title):
    format_list = [".png", ".pdf", ".root"]
    for fileFormat in format_list:
        canvas.SaveAs(title + fileFormat)


def prepare_canvas(var, title):
    c1 = TCanvas(title, "Efficiency")
    c1.SetCanvasSize(800, 600)
    c1.cd()
    c1.SetGridy()
    c1.SetGridx()

    if var == "Pt":
        hempty = TH1F("hempty_%s" % title, ";Transverse Momentum(GeV/c);Efficiency", 100, 0.05, 10)
        gPad.SetLogx()
    elif var == "Eta":
        hempty = TH1F("hempty_%s" % title, ";Pseudorapidity;Efficiency", 100, -4.0, 4.0)
    elif var == "Phi":
        hempty = TH1F("hempty_%s" % title, ";Azimuthal angle(rad);Efficiency", 100, 0.0, 6.0)

    hempty.GetYaxis().CenterTitle()
    hempty.GetXaxis().CenterTitle()
    hempty.GetXaxis().SetNoExponent()
    hempty.GetXaxis().SetMoreLogLabels(1)
    hempty.Draw()
    leg = TLegend(0.55, 0.15, 0.89, 0.35, "P")
    leg.SetNColumns(2)
    leg.SetHeader("Minimum bias pp #sqrt{s} = 5.02TeV", "C")
    leg.SetFillColor(0)

    return c1, leg, hempty

def efficiencytracking(fileo2, var, sign):
    # plots the efficiency vs pT, eta and phi for all the species
    # it extracts the efficiency from qa-efficiency
    hadron_list = ["Electron", "Muon", "Pion", "Kaon", "Proton", "All"]
    # Other hadrons: "Deuteron", "Triton", "He3", "Alpha"
    color_list = [1, 2, 4, 6, 8, 20]
    marker_list = [20, 21, 22, 34, 45, 47]

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

    c_all, leg_all, h_all = prepare_canvas(var, "c_all_%s_%s" % (sign, var))
    eff_list = []

    heff = fileo2.Get("qa-efficiency/EfficiencyMC")
    for i, had in enumerate(hadron_list):
        print("Looking for an object: %s %s" % (sign, had))
        leff = heff.FindObject("%s %s" % (sign, had))

        if var == "Pt":
            eff = leff.FindObject("ITS-TPC_vsPt_Prm")
        elif var == "Eta":
            eff = leff.FindObject("ITS-TPC_vsEta")
        elif var == "Phi":
            eff = leff.FindObject("ITS-TPC_vsPhi")
        c1, _, hempty = prepare_canvas(var, "c1_%s_%s_%s" % (sign, var, had))
        c1.cd()
        eff.Paint("p")
        gr = eff.GetPaintedGraph().Clone()
        c1.GetListOfPrimitives().Remove(eff)
        c1.Modified()
        for j in range(0, gr.GetN()):
            gr.GetEXlow()[j] = 0
            gr.GetEXhigh()[j] = 0

        gr.SetLineColor(color_list[i])
        gr.SetMarkerColor(color_list[i])
        gr.SetMarkerStyle(marker_list[i])
        eff_list.append(gr)
        gr.Draw(" same p")
        saveCanvas(c1, "efficiency_tracking_%s_%s_%s" % (had, sign, var))
        c_all.cd()
        gr.Draw(" same p")
        leg_all.AddEntry(eff_list[i], had, "p")
    leg_all.Draw()
    saveCanvas(c_all, "efficiency_tracking_alltogether_%s_%s" % (sign, var))


# TODO: This is not updated, it cannot be produced from QAEfficiency output
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


gROOT.SetBatch(True)

parser = argparse.ArgumentParser(description="Arguments to pass")
parser.add_argument("input_file", help="input AnalysisResults.root file")
parser.add_argument("--dump_eff", default=False, action="store_true",
                help="Dump efficiency tree")
args = parser.parse_args()

var_list = ["Pt", "Eta", "Phi"]
sign_list = ["Positive", "Negative"]
hfhadron_list = ["d0", "dplus", "lc", "xic", "jpsi"]

fileo2 = TFile(args.input_file)

if args.dump_eff:
    heff = fileo2.Get("qa-efficiency/EfficiencyMC")
    heff.Print()
    for i in range(heff.GetEntries()):
        teff = heff.At(i)
        teff.Print()

for var in var_list:
    for sign in sign_list:
        efficiencytracking(fileo2, var, sign)
#
#for had in hfhadron_list:
#    efficiencyhadron(had, "Pt")
