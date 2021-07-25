#!/usr/bin/env python3

"""
Produce plots of signal vs background distributions.

Takes a signal file and background file and any number of variables.
Makes and saves signal vs background plots for each decay channel and each variable.
Creates one plot per pT bin and saves them all to one canvas per variable
or optionally in separate canvases (singlepad = True).
User can also manually set the options to normalise or rebin the histograms,
defaults are True and 1 (no rebin), respectively.

Input: file with histograms produced by o2-analysis-hf-task-... in MC mode

Usage: python3 plotSigVsBkg.py

Parameters:
- path_file_sig: path to file with signal distributions
- path_file_bkg: path to file with background distributions
- variables: list of variable strings
- decays: list of decay channel strings (appearing in hf-task-{decay}...)

Contributors:
    Rik Spijkers <r.spijkers@students.uu.nl>
    Luigi Dello Stritto <luigi.dello.stritto@cern.ch>
    Vít Kučera <vit.kucera@cern.ch>
"""

import os
import sys
from math import ceil, sqrt

from ROOT import TCanvas, TFile, TLegend, gROOT, gStyle, kBlue, kRed


def create_canvas(n_plots, name, size_x=1500, size_y=800):
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


def save_canvas(canvas, name, *file_formats, dir_output="outputPlots"):
    """
    Saves the canvas as the desired output format in an output directory.
    """
    if not os.path.exists(dir_output):
        os.makedirs(dir_output)
    for file_format in file_formats:
        file_format = file_format.replace(".", "")
        canvas.SaveAs(f"{dir_output}/{name}.{file_format}")


def set_histogram(his, y_min, y_max, margin_low, margin_high, logscale):
    """
    Sets histogram axes labels and range.
    """
    for ax in his.GetXaxis(), his.GetYaxis():
        ax.SetTitleFont(textfont)
        ax.SetTitleSize(textsize)
        ax.SetTitleOffset(1.0)
        ax.SetLabelFont(textfont)
        ax.SetLabelSize(labelsize)
    k = 1.0 - margin_high - margin_low
    if logscale and y_min > 0 and y_max > 0:
        y_range = y_max / y_min
        his.GetYaxis().SetRangeUser(
            y_min / pow(y_range, margin_low / k), y_max * pow(y_range, margin_high / k)
        )
    else:
        logscale = False
        y_range = y_max - y_min
        his.GetYaxis().SetRangeUser(
            y_min - margin_low / k * y_range, y_max + margin_high / k * y_range
        )
    return logscale


