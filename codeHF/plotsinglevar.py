#!/usr/bin/env python
import os
from math import ceil, sqrt

import yaml
from ROOT import TH1F, TH2F, TCanvas, TGraph, TLatex, gPad, TFile, TF1
from ROOT import gStyle, gROOT, TStyle, TLegendEntry, TLegend


def makeSavePaths(title, *fileFormats, outputdir="outputPlots"):
    """
    Saves the canvas as the desired output format in an output directory (default = outputPlots)
    """
    if not os.path.exists(outputdir):
        os.makedirs(outputdir)
    return [outputdir + "/" + title + fileFormat for fileFormat in fileFormats]


def plotsinglevar(
    filename ="../codeHF/AnalysisResults_O2.root",
    dirname="hf-task-xicc-mc",latex="Xi_{cc}", iptBin=2,
    histonmasssig="hPtRecGenDiff", #hPtRecGenDiff hXSecVtxPosDiff
    xmin=-2.0, xmax=2.0, ymin=0.1, ymax=1.e6, rebin=4, logx=1, logy=1,
    xminfit= -2., xmaxfit=2., title="",
    xaxis="Xi_{cc} X vertex reco - gen (cm)"
):
    gStyle.SetOptStat(0)
    gROOT.SetBatch(1)

    fileSig = TFile(filename)
    histo2d = fileSig.Get("%s/%s" % (dirname, histonmasssig))
    hvar = histo2d.ProjectionX("hvar", iptBin + 1, iptBin + 1)
    hvar.GetXaxis().SetRangeUser(xmin, xmax)
    hvar.Draw()
    ptMin = histo2d.GetYaxis().GetBinLowEdge(iptBin + 1)
    ptMax = ptMin + histo2d.GetYaxis().GetBinWidth(iptBin + 1)
    ymax = hvar.GetMaximum()*10;
    hempty = TH2F("hempty", ";%s; Entries" % xaxis,
                  100, xmin, xmax, 100, 0.1, ymax)
    hempty.GetXaxis().SetLabelFont(42)
    hempty.GetXaxis().SetTitleOffset(1)
    hempty.GetXaxis().SetLabelSize(0.03)
    hempty.GetXaxis().SetTitleFont(42)
    hempty.GetYaxis().SetLabelFont(42)
    hempty.GetYaxis().SetTitleOffset(1.35)
    hempty.GetYaxis().SetTitleFont(42)
    hempty.GetZaxis().SetLabelFont(42)
    hempty.GetZaxis().SetTitleOffset(1)
    hempty.GetZaxis().SetTitleFont(42)

    canvas = TCanvas("canvas", "A Simple Graph Example", 881, 176, 668, 616)
    gStyle.SetOptStat(0)
    canvas.SetHighLightColor(2)
    canvas.Range(-1.25, -4.625, 11.25, 11.625)
    canvas.SetFillColor(0)
    canvas.SetBorderMode(0)
    canvas.SetBorderSize(2)
    canvas.SetFrameBorderMode(0)
    canvas.SetFrameBorderMode(0)
    if (logx == 1):
        canvas.SetLogx()
    if (logy == 1):
        canvas.SetLogy()
    canvas.cd()
    hempty.Draw("")
    hvar.Draw("PEsame")

    f = TF1("f","gaus")
    hvar.Fit("f","R","",xminfit, xmaxfit)
    xave = xmin + (xmax - xmin)/4.
    latexb = TLatex()
    latexb.SetTextSize(0.04)
    latexb.SetTextFont(42)
    latexb.SetTextAlign(3)
    mean = f.GetParameter(1)
    sigma = f.GetParameter(2)
    latexb.DrawLatex(xave, ymax*0.35, \
        "#mu = %.5f, #sigma = %.5f" \
         % (mean, sigma),
    )
    latexa = TLatex()
    latexa.SetTextSize(0.04)
    latexa.SetTextFont(42)
    latexa.SetTextAlign(3)
    latexa.DrawLatex(xave, ymax*.2, \
        "%.1f < p_{T} (%s) < %.1f GeV" \
         % (ptMin, latex, ptMax),
    )
    canvas.SaveAs("%s.pdf" % histonmasssig)

plotsinglevar("../codeHF/AnalysisResults_O2.root","hf-task-xicc-mc","Xi_{cc}",2,
              "hPtRecGenDiff",-0.5,0.5,0.1,1e4,4,0,1,-0.5,0.5,"", "Xi_{cc} reco - gen p_{T}")
plotsinglevar("../codeHF/AnalysisResults_O2.root","hf-task-xicc-mc","Xi_{cc}",2,
              "hXSecVtxPosDiff",-0.005,0.005,0.001,1e4,4,0,1,-0.003,0.003, "", "Xi_{cc} X vertex reco - gen (cm)")
plotsinglevar("../codeHF/AnalysisResults_O2.root","hf-task-xicc-mc","Xi_{cc}",4,
              "hmassSig",3.5,3.7,0.001,5e2,4,0,1,3.5,3.7,"","Xi_{cc} invariant mass")
