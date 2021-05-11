#!/usr/bin/env python
from ROOT import TCanvas, TFile, TLegend, gStyle
from math import sqrt, ceil
import os

def createCanvas(nplots, name, sizeX=1500, sizeY=700):
    """
    Creates a canvas and automatically divides it in a nxn grid of subpads,
    where n is the lowest value for which all the plots can be drawn

    Returns the canvas
    """
    canvas = TCanvas(name, "canvas title")
    # automatically determine the grid of plots based on the number of plots
    plts = ceil(sqrt(float(nplots)))
    canvas.SetCanvasSize(sizeX, sizeY)
    canvas.Divide(plts, plts)
    return canvas

def saveCanvas(canvas, title, *fileFormats, outputdir = 'outputPlots'):
    """
    Saves the canvas as the desired output format in an output directory (default = outputPlots)
    """
    if not os.path.exists(outputdir):
        os.makedirs(outputdir)
    for fileFormat in fileFormats:
        canvas.SaveAs(outputdir + "/" + title + fileFormat)

def makePlots(filePathSig = "AnalysisResults_O2.root", filePathBkg = "AnalysisResults_O2.root", *vars, hadron = "x", normalized = True, rebin = 1):
    """
    Makes and saves signal vs background plots for each variable passed as vars.
    Creates one plot per pT bin, and saves them all to one canvas per variable.
    Takes a signal file and background file, and any number of variables.
    Particle of interest can be specified by hadron.
    User can also manually set the options to normalize or rebin the histograms,
    defaults are True and 1 (no rebin) respectively.
    """
    formats = [".pdf", ".root"] # fileformats
    fileSig = TFile(filePathSig)
    fileBkg = TFile(filePathBkg)
    gStyle.SetOptStat(0)
    for var in vars:
        hsig = fileSig.Get(f"hf-task-{hadron}-mc/h{var}RecSig")
        hbkg = fileBkg.Get(f"hf-task-{hadron}-mc/h{var}RecBg")
        if type(hsig) != type(hbkg):
            print(f"Error: histograms are not of the same type! Skipping variable {var}...")
            continue
        nPtBins = hsig.GetNbinsY()
        if hbkg.GetNbinsY() != nPtBins:
            print(f"Error: histograms have different number of pT bins! Skipping variable {var}")
            continue
        cpt = createCanvas(nPtBins, f"{var}_canvas")
        if rebin > 1:
            hsig.RebinX(rebin)
            hbkg.RebinX(rebin)
        hsig.SetLineColor(6)
        if var == "CPA":
            leg = TLegend(0.1, 0.7, 0.4, 0.9, "") # put the legend in the top left
        else:
            leg = TLegend(0.6, 0.7, 0.9, 0.9, "") # put the legend in the top right
        leg.SetFillColor(0)
        # create a list of seperate legends, so we can draw a unique one for each pT bin
        # perhaps there is a nicer way of doing this, but I can't think of one
        leglist = [TLegend(leg) for _ in range(nPtBins)]

        for ptBin in range(1, nPtBins + 1):
            i = ptBin - 1 # iterator starting at 0 for legends
            cpt.cd(ptBin)
            # create a 1D histogram per pT bin
            hsig_px = hsig.ProjectionX("hsig_px", ptBin, ptBin)
            hbkg_px = hbkg.ProjectionX("hbkg_px", ptBin, ptBin)
            ptMin = hsig.GetYaxis().GetBinLowEdge(ptBin)
            ptMax = ptMin + hsig.GetYaxis().GetBinWidth(ptBin)
            hbkg_px.SetTitle(f"{var} signal vs background, {ptMin} < pT {hadron} < {ptMax} (pT bin {ptBin})")
            if normalized:
                nSigEntries = hsig_px.GetEntries()
                nBkgEntries = hbkg_px.GetEntries()
                leglist[i].AddEntry(hsig_px, f"Signal ({int(nSigEntries)} entries)")
                leglist[i].AddEntry(hbkg_px, f"Background ({int(nBkgEntries)} entries)")
                if nSigEntries != 0:
                    # make sure we don't divide by zero
                    hsig_px.Scale(1./nSigEntries, "nosw2")
                elif nSigEntries == 0:
                    print(f"Warning: signal histogram has no entries, so cannot be normalized! (ptbin = {ptBin})")
                hbkg_px.Scale(1./nBkgEntries, "nosw2")

            # ugly but necessary way of setting the Y range depending on the highest Y value
            # perhaps move this to a dedicated function setYRange(*histo)
            maximumY = max(hbkg_px.GetBinContent(hbkg_px.GetMaximumBin()), hsig_px.GetBinContent(hsig_px.GetMaximumBin()))
            hsig_px.GetYaxis().SetRangeUser(
                0.,
                1.3 * maximumY
            )
            hbkg_px.GetYaxis().SetRangeUser(
                0.,
                1.3 * maximumY
            )

            # draw the histograms and legend
            hbkg_px.DrawCopy()
            hsig_px.DrawCopy("same")
            leglist[i].Draw()

        saveCanvas(cpt, f"distribution_{var}", *formats, outputdir = f"output_{hadron}")

# the large list of variables is to produce the output, the short list is for testing purposes
# (so you don't fill your terminal with errors if something goes wrong within the loop :))

# variables = ["d0Prong0", "d0Prong1", "d0Prong2", "PtProng0", "PtProng1", "PtProng2", "CPA", "Eta", "Declength"]
variables = ["CPA"]

makePlots("AnalysisResults_O2_Signal.root", "AnalysisResults_O2_Background.root", *variables)
