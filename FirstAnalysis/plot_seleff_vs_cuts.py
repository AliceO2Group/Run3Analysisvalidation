#!/usr/bin/env python3

"""
file: plot_seleff_vs_cuts.py
brief: script to plot selection efficiencies vs cut obtained with HFSelOptimisation.cxx task
usage: python3 plot_seleff_vs_cuts.py CONFIG [--batch]
author: Fabrizio Grosa <fabrizio.grosa@cern.ch>, CERN
"""

import argparse
from typing import Dict

import yaml
from ROOT import (  # pylint: disable=import-error,no-name-in-module
    TCanvas,
    TFile,
    TLegend,
    gROOT,
    gStyle,
    kAzure,
    kBlack,
    kBlue,
    kFullCircle,
    kGreen,
    kMagenta,
    kOrange,
    kRed,
)


def set_hist_style(histo, color=kBlack, marker=kFullCircle, markersize=1):
    """
    Helper method to set histo graphic style
    """
    histo.SetLineColor(color)
    histo.SetMarkerColor(color)
    histo.SetMarkerStyle(marker)
    histo.SetMarkerSize(markersize)


gStyle.SetPadBottomMargin(0.15)
gStyle.SetPadLeftMargin(0.14)
gStyle.SetPadRightMargin(0.12)
gStyle.SetPadTickX(1)
gStyle.SetPadTickY(1)
gStyle.SetTitleSize(0.045, "xy")
gStyle.SetLabelSize(0.045, "y")
gStyle.SetLabelSize(0.055, "x")
gStyle.SetTitleOffset(1.4, "x")
gStyle.SetOptStat(0)

cand_types = ["2Prong", "3Prong"]
colors = {"Prompt": kRed + 1, "NonPrompt": kAzure + 4, "Bkg": kBlack}
colors_channel = {
    "D0ToPiK": kRed + 1,
    "JpsiToEE": kMagenta + 1,
    "2Prong": kBlack,
    "DPlusToPiKPi": kGreen + 2,
    "DsToPiKK": kOrange + 7,
    "LcToPKPi": kAzure + 4,
    "XicToPKPi": kBlue + 3,
    "3Prong": kBlack,
}
leg_orig_names = {"Prompt": "prompt", "NonPrompt": "non-prompt", "Bkg": "background"}
leg_channel_names = {
    "D0ToPiK": "D^{0} #rightarrow K^{#minus}#pi^{#plus}",
    "JpsiToEE": "J/#psi #rightarrow e^{#minus}e^{#plus}",
    "DPlusToPiKPi": "D^{#plus} #rightarrow K^{#minus}#pi^{#plus}#pi^{#plus}",
    "DsToPiKK": "D_{s}^{#plus} #rightarrow K^{#plus}K^{#minus}#pi^{#plus}",
    "LcToPKPi": "#Lambda_{c}^{#plus} #rightarrow pK^{#minus}#pi^{#plus}",
    "XicToPKPi": "#Xi_{c}^{#plus} #rightarrow pK^{#minus}#pi^{#plus}",
}

parser = argparse.ArgumentParser(description="Arguments")
parser.add_argument(
    "cfgfilename", metavar="text", default="config.yml", help="input yaml config file"
)
parser.add_argument("--batch", action="store_true", help="suppress video output")
args = parser.parse_args()

with open(args.cfgfilename, "r") as yml_file:
    cfg = yaml.safe_load(yml_file)

infile_names = {
    "Prompt": cfg["inputs"]["prompt"],
    "NonPrompt": cfg["inputs"]["nonprompt"],
    "Bkg": cfg["inputs"]["background"],
}

origins = cfg["origins"]
cands = {"2Prong": cfg["cands2Prong"], "3Prong": cfg["cands3Prong"]}
for cand_type in cand_types:
    cands[cand_type].insert(0, cand_type)
vars_to_plot = {"2Prong": cfg["vars2Prong"], "3Prong": cfg["vars3Prong"]}

if args.batch:
    gROOT.SetBatch(True)

