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
    dirname="hf-task-xicc-mc",hadron="Xi_cc", iptBin=5, histonmasssig="hXSecVtxPosDiff",
    xmin=0.0002, xmax=0.01, ymin=0.01, ymax=1.e4, rebin=4, logx=1, logy=1
):
    gStyle.SetOptStat(0)
    gROOT.SetBatch(1)

    fileSig = TFile(inputSig)
    histo2d = fileSig.Get("%s/%s" % (dirname, histonmasssig))
    hvar = histo2d.ProjectionX("hvar", iptBin + 1, iptBin + 1)
    hvar.GetXaxis().SetRangeUser(xmin, xmax)
    hvar.Draw()
    ptMin = histo2d.GetYaxis().GetBinLowEdge(iptBin + 1)
    ptMax = ptMin + histo2d.GetYaxis().GetBinWidth(iptBin + 1)
    yminhisto = hvar.GetMinimum();
    ymaxhisto = hvar.GetMaximum()*1.5;
    hempty = TH2F("hempty", ";%s; Entries" % histonmasssig,
                  100, xmin, xmax, 100, ymin, ymax)
    hempty.GetXaxis().SetLabelFont(42)
    hempty.GetXaxis().SetTitleOffset(1)
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
        canvas.SetLogy()
    if (logy == 1):
        canvas.SetLogy()
    canvas.cd()
    hempty.Draw("")
    hvar.Draw("PEsame")

    latexa = TLatex()
    latexa.SetTextSize(0.04)
    latexa.SetTextFont(42)
    latexa.SetTextAlign(3)
    latexa.DrawLatex(xave, ymaxhisto*0.8, \
        "%.1f < p_{T} (%s) < %.1f GeV" \
         % (ptMin, latexcand, ptMax),
    )
    canvas.SaveAs("%s.pdf" % histonmasssig)

plotsinglevar()