def main():
    """
    Main plotting function

    Loops over decays, variables and pT bins.
    """
    file_sig = TFile(path_file_sig)
    if file_sig.IsZombie():
        print(f"Error: Failed to open file {path_file_sig}")
        sys.exit(1)
    file_bkg = TFile(path_file_bkg)
    if file_bkg.IsZombie():
        print(f"Error: Failed to open file {path_file_bkg}")
        sys.exit(1)
    for decay in decays:
        print(f"Decay: {decay}")
        for var in variables:
            print(f"Variable: {var}")
            name_h_sig = f"hf-task-{decay}-mc/h{var}RecSig"
            h_sig = file_sig.Get(name_h_sig)
            if not h_sig:
                print(
                    f"Error: Failed to get histogram {name_h_sig}! Skipping variable {var}"
                )
                continue
            name_h_bkg = f"hf-task-{decay}-mc/h{var}RecBg"
            h_bkg = file_bkg.Get(name_h_bkg)
            if not h_bkg:
                print(
                    f"Error: Failed to get histogram {name_h_bkg}! Skipping variable {var}"
                )
                continue
            if type(h_sig) != type(h_bkg):
                print(
                    f"Error: Histograms are not of the same type! Skipping variable {var}..."
                )
                continue
            n_bins_pt = h_sig.GetNbinsY()
            print(f"pT bins: {n_bins_pt}")
            if h_bkg.GetNbinsY() != n_bins_pt:
                print(
                    f"Error: Histograms have different number of pT bins! Skipping variable {var}"
                )
                continue
            # rebin along the variable axis
            if rebin > 1:
                h_sig.RebinX(rebin)
                h_bkg.RebinX(rebin)
            # create canvas and legend
            if not singlepad:
                canvas_all = create_canvas(n_bins_pt, f"{var}_canvas")
            list_leg = []  # one legend for each pT bin

            for i in range(n_bins_pt):
                bin_pt = i + 1
                if singlepad:
                    canvas_single = create_canvas(1, f"{var}_canvas_{bin_pt}")
                    pad = canvas_single.cd(1)
                else:
                    pad = canvas_all.cd(bin_pt)
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
                # get pT range
                pt_min = h_sig.GetYaxis().GetBinLowEdge(bin_pt)
                pt_max = h_sig.GetYaxis().GetBinLowEdge(bin_pt + 1)
                h_bkg_px.SetTitle(
                    #f"{decay}, {pt_min:g} #leq #it{{p}}_{{T}}/(GeV/#it{{c}}) < {pt_max:g} (bin {bin_pt})"
                    f"{labels[decay]}, {pt_min:g} #leq #it{{p}}_{{T}}/(GeV/#it{{c}}) < {pt_max:g} (bin {bin_pt})"
                )
                h_bkg_px.SetYTitle("entries")
                # create legend and entries
                list_leg.append(TLegend(0.5, 0.8, 0.95, 0.9))
                list_leg[i].SetNColumns(2)
                n_entries_sig = h_sig_px.GetEntries()
                n_entries_bkg = h_bkg_px.GetEntries()
                list_leg[i].AddEntry(h_sig_px, f"Sig. ({int(n_entries_sig)})", "FL")
                list_leg[i].AddEntry(h_bkg_px, f"Bkg. ({int(n_entries_bkg)})", "FL")
                # normalise histograms
                if normalise:
                    if n_entries_sig > 0 and n_entries_bkg > 0:
                        h_sig_px.Scale(1.0 / n_entries_sig, "nosw2")
                        h_bkg_px.Scale(1.0 / n_entries_bkg, "nosw2")
                        h_bkg_px.SetYTitle("normalised entries")
                    else:
                        print(
                            f"Warning: Signal or backgound histogram has no entries, "
                            f"so cannot be normalised! (ptbin = {bin_pt})"
                        )
                # determine y range
                y_max = max(
                    h_bkg_px.GetBinContent(h_bkg_px.GetMaximumBin()),
                    h_sig_px.GetBinContent(h_sig_px.GetMaximumBin()),
                )
                y_min = min(h_bkg_px.GetMinimum(0), h_sig_px.GetMinimum(0))
                logscale = set_histogram(
                    h_bkg_px, y_min, y_max, margin_low, margin_high, True
                )
                if logscale:
                    pad.SetLogy()

                # draw the histograms and legend
                h_bkg_px.SetLineColor(4)
                h_bkg_px.SetLineWidth(2)
                h_bkg_px.SetFillColorAlpha(kBlue, 0.35)
                h_bkg_px.SetFillStyle(3345)
                h_bkg_px.GetYaxis().SetMaxDigits(3)
                h_bkg_px.DrawCopy("hist")

                h_sig_px.SetLineColor(2)
                h_sig_px.SetLineWidth(2)
                h_sig_px.SetFillColorAlpha(kRed + 1, 0.35)
                h_sig_px.SetFillStyle(3354)
                h_sig_px.DrawCopy("hist same")

                # lat.DrawLatex(0.17,0.88,"ALICE 3 Study, layout v1")
                # lat3.DrawLatex(0.17,0.83,"PYTHIA 8.2, #sqrt{#it{s}} = 14 TeV")
                # lat2.DrawLatex(0.17,0.78,"#Xi^{++}_{cc} #rightarrow #Xi_{c}^{+} #pi^{+}  and charge conj.")
                # lat2.DrawLatex(0.17,0.73,"|#it{y}_{#Xi_{cc}}| #leq 1.44,  4 < #it{p}_{T} < 6 GeV/#it{c}")

                list_leg[i].Draw()

                pad.RedrawAxis()

                if singlepad:
                    save_canvas(
                        canvas_single, f"sig_vs_bkg_{var}_{bin_pt}", *formats, dir_output=f"output_{decay}"
                    )

            if not singlepad:
                save_canvas(
                    canvas_all, f"sig_vs_bkg_{var}", *formats, dir_output=f"output_{decay}"
                )

# TLatex labels of decay channels
labels = {
"d0": "D^{0} #rightarrow #pi K",
"lc": "#Lambda^{+}_{c} #rightarrow p K #pi"
}

# general settings
gROOT.SetBatch(True)
gStyle.SetOptStat(0)
gStyle.SetPalette(0)
gStyle.SetFrameFillColor(0)

# text attributes
textfont = 42
textsize = 0.05
labelsize = 0.05

# plotting range
margin_low = 0.05
margin_high = 0.14

# canvas settings
gStyle.SetCanvasColor(0)
gStyle.SetPadLeftMargin(0.1)
gStyle.SetPadBottomMargin(0.12)
gStyle.SetPadRightMargin(0.05)
gStyle.SetPadTopMargin(0.1)
singlepad = False # make one canvas per pT bin

# legend settings
gStyle.SetLegendFillColor(0)
# gStyle.SetLegendBorderSize(0)
gStyle.SetLegendFont(textfont)
gStyle.SetLegendTextSize(textsize)

formats = ["pdf", "png", "root"]  # output file formats
normalise = True
rebin = 1

path_file_sig = "../codeHF/AnalysisResults_O2.root"
path_file_bkg = "../codeHF/AnalysisResults_O2.root"

# variables = ["d0Prong0", "d0Prong1", "d0Prong2", "PtProng0", "PtProng1", "PtProng2", "CPA", "Eta", "Declength"]
variables = ["CPA", "Pt", "Eta"]

decays = ["d0"]

main()
