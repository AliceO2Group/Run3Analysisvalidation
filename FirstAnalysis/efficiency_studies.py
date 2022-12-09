#!/usr/bin/env python

# pylint: disable=missing-module-docstring, missing-function-docstring

import argparse

from ROOT import TH1F, TCanvas, TEfficiency, TFile, TLegend # pylint: disable=import-error,no-name-in-module
from ROOT import gPad, gStyle, gROOT # pylint: disable=import-error,no-name-in-module

def save_canvas(canvas, title):
    format_list = [".png", ".pdf", ".root"]
    for file_format in format_list:
        canvas.SaveAs(title + file_format)


def prepare_canvas(var, title):
    canv = TCanvas(title, "Efficiency")
    canv.SetCanvasSize(800, 600)
    canv.cd()
    canv.SetGridy()
    canv.SetGridx()

    if var == "Pt":
        hempty = TH1F(f"hempty_{title}", ";Transverse Momentum(GeV/c);Efficiency", 100, 0.05, 10)
        gPad.SetLogx()
    elif var == "Eta":
        hempty = TH1F(f"hempty_{title}", ";Pseudorapidity;Efficiency", 100, -4.0, 4.0)
    elif var == "Phi":
        hempty = TH1F(f"hempty_{title}", ";Azimuthal angle(rad);Efficiency", 100, 0.0, 6.0)

    hempty.GetYaxis().CenterTitle()
    hempty.GetXaxis().CenterTitle()
    hempty.GetXaxis().SetNoExponent()
    hempty.GetXaxis().SetMoreLogLabels(1)
    hempty.Draw()
    leg = TLegend(0.55, 0.15, 0.89, 0.35, "P")
    leg.SetNColumns(2)
    leg.SetHeader("Minimum bias pp #sqrt{s} = 5.02TeV", "C")
    leg.SetFillColor(0)

    return canv, leg, hempty

def efficiencytracking(fileo2, var, sign): # pylint: disable=too-many-locals
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

    c_all, leg_all, h_all = prepare_canvas(var, f"c_all_{sign}_{var}") # pylint: disable=unused-variable
    eff_list = []

    heff = fileo2.Get("qa-efficiency/EfficiencyMC")
    for i, had in enumerate(hadron_list):
        leff = heff.FindObject(f"{sign} {had}")

        if var == "Pt":
            eff = leff.FindObject("ITS-TPC_vsPt_Prm")
        elif var == "Eta":
            eff = leff.FindObject("ITS-TPC_vsEta")
        elif var == "Phi":
            eff = leff.FindObject("ITS-TPC_vsPhi")
        c_single, _, h_single = prepare_canvas(var, f"c_single_{sign}_{var}_{had}") # pylint: disable=unused-variable
        c_single.cd()
        eff.Paint("p")
        graph = eff.GetPaintedGraph().Clone()
        c_single.GetListOfPrimitives().Remove(eff)
        c_single.Modified()
        for j in range(0, graph.GetN()):
            graph.GetEXlow()[j] = 0
            graph.GetEXhigh()[j] = 0

        graph.SetLineColor(color_list[i])
        graph.SetMarkerColor(color_list[i])
        graph.SetMarkerStyle(marker_list[i])
        eff_list.append(graph)
        graph.Draw(" same p")
        save_canvas(c_single, f"efficiency_tracking_{sign}_{var}_{had}")
        c_all.cd()
        graph.Draw(" same p")
        leg_all.AddEntry(eff_list[i], had, "p")
    leg_all.Draw()
    save_canvas(c_all, f"efficiency_tracking_alltogether_{sign}_{var}")


# TODO: This is not updated, it cannot be produced from QAEfficiency output
def efficiencyhadron(had, var):
    # extract the efficiency vs pT for single species(D0, Lc, Jpsi)
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
    ceffhf = TCanvas("ceffhf", "A Simple Graph Example")
    ceffhf.SetCanvasSize(1500, 700)
    ceffhf.Divide(2, 1)
    gPad.SetLogy()
    # hnum = fileo2.Get("qa-tracking-efficiency-%s/%s/num" % (had, var))
    hnum = fileo2.Get(f"hf-task-{had}-mc/h{var}RecSig")
    # hden = fileo2.Get("qa-tracking-efficiency-%s/%s/den" % (had, var))
    hden = fileo2.Get(f"hf-task-{had}-mc/h{var}Gen")
    hnum.Rebin(4)
    hden.Rebin(4)
    eff = TEfficiency(hnum, hden)
    eff.Draw()
    save_canvas(ceffhf, f"efficiency_hfcand_{had}_{var}")


def main():
    gROOT.SetBatch(True)

    parser = argparse.ArgumentParser(description="Arguments to pass")
    parser.add_argument("input_file", help="input AnalysisResults.root file")
    parser.add_argument("--dump_eff", default=False, action="store_true",
                    help="Dump efficiency tree")
    args = parser.parse_args()

    var_list = ["Pt", "Eta", "Phi"]
    sign_list = ["Positive", "Negative"]

    infile = TFile(args.input_file)

    if args.dump_eff:
        heff = infile.Get("qa-efficiency/EfficiencyMC")
        heff.Print()
        for i in range(heff.GetEntries()):
            teff = heff.At(i)
            teff.Print()

    for var in var_list:
        for sign in sign_list:
            efficiencytracking(infile, var, sign)

    #hfhadron_list = ["d0", "dplus", "lc", "xic", "jpsi"]
    #for had in hfhadron_list:
    #    efficiencyhadron(had, "Pt")

if __name__ == "__main__":
    main()
