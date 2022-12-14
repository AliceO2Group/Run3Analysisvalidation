#!/usr/bin/env python

# pylint: disable=missing-module-docstring, missing-function-docstring

import argparse

from ROOT import TH1F, TCanvas, TEfficiency, TFile, TLegend, TLatex # pylint: disable=import-error,no-name-in-module
from ROOT import gPad, gStyle, gROOT # pylint: disable=import-error,no-name-in-module

def save_canvas(canvas, title):
    format_list = [".png", ".pdf", ".root"]
    for file_format in format_list:
        canvas.SaveAs(title + file_format)


def prepare_canvas(var, sign, had, det): # pylint: disable=too-many-locals
    cname = f"c_{had}_{det}_{sign}_{var}"
    hname = f"hempty_{had}_{det}_{sign}_{var}"
    ctitle = f"Efficiency for {sign} {had}, {det}"
    if var == "Pt":
        ctitle = f"{ctitle} primaries"

    def get_pt_hist():
        hempty = TH1F(hname, f"{ctitle};Transverse Momentum (GeV/c);Efficiency", 100, 0.05, 10)
        gPad.SetLogx()
        return hempty
    def get_eta_hist():
        return TH1F(hname, f"{ctitle};Pseudorapidity;Efficiency", 100, -1.5, 1.5)
    def get_phi_hist():
        return TH1F(hname, f"{ctitle};Azimuthal angle (rad);Efficiency", 100, 0.0, 6.0)

    hists = {"Pt": get_pt_hist, "Eta": get_eta_hist, "Phi": get_phi_hist}

    canv = TCanvas(cname, "Efficiency")
    canv.SetCanvasSize(800, 600)
    canv.cd()
    canv.SetGridy()
    canv.SetGridx()

    hempty = hists[var]()
    hempty.GetYaxis().CenterTitle()
    hempty.GetXaxis().CenterTitle()
    hempty.GetXaxis().SetNoExponent()
    hempty.GetXaxis().SetMoreLogLabels(1)
    hempty.Draw()

    xmin = hempty.GetXaxis().GetXmin()
    xmax = hempty.GetXaxis().GetXmax()
    ymax = hempty.GetYaxis().GetXmax()
    latexa = TLatex()
    latexa.SetTextSize(0.04)
    latexa.SetTextFont(42)
    latexa.SetTextAlign(3)
    xave = xmin + (xmax - xmin) / 4.0
    latexa.DrawLatex(
        xave, ymax * 0.2, "-0.8 #geq #eta #geq 0.8"
    )
    latexa.DrawLatex(xave, ymax * 0.15, "0.00 #geq #varphi #geq 2#pi")

    leg = TLegend(0.55, 0.15, 0.89, 0.35, "P")
    leg.SetNColumns(2)
    leg.SetHeader("Minimum bias pp #sqrt{s} = 5.02TeV", "C")
    leg.SetFillColor(0)

    return canv, leg, hempty

def efficiencytracking(fileo2, det, sign, var): # pylint: disable=too-many-locals
    # plots the efficiency vs pT, eta and phi for all the species
    # it extracts the efficiency from qa-efficiency
    hadron_list = ["Pion", "Kaon", "Proton", "All"]
    # Other hadrons: "Deuteron", "Triton", "He3", "Alpha"
    color_list = [1, 2, 4, 6, 8, 28]
    marker_list = [20, 21, 22, 34, 45, 47]
    eff_objs = {"Pt": f"{det}_vsPt_Prm", "Eta": f"{det}_vsEta", "Phi": f"{det}_vsPhi"}

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

    results = prepare_canvas(var, sign, "all", det)
    c_all = results[0]
    leg_all = results[1]
    eff_list = []

    heff = fileo2.Get("qa-efficiency/EfficiencyMC")
    for i, had in enumerate(hadron_list):
        leff = heff.FindObject(f"{sign} {had}")
        eff = leff.FindObject(eff_objs[var])
        results_single = prepare_canvas(var, sign, had, det)
        c_single = results_single[0]
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
        save_canvas(c_single, f"efficiency_tracking_{det}_{sign}_{var}_{had}")
        c_all.cd()
        graph.Draw(" same p")
        leg_all.AddEntry(eff_list[i], had, "p")
    leg_all.Draw()
    save_canvas(c_all, f"efficiency_tracking_alltogether_{det}_{sign}_{var}")


# This is not updated, it cannot be produced from QAEfficiency output
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
    det_list = ["ITS-TPC", "ITS-TPC-TOF"]

    infile = TFile(args.input_file)

    if args.dump_eff:
        heff = infile.Get("qa-efficiency/EfficiencyMC")
        heff.Print()
        for i in range(heff.GetEntries()):
            teff = heff.At(i)
            teff.Print()

    for var in var_list:
        for sign in sign_list:
            for det in det_list:
                efficiencytracking(infile, det, sign, var)
    for sign in sign_list:
        efficiencytracking(infile, "ITS", sign, "Pt")

    #hfhadron_list = ["d0", "dplus", "lc", "xic", "jpsi"]
    #for had in hfhadron_list:
    #    efficiencyhadron(had, "Pt")

if __name__ == "__main__":
    main()
