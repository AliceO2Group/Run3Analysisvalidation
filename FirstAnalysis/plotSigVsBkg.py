#!/usr/bin/env python

"""
Produce plots of signal vs. background distributions.

Input: file with histograms produced by o2-analysis-hf-task-... in MC mode
Contact: Vít Kučera <vit.kucera@cern.ch>
"""

import os
import sys
from math import ceil, sqrt

from ROOT import TCanvas, TFile, TLegend, gStyle, gROOT, kRed, kBlue

textfont = 42
textsize = 0.042
labelsize = 0.042

gStyle.SetOptStat(0)
gStyle.SetPalette(0)
gStyle.SetCanvasColor(0)
gStyle.SetFrameFillColor(0)
gStyle.SetLegendBorderSize(0)
gStyle.SetLegendFont(textfont)
gStyle.SetLegendTextSize(textsize)

#bottommargin=0.12
#leftmargin=0.12
#rightmargin=0.1
#topmargin=0.05
#gStyle.SetPadBottomMargin(bottommargin)
#gStyle.SetPadLeftMargin(leftmargin)
#gStyle.SetPadRightMargin(rightmargin)
#gStyle.SetPadTopMargin(topmargin)

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
                f"{hadron}, {pt_min} < #it{{p}}_{{T}}/(GeV/#it{{c}}) < {pt_max} (bin {bin_pt})"
            )
            if normalise:
                n_entries_sig = h_sig_px.GetEntries()
                n_entries_bkg = h_bkg_px.GetEntries()
                list_leg[i].AddEntry(h_sig_px, f"Signal ({int(n_entries_sig)} entries)", "L")
                list_leg[i].AddEntry(h_bkg_px, f"Background ({int(n_entries_bkg)} entries)", "L")
                if n_entries_sig > 0:
                    # make sure we don't divide by zero
                    h_sig_px.Scale(1.0 / n_entries_sig, "nosw2")
                else:
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
            h_bkg_px.SetLineColor(4)
            h_bkg_px.SetLineWidth(2)
            h_sig_px.SetLineColor(2)
            h_sig_px.SetLineWidth(2)
            #h_bkg_px.SetTitle("")
            h_bkg_px.GetYaxis().SetMaxDigits(3)
            #yMin = min(h_sig_px.GetMinimum(0), h_bkg_px.GetMinimum(0))
            #yMax = max(h_sig_px.GetMaximum(), h_bkg_px.GetMaximum())+0.2
            #SetHistogram(h_bkg_px, yMin, yMax, marginLow, marginHigh, logScaleH)
            #SetPad(padH, logScaleH)

            h_bkg_px.GetXaxis().SetTitleFont(textfont)
            h_bkg_px.GetXaxis().SetTitleSize(textsize)
            #h_bkg_px.GetXaxis().SetTitle("CPA(#Lambda_{c})")
            #h_bkg_px.GetYaxis().SetTitle("Counts")
            h_bkg_px.GetXaxis().SetTitleOffset(1.0)
            h_bkg_px.GetYaxis().SetTitleOffset(1.15)
            h_bkg_px.GetYaxis().SetTitleFont(textfont)
            h_bkg_px.GetYaxis().SetTitleSize(textsize)
            h_bkg_px.GetXaxis().SetLabelFont(textfont)
            h_bkg_px.GetXaxis().SetLabelSize(labelsize)
            h_bkg_px.GetYaxis().SetLabelFont(textfont)
            h_bkg_px.GetYaxis().SetLabelSize(labelsize)

            # h_bkg_px.GetYaxis().SetRangeUser(0,0.7)
            h_bkg_px.SetFillColorAlpha(kBlue, 0.35)
            h_bkg_px.SetFillStyle(3002)
            h_sig_px.SetFillColorAlpha(kRed+1, 0.35)
            h_sig_px.SetFillStyle(3003)

            h_bkg_px.Draw("histo")
            h_sig_px.Draw("histo same")

            #lat.DrawLatex(0.17,0.88,"ALICE 3 Study, layout v1")
            #lat3.DrawLatex(0.17,0.83,"PYTHIA 8.2, #sqrt{#it{s}} = 14 TeV")
            #lat2.DrawLatex(0.17,0.78,"#Xi^{++}_{cc} #rightarrow #Xi_{c}^{+} #pi^{+}  and charge conj.")
            #lat2.DrawLatex(0.17,0.73,"|#it{y}_{#Xi_{cc}}| #leq 1.44,  4 < #it{p}_{T} < 6 GeV/#it{c}")

            #h_bkg_px.DrawCopy()
            #h_sig_px.DrawCopy("same")
            list_leg[i].Draw()

        save_canvas(canvas, f"distribution_{var}", *formats, dir_output=f"output_{hadron}")


# the large list of variables is to produce the output, the short list is for testing purposes
# (so you don't fill your terminal with errors if something goes wrong within the loop :))

# variables = ["d0Prong0", "d0Prong1", "d0Prong2", "PtProng0", "PtProng1", "PtProng2", "CPA", "Eta", "Declength"]
variables = ["CPA"]

make_plots(
    "../codeHF/AnalysisResults_O2.root", "../codeHF/AnalysisResults_O2.root", *variables
)
