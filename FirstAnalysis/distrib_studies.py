#!/usr/bin/env python
import os
from math import ceil, sqrt

import yaml
from hfplot.plot_spec_root import ROOTFigure
from hfplot.style import StyleObject1D
from ROOT import TFile


def makeSavePaths(title, *fileFormats, outputdir="outputPlots"):
    """
    Saves the canvas as the desired output format in an output directory (default = outputPlots)
    """
    if not os.path.exists(outputdir):
        os.makedirs(outputdir)
    return [outputdir + "/" + title + fileFormat for fileFormat in fileFormats]


def distr_studies(hadron="Xi_cc", collision="pp14p0", yrange="absy1p44"):
    """
    Make distribution comparisons
    """
    with open(r"database.yaml") as database:
        param = yaml.safe_load(database)
    latexcand = param[hadron][collision][yrange]["latexcand"]
    inputBkg = param[hadron][collision][yrange]["inputBkg"]
    inputSig = param[hadron][collision][yrange]["inputSig"]
    dirname = param[hadron][collision][yrange]["dirname"]
    normalized = param[hadron][collision][yrange]["normalized"]
    lvarlist = param[hadron][collision][yrange]["varlist"]
    lvarlatex = param[hadron][collision][yrange]["varlatex"]
    lhistonamesig = param[hadron][collision][yrange]["histonamesig"]
    lhistonamebkg = param[hadron][collision][yrange]["histonamebkg"]
    lrebin = param[hadron][collision][yrange]["rebin"]
    ldolog = param[hadron][collision][yrange]["dolog"]
    ldologx = param[hadron][collision][yrange]["dologx"]

    formats = [".pdf", ".root", ".C"]  # fileformats
    fileBkg = TFile(inputBkg)
    fileSig = TFile(inputSig)

    lhistosig = []
    lhistobkg = []
    nPtBins = 0
    lptMin = []
    lptMax = []

    # Define some styles
    style_sig = StyleObject1D()
    style_sig.markercolor = 2
    style_sig.markerstyle = 21
    style_sig.markersize = 2
    style_sig.draw_options = "P"
    style_bkg = StyleObject1D()
    style_bkg.markerstyle = 23
    style_bkg.markersize = 2
    style_bkg.draw_options = "P"

    for index, var in enumerate(lvarlist):

        lhistosigvar = []
        lhistobkgvar = []
        hsig = fileSig.Get(f"{dirname}/{lhistonamesig[index]}")
        hbkg = fileBkg.Get(f"{dirname}/{lhistonamebkg[index]}")

        nPtBins = hsig.GetNbinsY()
        print(var)
        for iptBin in range(nPtBins):
            # Collect the histogram projections in bins of pT for each variable
            hsig_px = hsig.ProjectionX(
                f"hsig_px_var{var}_pt{iptBin}", iptBin + 1, iptBin + 1
            )
            hbkg_px = hbkg.ProjectionX(
                f"hbkg_px_var{var}_pt{iptBin}", iptBin + 1, iptBin + 1
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
        # sort out required number of columns and rows for a squared grid and create a figure
        n_cols_rows = ceil(sqrt(nPtBins))
        column_margin = [(0.05, 0.01)] * n_cols_rows
        row_margin = [(0.05, 0.01)] * n_cols_rows
        figure = ROOTFigure(
            n_cols_rows,
            n_cols_rows,
            row_margin=row_margin,
            column_margin=column_margin,
            size=(1500, 900),
        )
        # can adjust some axis properties globally
        figure.axes(label_size=0.02, title_size=0.02)
        # here we use the feature to only apply to certain axes
        figure.axes("x", title=lvarlatex[index], title_offset=1)
        figure.axes("y", title="Entries", title_offset=1.08)
        # legend positioning, default would be "top right", so just put left (top will be added by default)
        figure.legend(position=(0.05, 0.6, 0.7, 0.8))

        rebin = lrebin[index]

        for iptBin in range(nPtBins):
            hist_sig = lhistosig[index][iptBin]
            hist_bkg = lhistobkg[index][iptBin]

            if rebin > 1:
                hist_sig.RebinX(rebin)
                hist_bkg.RebinX(rebin)

            nSigEntries = hist_sig.Integral()
            nBkgEntries = hist_bkg.Integral()

            if not nSigEntries or not nBkgEntries:
                print(
                    f"ERROR: Found empty signal or background distribution for variable={var} in pT bin={iptBin}"
                )
                continue

            if normalized:
                for ibin in range(hist_sig.GetNbinsX()):
                    bincontent = hist_sig.GetBinContent(ibin + 1)
                    hist_sig.SetBinContent(ibin + 1, bincontent / nSigEntries)
                    hist_sig.SetBinError(ibin + 1, 0.0)

                for ibin in range(hist_bkg.GetNbinsX()):
                    bincontent = hist_bkg.GetBinContent(ibin + 1)
                    hist_bkg.SetBinContent(ibin + 1, bincontent / nBkgEntries)
                    hist_bkg.SetBinError(ibin + 1, 0.0)

            figure.define_plot(x_log=ldologx[index], y_log=ldolog[index])
            figure.add_object(
                hist_sig,
                style=style_sig,
                label=f"Sig before norm ({int(nSigEntries)} entries)",
            )
            figure.add_object(
                hist_bkg,
                style=style_bkg,
                label=f"Bkg before norm ({int(nBkgEntries)} entries)",
            )
            figure.add_text(
                f"{lptMin[iptBin]:.1f} GeV < p_{{T}} ({latexcand}) < {lptMax[iptBin]:.1f} GeV",
                0.0,
                0.8,
            )

        figure.create()
        for save_paths in makeSavePaths(
            f"distribution_{var}", *formats, outputdir=f"output_{hadron}"
        ):
            figure.save(save_paths)


distr_studies()
