#!/usr/bin/env python
import os

import yaml
from hfplot.plot_spec_root import ROOTFigure
from hfplot.style import ROOTStyle1D
from ROOT import TFile


def makeSavePaths(title, *fileFormats, outputdir="outputPlots"):
    """
    Saves the canvas as the desired output format in an output directory (default = outputPlots)
    """
    if not os.path.exists(outputdir):
        os.makedirs(outputdir)
    return [outputdir + "/" + title + fileFormat for fileFormat in fileFormats]


def mcdatacomparison_singlevar(
    hadron="Xi_cc", collision="pp14p0", yrange="absy1p44", varindex=0, iptBin=3
):
    """
    Make distribution comparisons
    """
    with open(r"database.yaml") as database:
        paraml = yaml.load(database, Loader=yaml.FullLoader)
    param = paraml[hadron][collision][yrange]
    latexcand = param["latexcand"]
    inputBkg = param["inputBkg"]
    inputSig = param["inputSig"]
    dirname = param["dirname"]
    normalized = param["normalized"]
    lvarlist = param["varlist"]
    lvarlatex = param["varlatex"]
    lhistonamesig = param["histonamesig"]
    lhistonamebkg = param["histonamebkg"]
    lrebin = param["rebin"]

    formats = [".pdf", ".root", ".C"]  # fileformats
    fileBkg = TFile(inputBkg)
    fileSig = TFile(inputSig)

    # Define some styles
    style_sig = ROOTStyle1D()
    style_sig.linecolor = 2
    style_sig.markercolor = 2
    style_sig.markerstyle = 22
    style_sig.markersize = 1
    style_sig.draw_options = "PE"
    style_bkg = ROOTStyle1D()
    style_bkg.markercolor = 1
    style_bkg.linecolor = 1
    style_bkg.markerstyle = 24
    style_bkg.markersize = 1
    style_bkg.draw_options = "PE"

    var = lvarlist[varindex]

    hsig = fileSig.Get(f"{dirname}/{lhistonamesig[varindex]}")
    hbkg = fileBkg.Get(f"{dirname}/{lhistonamebkg[varindex]}")

    hsig_px = hsig.ProjectionX(f"hsig_px_var{var}_pt{iptBin}", iptBin, iptBin + 1)
    hbkg_px = hbkg.ProjectionX(f"hbkg_px_var{var}_pt{iptBin}", iptBin + 1, iptBin + 1)
    ptMin = hsig.GetYaxis().GetBinLowEdge(iptBin + 1)
    ptMax = ptMin + hsig.GetYaxis().GetBinWidth(iptBin + 1)
    figure = ROOTFigure(1, 1, size=(700, 600), row_margin=0.05, column_margin=0.1)
    figure.axes("x", title=lvarlatex[varindex])
    figure.axes("y", "Entries")
    rebin = lrebin[varindex]
    if rebin > 1:
        hsig_px.RebinX(rebin)
        hbkg_px.RebinX(rebin)
    nSigEntries = hsig_px.Integral()
    nBkgEntries = hbkg_px.Integral()

    if not nSigEntries or not nBkgEntries:
        print(
            f"ERROR: Found empty signal or background distribution for variable={var} in pT bin={iptBin}"
        )
    if normalized:
        for ibin in range(hsig_px.GetNbinsX()):
            bincontent = hsig_px.GetBinContent(ibin + 1)
            hsig_px.SetBinContent(ibin + 1, bincontent / nSigEntries)
            hsig_px.SetBinError(ibin + 1, 0.0)

        for ibin in range(hbkg_px.GetNbinsX()):
            bincontent = hbkg_px.GetBinContent(ibin + 1)
            hbkg_px.SetBinContent(ibin + 1, bincontent / nBkgEntries)
            hbkg_px.SetBinError(ibin + 1, 0.0)
    figure.add_object(
        hsig_px, style=style_sig, label=f"Sig before norm ({int(nSigEntries)} entries)"
    )
    figure.add_object(
        hbkg_px, style=style_bkg, label=f"Bkg before norm ({int(nBkgEntries)} entries)"
    )
    figure.add_text(
        f"{ptMin:.1f} GeV < p_{{T}} ({latexcand}) < {ptMax:.1f} GeV", 0.1, 0.85
    )
    #    style_bkg.linecolor = 1
    #plot = figure.change_plot()
    # Set y axis limits explicitly
    figure.create()
    for save_paths in makeSavePaths(
        f"distribution_{var}", *formats, outputdir=f"output_{hadron}"
    ):
        figure.save(save_paths)


mcdatacomparison_singlevar()
