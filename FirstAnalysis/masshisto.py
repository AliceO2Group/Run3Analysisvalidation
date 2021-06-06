#!/usr/bin/env python
import os

from ROOT import TF1, TH2F, TCanvas, TFile, TLatex, gROOT, gStyle, yaml


def makeSavePaths(title, *fileFormats, outputdir="outputPlots"):
    """
    Saves the canvas as the desired output format in an output directory (default = outputPlots)
    """
    if not os.path.exists(outputdir):
        os.makedirs(outputdir)
    return [outputdir + "/" + title + fileFormat for fileFormat in fileFormats]


def masshisto(
    hadron="Xi_cc", collision="pp14p0", yrange="absy1p44", varindex=0, iptBin=5
):
    """
    Make distribution comparisons
    """
    gStyle.SetOptStat(0)
    gROOT.SetBatch(1)
    with open(r"database.yaml") as database:
        param = yaml.load(database, Loader=yaml.FullLoader)
    latexcand = param[hadron][collision][yrange]["latexcand"]
    # inputBkg = param[hadron][collision][yrange]["inputBkg"]
    inputSig = param[hadron][collision][yrange]["inputSig"]
    dirname = param[hadron][collision][yrange]["dirname"]
    lhistonamesig = param[hadron][collision][yrange]["histonamesig"]
    # lhistonamebkg = param[hadron][collision][yrange]["histonamebkg"]
    lxmin = param[hadron][collision][yrange]["xmin"]
    lxmax = param[hadron][collision][yrange]["xmax"]
    histonmasssig = lhistonamesig[0]
    # histonmassbkg = lhistonamebkg[0]

    # fileBkg = TFile(inputBkg)
    fileSig = TFile(inputSig)
    histo2d = fileSig.Get("%s/%s" % (dirname, histonmasssig))
    hmass = histo2d.ProjectionX("hmass", iptBin + 1, iptBin + 1)
    hmass.GetXaxis().SetRangeUser(lxmin[0], lxmax[0])
    hmass.Draw()
    ptMin = histo2d.GetYaxis().GetBinLowEdge(iptBin + 1)
    ptMax = ptMin + histo2d.GetYaxis().GetBinWidth(iptBin + 1)
    print(ptMin, ptMax)
    yminhisto = hmass.GetMinimum()
    ymaxhisto = hmass.GetMaximum() * 1.5
    hempty = TH2F(
        "hempty",
        ";Invariant mass (GeV/c^{2}); Entries",
        100,
        lxmin[0],
        lxmax[0],
        100,
        yminhisto,
        ymaxhisto,
    )
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
    canvas.cd()
    hempty.Draw("")
    hmass.Draw("PEsame")

    f = TF1("f", "gaus")
    hmass.Fit("f", "R", "", lxmin[0], lxmax[0])
    xave = lxmin[0] + (lxmax[0] - lxmin[0]) / 4.0
    latexa = TLatex()
    latexa.SetTextSize(0.04)
    latexa.SetTextFont(42)
    latexa.SetTextAlign(3)
    latexa.DrawLatex(
        xave,
        ymaxhisto * 0.8,
        "%.1f < p_{T} (%s) < %.1f GeV" % (ptMin, latexcand, ptMax),
    )
    latexb = TLatex()
    latexb.SetTextSize(0.04)
    latexb.SetTextFont(42)
    latexb.SetTextAlign(3)
    mean = f.GetParameter(1)
    sigma = f.GetParameter(2)
    latexb.DrawLatex(
        xave,
        ymaxhisto * 0.87,
        "#mu = %.1f, #sigma = %.1f GeV/c^2" % (1000.0 * mean, 1000.0 * sigma),
    )
    canvas.SaveAs("masshistotest.pdf")


masshisto()
