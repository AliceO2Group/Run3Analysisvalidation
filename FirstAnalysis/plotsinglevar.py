#!/usr/bin/env python
import os

from ROOT import TF1, TH2F, TCanvas, TFile, TLatex, gROOT, gStyle


def makeSavePaths(title, *fileFormats, outputdir="outputPlots"):
    """
    Saves the canvas as the desired output format in an output directory (default = outputPlots)
    """
    if not os.path.exists(outputdir):
        os.makedirs(outputdir)
    return [outputdir + "/" + title + fileFormat for fileFormat in fileFormats]


def plotsinglevar(
    mode="MM",
    filename="../codeHF/AnalysisResults_O2.root",
    dirname="hf-task-xicc-mc",
    latex="Xi_{cc}",
    iptBin=2,
    histonmasssig="hPtRecGenDiff",  # hPtRecGenDiff hXSecVtxPosDiff
    xmin=-2.0,
    xmax=2.0,
    ymin=0.1,
    ymax=1.0e6,
    rebin=4,
    logx=1,
    logy=1,
    xminfit=-2.0,
    xmaxfit=2.0,
    title="",
    xaxis="Xi_{cc} X vertex reco - gen (cm)",
    dofit=0,
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
    # ymax = hvar.GetMaximum() * 10
    # ymin = hvar.GetMinimum()
    hempty = TH2F("hempty", ";%s; Entries" % xaxis, 100, xmin, xmax, 100, ymin, ymax)
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
    if logx == 1:
        canvas.SetLogx()
    if logy == 1:
        canvas.SetLogy()
    canvas.cd()
    hempty.Draw("")
    hvar.Draw("PEsame")
    latexa = TLatex()
    latexa.SetTextSize(0.04)
    latexa.SetTextFont(42)
    latexa.SetTextAlign(3)
    xave = xmin + (xmax - xmin) / 4.0
    latexa.DrawLatex(
        xave, ymax * 0.2, "%.1f < p_{T} (%s) < %.1f GeV" % (ptMin, latex, ptMax)
    )
    if dofit:
        f = TF1("f", "gaus")
        hvar.Fit("f", "R", "", xminfit, xmaxfit)
        latexb = TLatex()
        latexb.SetTextSize(0.04)
        latexb.SetTextFont(42)
        latexb.SetTextAlign(3)
        mean = f.GetParameter(1)
        sigma = f.GetParameter(2)
        latexb.DrawLatex(xave, ymax * 0.35, "#mu = %.5f, #sigma = %.5f" % (mean, sigma))
    canvas.SaveAs("%s%s.pdf" % (histonmasssig, mode))


plotsinglevar(
    "MuMu",
    "/home/auras/analysis/AnalysisResults_O2_Signal_PID_mumu.root",
    "hf-task-jpsi-mc",
    "J/#psi",
    2,
    "hmassBg",
    2.7,
    3.4,
    0.1,
    5e3,
    4,
    0,
    1,
    2.0,
    4.0,
    "",
    "Inv. Mass (GeV/c2)",
    0,
)
