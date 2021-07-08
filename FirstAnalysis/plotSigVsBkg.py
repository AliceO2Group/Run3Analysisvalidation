#!/usr/bin/env python

"""
Produce plots of signal vs. background distributions.

Input: file with histograms produced by o2-analysis-hf-task-... in MC mode
Contact: Vít Kučera <vit.kucera@cern.ch>
"""

import os
import sys
from math import ceil, sqrt

from ROOT import TCanvas, TFile, TLegend, gStyle, gROOT

gStyle.SetOptStat(0)
gROOT.SetBatch(True)

def create_canvas(n_plots, name, size_x=1500, size_y=700):
    """
    Creates a canvas and automatically divides it in a nxn grid of subpads,
    where n is the lowest value for which all the plots can be drawn

    Returns the canvas
    """
    canvas = TCanvas(name, "canvas title")
    # automatically determine the grid of plots based on the number of plots
    n_div = ceil(sqrt(float(n_plots)))
    canvas.SetCanvasSize(size_x, size_y)
    canvas.Divide(n_div, n_div)
    return canvas


def save_canvas(canvas, title, *file_formats, dir_output="outputPlots"):
    """
    Saves the canvas as the desired output format in an output directory (default = outputPlots)
    """
    if not os.path.exists(dir_output):
        os.makedirs(dir_output)
    for file_format in file_formats:
        canvas.SaveAs(dir_output + "/" + title + file_format)


def make_plots(
    path_file_sig="AnalysisResults_O2.root",
    path_file_bkg="AnalysisResults_O2.root",
    *vars,
    hadron="d0",
    normalise=True,
    rebin=1,
):
    """
    Makes and saves signal vs background plots for each variable passed as vars.
    Creates one plot per pT bin, and saves them all to one canvas per variable.
    Takes a signal file and background file, and any number of variables.
    Particle of interest can be specified by hadron.
    User can also manually set the options to normalise or rebin the histograms,
    defaults are True and 1 (no rebin) respectively.
    """
    formats = [".pdf", ".root"]  # file formats
    file_sig = TFile(path_file_sig)
    if file_sig.IsZombie():
        print(f"Error: Failed to open file {path_file_sig}")
        sys.exit(1)
    file_bkg = TFile(path_file_bkg)
    if file_bkg.IsZombie():
        print(f"Error: Failed to open file {path_file_bkg}")
        sys.exit(1)
    for var in vars:
        print(var)
        name_h_sig = f"hf-task-{hadron}-mc/h{var}RecSig"
        h_sig = file_sig.Get(name_h_sig)
        if not h_sig:
            print(f"Error: Failed to get histogram {name_h_sig}")
            sys.exit(1)
        name_h_bkg = f"hf-task-{hadron}-mc/h{var}RecBg"
        h_bkg = file_bkg.Get(name_h_bkg)
        if not h_bkg:
            print(f"Error: Failed to get histogram {name_h_bkg}")
            sys.exit(1)
        if type(h_sig) != type(h_bkg):
            print(
                f"Error: histograms are not of the same type! Skipping variable {var}..."
            )
            continue
        n_bins_pt = h_sig.GetNbinsY()
        if h_bkg.GetNbinsY() != n_bins_pt:
            print(
                f"Error: histograms have different number of pT bins! Skipping variable {var}"
            )
            continue
        canvas = create_canvas(n_bins_pt, f"{var}_canvas")
        if rebin > 1:
            h_sig.RebinX(rebin)
            h_bkg.RebinX(rebin)
        h_sig.SetLineColor(6)
        if var == "CPA":
            leg = TLegend(0.1, 0.7, 0.4, 0.9, "")  # put the legend in the top left
        else:
            leg = TLegend(0.6, 0.7, 0.9, 0.9, "")  # put the legend in the top right
        leg.SetFillColor(0)
        # create a list of seperate legends, so we can draw a unique one for each pT bin
        # perhaps there is a nicer way of doing this, but I can't think of one
        list_leg = [TLegend(leg) for _ in range(n_bins_pt)]

        for bin_pt in range(1, n_bins_pt + 1):
            i = bin_pt - 1  # iterator starting at 0 for legends
            canvas.cd(bin_pt)
            # create a 1D histogram per pT bin
            if h_sig.InheritsFrom("TH2"):
                h_sig_px = h_sig.ProjectionX("h_sig_px", bin_pt, bin_pt)
                h_bkg_px = h_bkg.ProjectionX("h_bkg_px", bin_pt, bin_pt)
            elif h_sig.InheritsFrom("TH1"):
                h_sig_px = h_sig
                h_bkg_px = h_bkg
            else:
                print(f"Error: Unsupported histogram type: {type(h_sig)}")
                sys.exit(1)
            pt_min = h_sig.GetYaxis().GetBinLowEdge(bin_pt)
            pt_max = h_sig.GetYaxis().GetBinLowEdge(bin_pt + 1)
            h_bkg_px.SetTitle(
                f"{var} signal vs background, {pt_min} < pT {hadron} < {pt_max} (pT bin {bin_pt})"
            )
            if normalise:
                n_entries_sig = h_sig_px.GetEntries()
                n_entries_bkg = h_bkg_px.GetEntries()
                list_leg[i].AddEntry(h_sig_px, f"Signal ({int(n_entries_sig)} entries)")
                list_leg[i].AddEntry(h_bkg_px, f"Background ({int(n_entries_bkg)} entries)")
                if n_entries_sig != 0:
                    # make sure we don't divide by zero
                    h_sig_px.Scale(1.0 / n_entries_sig, "nosw2")
                elif n_entries_sig == 0:
                    print(
                        f"Warning: signal histogram has no entries, so cannot be normalised! (ptbin = {bin_pt})"
                    )
                h_bkg_px.Scale(1.0 / n_entries_bkg, "nosw2")

            # ugly but necessary way of setting the Y range depending on the highest Y value
            # perhaps move this to a dedicated function setYRange(*histo)
            y_max = max(
                h_bkg_px.GetBinContent(h_bkg_px.GetMaximumBin()),
                h_sig_px.GetBinContent(h_sig_px.GetMaximumBin()),
            )
            h_sig_px.GetYaxis().SetRangeUser(0.0, 1.3 * y_max)
            h_bkg_px.GetYaxis().SetRangeUser(0.0, 1.3 * y_max)

            # draw the histograms and legend
            h_bkg_px.DrawCopy()
            h_sig_px.DrawCopy("same")
            list_leg[i].Draw()

        save_canvas(canvas, f"distribution_{var}", *formats, dir_output=f"output_{hadron}")


# the large list of variables is to produce the output, the short list is for testing purposes
# (so you don't fill your terminal with errors if something goes wrong within the loop :))

# variables = ["d0Prong0", "d0Prong1", "d0Prong2", "PtProng0", "PtProng1", "PtProng2", "CPA", "Eta", "Declength"]
variables = ["CPA"]

make_plots(
    "../codeHF/AnalysisResults_O2.root", "../codeHF/AnalysisResults_O2.root", *variables
)