hvar_vs_pt = {}  # type: Dict[str, Dict]
hvar = {}  # type: Dict[str, Dict]
hvar_perevent = {}  # type: Dict[str, Dict]
hvar_fracs = {}  # type: Dict[str, Dict]
hnorm_vs_pt = {}  # type: Dict[str, Dict]
for cand_type in cand_types:
    for cand in cands[cand_type]:
        (
            hvar_vs_pt[cand],
            hvar[cand],
            hvar_perevent[cand],
            hvar_fracs[cand],
            hnorm_vs_pt[cand],
        ) = ({} for _ in range(5))

        for iVar, var in enumerate(vars_to_plot[cand_type]):
            (
                hvar_vs_pt[cand][var],
                hvar[cand][var],
                hvar_perevent[cand][var],
                hvar_fracs[cand][var],
            ) = ({} for _ in range(4))

            for orig in origins:
                infile = TFile.Open(infile_names[orig])
                hvar_vs_pt[cand][var][orig] = infile.Get(
                    f"hf-sel-optimisation/h{orig}{var}VsPt{cand}"
                )
                if iVar == 0:
                    hnorm_vs_pt[cand][orig] = infile.Get(
                        f"hf-sel-optimisation/h{orig}VsPt{cand}"
                    )
                hEvents = infile.Get("hf-tag-sel-collisions/hEvents")
                hvar_vs_pt[cand][var][orig].SetDirectory(0)
                hnorm_vs_pt[cand][orig].SetDirectory(0)
                hEvents.SetDirectory(0)
                nEvents = hEvents.GetBinContent(2)

                hvar[cand][var][orig] = []
                hvar_perevent[cand][var][orig] = []
                hvar_fracs[cand][var][orig] = []
                nPtBins = hvar_vs_pt[cand][var][orig].GetXaxis().GetNbins()

                for iPtBin in range(1, nPtBins + 1):
                    hvar[cand][var][orig].append(
                        hvar_vs_pt[cand][var][orig].ProjectionY(
                            f"h{orig}{var}{cand}_pTbin{iPtBin}", iPtBin, iPtBin
                        )
                    )
                    hvar[cand][var][orig][iPtBin - 1].SetDirectory(0)
                    hvar[cand][var][orig][iPtBin - 1].GetXaxis().SetRangeUser(
                        1.0, hvar_vs_pt[cand][var][orig].GetYaxis().GetNbins()
                    )
                    hvar_fracs[cand][var][orig].append(
                        hvar[cand][var][orig][iPtBin - 1].Clone(
                            f"h{orig}{var}{cand}_Frac_pTbin{iPtBin}"
                        )
                    )
                    hvar_fracs[cand][var][orig][iPtBin - 1].SetDirectory(0)
                    hvar_fracs[cand][var][orig][iPtBin - 1].GetXaxis().SetRangeUser(
                        1.0, hvar_vs_pt[cand][var][orig].GetYaxis().GetNbins()
                    )
                    set_hist_style(
                        hvar[cand][var][orig][iPtBin - 1],
                        colors[orig],
                        kFullCircle,
                        1.0,
                    )
                    set_hist_style(
                        hvar_fracs[cand][var][orig][iPtBin - 1],
                        colors_channel[cand],
                        kFullCircle,
                        1.0,
                    )
                    hvar_perevent[cand][var][orig].append(
                        hvar[cand][var][orig][iPtBin - 1].Clone(
                            f"h{orig}{var}{cand}_perEvent_pTbin{iPtBin}"
                        )
                    )
                    hvar_perevent[cand][var][orig][iPtBin - 1].SetDirectory(0)
                    norm = hnorm_vs_pt[cand][orig].GetBinContent(iPtBin)
                    normCandType = hnorm_vs_pt[cand_type][orig].GetBinContent(iPtBin)
                    if norm == 0:
                        norm = 1
                    if normCandType == 0:
                        normCandType = 1
                    hvar[cand][var][orig][iPtBin - 1].Scale(1.0 / norm)
                    hvar_fracs[cand][var][orig][iPtBin - 1].Divide(
                        hvar[cand][var][orig][iPtBin - 1],
                        hvar[cand_type][var][orig][iPtBin - 1],
                        norm,
                        normCandType,
                        "",
                    )
                    hvar_perevent[cand][var][orig][iPtBin - 1].Scale(1.0 / nEvents)

# plots
leg_orig = TLegend(0.35, 0.2, 0.7, 0.35)
leg_orig.SetTextSize(0.045)
leg_orig.SetBorderSize(0)
leg_orig.SetFillStyle(0)

leg_channels = {}  # type: Dict[str, TLegend]

