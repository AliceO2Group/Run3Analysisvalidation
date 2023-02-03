#!/usr/bin/env python
"""
file: efficiency_studies.py
brief: Plotting macro for the qa-efficiency task.
usage: ./efficiency_studies.py AnalysisResults_O2.root [--plot_sel | --dump_eff]
author: Maja Kabus <mkabus@cern.ch>, CERN / Warsaw University of Technology
"""

import argparse

from ROOT import TH1F, TCanvas, TEfficiency, TFile, TLegend, TLatex # pylint: disable=import-error,no-name-in-module
from ROOT import gPad, gStyle, gROOT # pylint: disable=import-error,no-name-in-module
from ROOT import kGreen, kOrange # pylint: disable=import-error,no-name-in-module


def save_canvas(canvas, title):
    """
    Save canvas in png, pdf, root.
    """
    format_list = [".png", ".pdf", ".root"]
    for file_format in format_list:
        canvas.SaveAs(title + file_format)


def get_titles(var, sign, had, det):
    """
    Compose titles and names for histograms and canvas.
    """
    hname = f"hempty_{sign}_{had}_{det}_{var}"
    cname = f"c_{sign}_{had}_{det}_{var}"
    ctitle = f"Efficiency for {sign} {had}, {det}"
    if var == "Pt":
        ctitle = f"{ctitle} primaries"
    return hname, cname, ctitle


def prepare_canvas(var, titles):
    """
    Initialize canvas, axes, legend.
    `hempty` must be captured at return, otherwise ROOT crashes.
    """
    hname, cname, ctitle = titles
    def get_pt_hist():
        hempty = TH1F(hname, f"{ctitle};Transverse Momentum (GeV/c);Efficiency", 16, 0.00, 16)
        #gPad.SetLogx()
        return hempty
    def get_eta_hist():
        return TH1F(hname, f"{ctitle};Pseudorapidity;Efficiency", 16, -1.5, 1.5)
    def get_phi_hist():
        return TH1F(hname, f"{ctitle};Azimuthal angle (rad);Efficiency",
                    16, -2 * 3.1416 - 0.5, 2 * 3.1416 + 0.5)

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
    hempty.GetYaxis().SetNdivisions(11)
    hempty.Draw()

    latexa = TLatex()
    latexa.SetTextSize(0.04)
    latexa.SetTextFont(42)
    latexa.SetTextAlign(3)
    latexa.DrawLatexNDC(0.15, 0.3, "-0.9 #geq #eta #geq 0.9")
    latexa.DrawLatexNDC(0.15, 0.25, "-2#pi #geq #varphi #geq 2#pi")

    leg = TLegend(0.55, 0.15, 0.89, 0.35, "P")
    leg.SetNColumns(2)
    leg.SetHeader("Minimum bias pp #sqrt{s} = 5.02TeV", "C")
    leg.SetFillColor(0)

    return canv, leg, hempty


def efficiency_tracking(heff, det, sign, var, err_y): # pylint: disable=too-many-locals
    """
    Plot efficiency vs pT, eta and phi for all hadron species.
    Pt plots are drawn for primaries.
    """
    hadron_list = ["Pion", "Kaon", "Proton", "Electron"]
    # Other hadrons: "Deuteron", "Triton", "He3", "Alpha"
    color_list = [1, 2, 4, kGreen + 2, kOrange - 3]
    marker_list = [20, 21, 22, 21, 22]
    eff_objs = {"Pt": f"{det}_vsPt_Prm", "Eta": f"{det}_vsEta_Prm", "Phi": f"{det}_vsPhi"}

    gStyle.SetOptStat(0)
    gStyle.SetFrameLineWidth(2)
    gStyle.SetTitleSize(0.045, "x")
    gStyle.SetTitleSize(0.045, "y")
    gStyle.SetMarkerSize(1)
    gStyle.SetLabelOffset(0.015, "x")
    gStyle.SetLabelOffset(0.02, "y")
    gStyle.SetTitleOffset(1.1, "x")
    gStyle.SetTitleOffset(1.0, "y")

    results = prepare_canvas(var, get_titles(var, sign, "all", det))
    c_all = results[0]
    leg_all = results[1]
    eff_list = []

    for i, had in enumerate(hadron_list):
        eff = heff.FindObject(f"{sign} {had}").FindObject(eff_objs[var])
        c_all.cd()
        eff.Paint("p")
        graph = eff.GetPaintedGraph().Clone()
        c_all.GetListOfPrimitives().Remove(eff)
        c_all.Modified()
        for j in range(0, graph.GetN()):
            graph.GetEXlow()[j] = 0
            graph.GetEXhigh()[j] = 0
            if not err_y:
                graph.GetEYlow()[j] = 0
                graph.GetEYhigh()[j] = 0

        graph.SetLineColor(color_list[i])
        graph.SetMarkerColor(color_list[i])
        graph.SetMarkerStyle(marker_list[i])
        eff_list.append(graph)
        graph.Draw(" same p")
        leg_all.AddEntry(eff_list[i], had, "p")

    leg_all.Draw()
    save_canvas(c_all, f"efficiency_tracking_{det}_{sign}_{var}")


def efficiency_hadron(had, var):
    """
    Extract efficiency vs pT for single species (D0, Lc, Jpsi).
    This is not updated, it cannot be produced from QAEfficiency output.
    """
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


def plot_selections(infile):
    """
    Plot track and MC particle selections.
    """
    track_sel = infile.Get("qa-efficiency/MC/trackSelection")
    part_sel = infile.Get("qa-efficiency/MC/particleSelection")

    canv = TCanvas("Track selection", "Track selection")
    canv.SetCanvasSize(800, 600)
    canv.cd()
    track_sel.Draw()

    canv2 = TCanvas("Particle selection", "Particle selection")
    canv2.SetCanvasSize(800, 600)
    canv2.cd()
    part_sel.Draw()

    save_canvas(canv, "efficiency_tracking_track_selection")
    save_canvas(canv2, "efficiency_tracking_particle_selection")


def main():
    """
    Main function.
    """
    gROOT.SetBatch(True)

    parser = argparse.ArgumentParser(description="Arguments to pass")
    parser.add_argument("input_file", help="input AnalysisResults.root file")
    parser.add_argument("--plot_erry", default=False, action="store_true",
                        help="Plot efficiency with y error bars")
    parser.add_argument("--plot_sel", default=False, action="store_true",
                        help="Plot track and particle selections")
    parser.add_argument("--dump_eff", default=False, action="store_true",
                        help="Dump efficiency tree")
    args = parser.parse_args()

    var_list = ["Pt", "Eta", "Phi"]
    sign_list = ["Positive", "Negative"]
    det_list = ["ITS-TPC", "ITS-TPC-TOF"]

    infile = TFile(args.input_file)
    heff = infile.Get("qa-efficiency/EfficiencyMC")

    if args.dump_eff:
        heff.Print()
        for i in range(heff.GetEntries()):
            teff = heff.At(i)
            teff.Print()

    if args.plot_sel:
        plot_selections(infile)

    for var in var_list:
        for sign in sign_list:
            for det in det_list:
                efficiency_tracking(heff, det, sign, var, args.plot_erry)
    for sign in sign_list:
        efficiency_tracking(heff, "ITS", sign, "Pt", args.plot_erry)

    #hfhadron_list = ["d0", "dplus", "lc", "xic", "jpsi"]
    #for had in hfhadron_list:
    #    efficiency_hadron(had, "Pt")

if __name__ == "__main__":
    main()
