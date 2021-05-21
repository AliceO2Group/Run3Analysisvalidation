#!/usr/bin/env python
import os
from math import ceil, sqrt
import yaml

from ROOT import TCanvas, TFile, TLegend, gStyle, gPad, gROOT, TH2F, TLatex


def createCanvas(nplots, name, sizeX=1500, sizeY=900):
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


def saveCanvas(canvas, title, *fileFormats, outputdir="outputPlots"):
    """
    Saves the canvas as the desired output format in an output directory (default = outputPlots)
    """
    if not os.path.exists(outputdir):
        os.makedirs(outputdir)
    for fileFormat in fileFormats:
        canvas.SaveAs(outputdir + "/" + title + fileFormat)


def distr_studies(hadron="Xi_cc", collision="pp14p0", yrange="absy1p44"):
    gROOT.SetBatch(1)
    """
    Make distribution comparisons
    """
    with open(r"database.yaml") as database:
        param = yaml.load(database, Loader=yaml.FullLoader)
    latexcand = param[hadron][collision][yrange]["latexcand"]
    inputBkg = param[hadron][collision][yrange]["inputBkg"]
    inputSig = param[hadron][collision][yrange]["inputSig"]
    dirname = param[hadron][collision][yrange]["dirname"]
    normalized = param[hadron][collision][yrange]["normalized"]
    lvarlist = param[hadron][collision][yrange]["varlist"]
    lvarlatex = param[hadron][collision][yrange]["varlatex"]
    lhistonamesig = param[hadron][collision][yrange]["histonamesig"]
    lhistonamebkg = param[hadron][collision][yrange]["histonamebkg"]
    lymin = param[hadron][collision][yrange]["ymin"]
    lymax = param[hadron][collision][yrange]["ymax"]
    lxmin = param[hadron][collision][yrange]["xmin"]
    lxmax = param[hadron][collision][yrange]["xmax"]
    lrebin = param[hadron][collision][yrange]["rebin"]
    ldolog = param[hadron][collision][yrange]["dolog"]
    ldologx = param[hadron][collision][yrange]["dologx"]

    formats = [".pdf", ".root", ".C"]  # fileformats
    fileBkg = TFile(inputBkg)
    fileSig = TFile(inputSig)
    gStyle.SetOptStat(0)

    lhistosig = []
    lhistobkg = []
    nPtBins = 0
    lhemptyvar = []
    lptMin = []
    lptMax = []

    for index, var in enumerate(lvarlist):
        ymin = lymin[index]
        ymax = lymax[index]
        xmin = lxmin[index]
        xmax = lxmax[index]
        varlatex = lvarlatex[index]
        lhistosigvar = []
        lhistobkgvar = []
        hempty = TH2F("hemptyvar%d" % index, "", 100, xmin, xmax, 100, ymin, ymax)
        hempty.GetYaxis().SetTitle("Entries")
        hempty.GetXaxis().SetTitle(varlatex)
        hempty.GetXaxis().SetTitleOffset(1.0)
        hempty.GetYaxis().SetTitleOffset(1.0)
        hempty.GetXaxis().SetTitleSize(0.045)
        hempty.GetYaxis().SetTitleSize(0.045)
        lhemptyvar.append(hempty)
        histonamesig = lhistonamesig[index]
        histonamebkg = lhistonamebkg[index]
        hsig = fileSig.Get(f"{dirname}/{histonamesig}")
        hbkg = fileBkg.Get(f"{dirname}/{histonamebkg}")
        nPtBins = hsig.GetNbinsY()
        for iptBin in range(nPtBins):
            hsig_px = hsig.ProjectionX(
                "hsig_px_var%s_pt%d" % (var, iptBin), iptBin + 1, iptBin + 1
            )
            hbkg_px = hbkg.ProjectionX(
                "hbkg_px_var%s_pt%d" % (var, iptBin), iptBin + 1, iptBin + 1
            )
            lhistosigvar.append(hsig_px)
            lhistobkgvar.append(hbkg_px)
            if index == 0:
                ptMin = hsig.GetYaxis().GetBinLowEdge(iptBin + 1)
                ptMax = ptMin + hsig.GetYaxis().GetBinWidth(iptBin + 1)
                lptMin.append(ptMin)
                lptMax.append(ptMax)

        lhistosig.append(lhistosigvar)
        lhistobkg.append(lhistobkgvar)

    for index, var in enumerate(lvarlist):
        cpt = createCanvas(nPtBins, f"{var}_canvas")
        dolog = ldolog[index]
        dologx = ldologx[index]
        ymin = lymin[index]
        ymax = lymax[index]
        xmin = lxmin[index]
        rebin = lrebin[index]
        xmax = lxmax[index]
        leg = TLegend(0.2, 0.75, 0.8, 0.85, "")
        leglist = [TLegend(leg) for _ in range(nPtBins)]
        for iptBin in range(nPtBins):
            cpt.cd(iptBin + 1)
            if rebin > 1:
                lhistosig[index][iptBin].RebinX(rebin)
                lhistobkg[index][iptBin].RebinX(rebin)
            lhistosig[index][iptBin].GetXaxis().SetRangeUser(xmin, xmax)
            lhistobkg[index][iptBin].GetXaxis().SetRangeUser(xmax, xmax)
            lhistosig[index][iptBin].SetMaximum(ymax)
            lhistobkg[index][iptBin].SetMaximum(ymax)
            if dolog == 1:
                gPad.SetLogy()
            if dologx == 1:
                gPad.SetLogx()
            nSigEntries = lhistosig[index][iptBin].Integral()
            nBkgEntries = lhistobkg[index][iptBin].Integral()
            if normalized:
                if nSigEntries != 0:
                    for ibin in range(lhistosig[index][iptBin].GetNbinsX()):
                        bincontent = lhistosig[index][iptBin].GetBinContent(ibin + 1)
                        lhistosig[index][iptBin].SetBinContent(
                            ibin + 1, bincontent / nSigEntries
                        )
                        lhistosig[index][iptBin].SetBinError(ibin + 1, 0.0)
                else:
                    print(
                        "ERROR: no counts in signal distribution for var=%s, pt bin=%d"
                        % (var, iptBin)
                    )
                if nBkgEntries != 0:
                    for ibin in range(lhistobkg[index][iptBin].GetNbinsX()):
                        bincontent = lhistobkg[index][iptBin].GetBinContent(ibin + 1)
                        lhistobkg[index][iptBin].SetBinContent(
                            ibin + 1, bincontent / nBkgEntries
                        )
                        lhistobkg[index][iptBin].SetBinError(ibin + 1, 0.0)
                else:
                    print(
                        "ERROR: no counts in bkg distribution for var=%s, pt bin=%d"
                        % (var, iptBin)
                    )

            lhistosig[index][iptBin].SetLineColor(2)
            lhemptyvar[index].Draw()
            lhistosig[index][iptBin].Draw("histsame")
            lhistobkg[index][iptBin].Draw("histsame")
            latexa = TLatex()
            latexa.SetTextSize(0.055)
            latexa.SetTextFont(42)
            latexa.SetTextAlign(3)
            if dologx == 0:
                latexa.DrawLatex(
                    xmin + (xmax - xmin) / 4.0,
                    ymax * 2.5,
                    "%.1f < p_{T} (%s) < %.1f GeV"
                    % (lptMin[iptBin], latexcand, lptMax[iptBin]),
                )
            leglist[iptBin].AddEntry(
                lhistosig[index][iptBin],
                f"Sig before norm ({int(nSigEntries)} entries)",
            )
            leglist[iptBin].AddEntry(
                lhistobkg[index][iptBin],
                f"Bkg before norm ({int(nBkgEntries)} entries)",
            )
            leglist[iptBin].Draw()
        saveCanvas(cpt, f"distribution_{var}", *formats, outputdir=f"output_{hadron}")


distr_studies()