ceff = {}  # type: Dict[str, TCanvas]
ccand_perevent = {}  # type: Dict[str, TCanvas]
cfracs = {}  # type: Dict[str, TCanvas]
for cand_type in cand_types:
    leg_channels[cand_type] = TLegend(0.35, 0.2, 0.7, 0.45)
    leg_channels[cand_type].SetTextSize(0.045)
    leg_channels[cand_type].SetBorderSize(0)
    leg_channels[cand_type].SetFillStyle(0)
    for cand in cands[cand_type]:
        ceff[cand], ccand_perevent[cand] = ({} for _ in range(2))

        for iVar, var in enumerate(vars_to_plot[cand_type]):
            ceff[cand][var] = TCanvas(f"c{var}{cand}", "", 1200, 400)
            ccand_perevent[cand][var] = TCanvas(f"c{var}{cand}_perEvent", "", 1200, 400)

            nCutsTested = hvar[cand][var][origins[0]][-1].GetNbinsX()
            varTitle = hvar[cand][var][origins[0]][-1].GetXaxis().GetTitle()
            ceff[cand][var].Divide(nPtBins, 1)
            ccand_perevent[cand][var].Divide(nPtBins, 1)

            for iPtBin in range(nPtBins):
                # fill legend
                for orig in origins:
                    if iPtBin == 1 and leg_orig.GetNRows() < len(origins):
                        leg_orig.AddEntry(
                            hvar[cand][var][orig][iPtBin - 1],
                            leg_orig_names[orig],
                            "pl",
                        )

                ptMin = (
                    hvar_vs_pt[cand][var][origins[0]]
                    .GetXaxis()
                    .GetBinLowEdge(iPtBin + 1)
                )
                ptMax = (
                    hvar_vs_pt[cand][var][origins[0]]
                    .GetXaxis()
                    .GetBinUpEdge(iPtBin + 1)
                )
                h_frame = hvar[cand][var][origins[0]][iPtBin].Clone(
                    f"h_frame{var}{cand}_pTbin{iPtBin}"
                )
                h_frame.Reset()
                h_frame.SetTitle(
                    f"{ptMin}<#it{{p}}_{{T}}<{ptMax} GeV/#it{{c}};{varTitle};selection efficiency;"
                )
                h_frame.GetYaxis().SetRangeUser(
                    hvar[cand][var]["Bkg"][iPtBin].GetMinimum() / 10, 1.2
                )
                ceff[cand][var].cd(iPtBin + 1).SetLogy()
                h_frame.DrawCopy()
                for orig in origins:
                    hvar[cand][var][orig][iPtBin].DrawCopy("same")
                leg_orig.Draw()
                ccand_perevent[cand][var].cd(iPtBin + 1).SetLogy()
                h_frame.GetYaxis().SetRangeUser(
                    hvar_perevent[cand][var]["Bkg"][iPtBin].GetMinimum() / 10,
                    hvar_perevent[cand][var]["Bkg"][iPtBin].GetMaximum() * 20,
                )
                h_frame.GetYaxis().SetTitle("candidates/event")
                h_frame.DrawCopy()
                hvar_perevent[cand][var]["Bkg"][iPtBin].DrawCopy("same")
            ceff[cand][var].SaveAs(f"c{var}{cand}.pdf")
            ccand_perevent[cand][var].SaveAs(f"c{var}{cand}_perEvent.pdf")

    for iVar, var in enumerate(vars_to_plot[cand_type]):
        cfracs[var] = {}
        nCutsTested = hvar[cand][var][origins[0]][-1].GetNbinsX()
        varTitle = hvar[cand][var][origins[0]][-1].GetXaxis().GetTitle()
        for orig in origins:
            cfracs[var][orig] = TCanvas(f"c{orig}{var}{cand_type}_Fracs", "", 1200, 400)
            cfracs[var][orig].Divide(nPtBins, 1)
            for iPtBin in range(nPtBins):
                ptMin = (
                    hvar_vs_pt[cand_type][var][orig]
                    .GetXaxis()
                    .GetBinLowEdge(iPtBin + 1)
                )
                ptMax = (
                    hvar_vs_pt[cand_type][var][orig].GetXaxis().GetBinUpEdge(iPtBin + 1)
                )
                h_frame = hvar_fracs[cand][var][origins[0]][iPtBin].Clone(
                    f"h_frame_frac{var}{cand}_pTbin{iPtBin}"
                )
                h_frame.Reset()
                cfracs[var][orig].cd(iPtBin + 1).SetLogy()
                h_frame.GetYaxis().SetRangeUser(1.0e-3, 1.2)
                h_frame.GetYaxis().SetTitle("fraction")
                h_frame.DrawCopy()
                for cand in cands[cand_type]:
                    if cand != cand_type:
                        n_rows_leg = leg_channels[cand_type].GetNRows()
                        if iPtBin == 1 and n_rows_leg < len(cands[cand_type]) - 1:
                            leg_channels[cand_type].AddEntry(
                                hvar_fracs[cand][var][orig][iPtBin],
                                leg_channel_names[cand],
                                "pl",
                            )
                        hvar_fracs[cand][var][orig][iPtBin].DrawCopy("same")
                leg_channels[cand_type].Draw()
            cfracs[var][orig].SaveAs(f"c{cand_type}{orig}{var}_Frac.pdf")

if not args.batch:
    input("Press enter to exit")
