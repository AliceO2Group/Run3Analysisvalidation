#!/usr/bin/env python3

'''
Copyright CERN and copyright holders of ALICE O2. This software is
distributed under the terms of the GNU General Public License v3 (GPL
Version 3), copied verbatim in the file "COPYING".

See http://alice-o2.web.cern.ch/license for full licensing information.

In applying this license CERN does not waive the privileges and immunities
granted to it by virtue of its status as an Intergovernmental Organization
or submit itself to any jurisdiction.

file: HfPtSpectrum.py
brief: script to plot selection efficiencies vs cut obtained with HFSelOptimisation.cxx task
usage: python3 HfPtSpectrum.py CONFIG [--batch]
author: Fabrizio Grosa <fabrizio.grosa@cern.ch>, CERN
'''

import argparse
import yaml
from ROOT import TFile, TH1F, TCanvas, TLegend
from ROOT import gROOT, gStyle, kBlack, kRed, kAzure, kMagenta, kGreen, kOrange, kBlue, kFullCircle


def SetHistStyle(histo, color=kBlack, marker=kFullCircle, markersize=1):
    '''
    Helper method to set histo graphic style
    '''
    histo.SetLineColor(color)
    histo.SetMarkerColor(color)
    histo.SetMarkerStyle(marker)
    histo.SetMarkerSize(markersize)


def SetBinLabels(histo, labels, axis='x'):
    '''
    Helper method to set bin labels

    Parameters
    ----------
    - histo: ROOT.TH1
    - labels: list of string
    - axis: string that identifies the axis on which setting the lables (x or y)

    '''

    if axis == 'x':
        nBins = histo.GetXaxis().GetNbins()
    elif axis == 'y':
        nBins = histo.GetYaxis().GetNbins()
    if len(labels) != nBins:
        print('WARNING: number of labels does not match with number of bins, not setting labels')
        return

    for iBin in range(1, nBins+1):
        if axis == 'x':
            histo.GetXaxis().SetBinLabel(iBin, f'{labels[iBin-1]}')
        elif axis == 'y':
            histo.GetYaxis().SetBinLabel(iBin, f'{labels[iBin-1]}')


gStyle.SetPadBottomMargin(0.15)
gStyle.SetPadLeftMargin(0.14)
gStyle.SetPadTickX(1)
gStyle.SetPadTickY(1)
gStyle.SetTitleSize(0.045, 'xy')
gStyle.SetLabelSize(0.045, 'xy')
gStyle.SetTitleOffset(1.4, 'x')
gStyle.SetOptStat(0)

candTypes = ['2Prong', '3Prong']
origins = ['Prompt', 'NonPrompt', 'Bkg']
colors = {'Prompt': kRed+1, 'NonPrompt': kAzure+4, 'Bkg': kBlack}
colorsChannel = {
    'D0ToPiK': kRed+1,
    'JpsiToEE': kMagenta+1,
    '2Prong': kBlack,
    'DPlusToPiKPi': kGreen+2,
    'DsToPiKK': kOrange+7,
    'LcToPKPi': kAzure+4,
    'XicToPKPi': kBlue+3,
    '3Prong': kBlack
}
legOrigNames = {'Prompt': 'prompt', 'NonPrompt': 'non-prompt', 'Bkg': 'background'}
legChannelNames = {
    'D0ToPiK': 'D^{0} #rightarrow K^{#minus}#pi^{#plus}',
    'JpsiToEE': 'J/#psi #rightarrow e^{#minus}e^{#plus}',
    'DPlusToPiKPi': 'D^{#plus} #rightarrow K^{#minus}#pi^{#plus}#pi^{#plus}',
    'DsToPiKK': 'D_{s}^{#plus} #rightarrow K^{#plus}K^{#minus}#pi^{#plus}',
    'LcToPKPi': '#Lambda_{c}^{#plus} #rightarrow pK^{#minus}#pi^{#plus}',
    'XicToPKPi': '#Xi_{c}^{#plus} #rightarrow pK^{#minus}#pi^{#plus}'
}

parser = argparse.ArgumentParser(description='Arguments')
parser.add_argument('cfgFileName', metavar='text', default='config.yml', help='input yaml config file')
parser.add_argument('--batch', action='store_true', help='suppress video output')
args = parser.parse_args()

with open(args.cfgFileName, 'r') as ymlFile:
    cfg = yaml.load(ymlFile, yaml.FullLoader)

labels = cfg['labels']
inFileNames = {
    'Prompt': cfg['inputs']['signal'],
    'NonPrompt': cfg['inputs']['signal'],
    'Bkg': cfg['inputs']['background']
}

cands = {'2Prong': cfg['cands2Prong'], '3Prong': cfg['cands3Prong']}
for candType in candTypes:
    cands[candType].insert(0, candType)
vars = {'2Prong': cfg['vars2Prong'], '3Prong': cfg['vars3Prong']}

if args.batch:
    gROOT.SetBatch(True)

hVarVsPt, hVar, hVarPerEvent, hVarFracs, hNormVsPt = ({} for _ in range(5))
for candType in candTypes:
    for cand in cands[candType]:
        hVarVsPt[cand], hVar[cand], hVarPerEvent[cand], hVarFracs[cand], hNormVsPt[cand] \
            = ({} for _ in range(5))

        for iVar, var in enumerate(vars[candType]):
            hVarVsPt[cand][var], hVar[cand][var], hVarPerEvent[cand][var], \
                hVarFracs[cand][var] = ({} for _ in range(4))

            for orig in origins:
                inFile = TFile.Open(inFileNames[orig])
                hVarVsPt[cand][var][orig] = inFile.Get(f'hf-sel-optimisation/h{orig}{var}VsPt{cand}')
                if iVar == 0:
                    hNormVsPt[cand][orig] = inFile.Get(f'hf-sel-optimisation/h{orig}VsPt{cand}')
                hEvents = inFile.Get('hf-tag-sel-collisions/hEvents')
                hVarVsPt[cand][var][orig].SetDirectory(0)
                hNormVsPt[cand][orig].SetDirectory(0)
                hEvents.SetDirectory(0)
                nEvents = hEvents.GetBinContent(2)

                hVar[cand][var][orig] = []
                hVarPerEvent[cand][var][orig] = []
                hVarFracs[cand][var][orig] = []
                nPtBins = hVarVsPt[cand][var][orig].GetXaxis().GetNbins()

                for iPtBin in range(1, nPtBins+1):
                    hVar[cand][var][orig].append(hVarVsPt[cand][var][orig].ProjectionY(
                        f'h{orig}{var}{cand}_pTbin{iPtBin}', iPtBin, iPtBin))
                    hVar[cand][var][orig][iPtBin-1].SetDirectory(0)
                    hVarFracs[cand][var][orig].append(hVar[cand][var][orig][iPtBin-1].Clone(
                        f'h{orig}{var}{cand}_Frac_pTbin{iPtBin}'))
                    hVarFracs[cand][var][orig][iPtBin-1].SetDirectory(0)
                    SetHistStyle(hVar[cand][var][orig][iPtBin-1], colors[orig], kFullCircle, 1.)
                    SetHistStyle(hVarFracs[cand][var][orig][iPtBin-1], colorsChannel[cand], kFullCircle, 1.)
                    hVarPerEvent[cand][var][orig].append(
                        hVar[cand][var][orig][iPtBin-1].Clone(f'h{orig}{var}{cand}_perEvent_pTbin{iPtBin}'))
                    hVarPerEvent[cand][var][orig][iPtBin-1].SetDirectory(0)
                    norm = hNormVsPt[cand][orig].GetBinContent(iPtBin)
                    normCandType = hNormVsPt[candType][orig].GetBinContent(iPtBin)
                    if norm == 0:
                        norm = 1
                    if normCandType == 0:
                        normCandType = 1
                    hVar[cand][var][orig][iPtBin-1].Scale(1./norm)
                    hVarFracs[cand][var][orig][iPtBin-1].Divide(hVar[cand][var][orig][iPtBin-1],
                                                                hVar[candType][var][orig][iPtBin-1],
                                                                norm, normCandType, '')
                    hVarPerEvent[cand][var][orig][iPtBin-1].Scale(1./nEvents)

# plots
legOrig = TLegend(0.35, 0.2, 0.7, 0.35)
legOrig.SetTextSize(0.045)
legOrig.SetBorderSize(0)
legOrig.SetFillStyle(0)

legChannels = {}

cEff, cCandPerEvent, cFracs = ({} for _ in range(3))
for candType in candTypes:
    legChannels[candType] = TLegend(0.35, 0.2, 0.7, 0.45)
    legChannels[candType].SetTextSize(0.045)
    legChannels[candType].SetBorderSize(0)
    legChannels[candType].SetFillStyle(0)
    for cand in cands[candType]:
        cEff[cand], cCandPerEvent[cand] = ({} for _ in range(2))

        for iVar, var in enumerate(vars[candType]):
            cEff[cand][var] = TCanvas(f'c{var}{cand}', '', 1200, 400)
            cCandPerEvent[cand][var] = TCanvas(f'c{var}{cand}_perEvent', '', 1200, 400)

            nCutsTested = hVar[cand][var]['Prompt'][-1].GetNbinsX()
            varTitle = hVar[cand][var]['Prompt'][-1].GetXaxis().GetTitle()
            cEff[cand][var].Divide(nPtBins, 1)
            cCandPerEvent[cand][var].Divide(nPtBins, 1)

            for iPtBin in range(nPtBins):
                # fill legend
                for orig in origins:
                    if iPtBin == 1 and legOrig.GetNRows() < len(origins):
                        legOrig.AddEntry(hVar[cand][var][orig][iPtBin-1], legOrigNames[orig], 'pl')

                ptMin = hVarVsPt[cand][var]['Prompt'].GetXaxis().GetBinLowEdge(iPtBin+1)
                ptMax = hVarVsPt[cand][var]['Prompt'].GetXaxis().GetBinUpEdge(iPtBin+1)
                hFrame = TH1F(f'hFrame{var}{cand}_pTbin{iPtBin}',
                              f'{ptMin}<#it{{p}}_{{T}}<{ptMax} GeV/#it{{c}};{varTitle};selection efficiency;',
                              nCutsTested, 0.5, 0.5+nCutsTested)
                SetBinLabels(hFrame, labels[var])
                cEff[cand][var].cd(iPtBin+1).SetLogy()
                hFrame.GetYaxis().SetRangeUser(1.e-3, 1.2)
                hFrame.DrawCopy()
                for orig in origins:
                    hVar[cand][var][orig][iPtBin].DrawCopy('same')
                legOrig.Draw()
                cCandPerEvent[cand][var].cd(iPtBin+1).SetLogy()
                hFrame.GetYaxis().SetRangeUser(hVarPerEvent[cand][var]['Bkg'][iPtBin].GetMinimum()/10,
                                               hVarPerEvent[cand][var]['Bkg'][iPtBin].GetMaximum()*5)
                hFrame.GetYaxis().SetTitle('candidates/event')
                hFrame.DrawCopy()
                hVarPerEvent[cand][var]['Bkg'][iPtBin].DrawCopy('same')
            cEff[cand][var].SaveAs(f'c{var}{cand}.pdf')
            cCandPerEvent[cand][var].SaveAs(f'c{var}{cand}_perEvent.pdf')

    for iVar, var in enumerate(vars[candType]):
        cFracs[var] = {}
        nCutsTested = hVar[cand][var]['Prompt'][-1].GetNbinsX()
        varTitle = hVar[cand][var]['Prompt'][-1].GetXaxis().GetTitle()
        for orig in origins:
            cFracs[var][orig] = TCanvas(f'c{orig}{var}{candType}_Fracs', '', 1200, 400)
            cFracs[var][orig].Divide(nPtBins, 1)
            for iPtBin in range(nPtBins):
                ptMin = hVarVsPt[candType][var][orig].GetXaxis().GetBinLowEdge(iPtBin+1)
                ptMax = hVarVsPt[candType][var][orig].GetXaxis().GetBinUpEdge(iPtBin+1)
                hFrame = TH1F(f'hFrameFrac{orig}{var}{candType}_pTbin{iPtBin}',
                              f'{ptMin}<#it{{p}}_{{T}}<{ptMax} GeV/#it{{c}};{varTitle};fraction;',
                              nCutsTested, 0.5, 0.5+nCutsTested)
                SetBinLabels(hFrame, labels[var])
                cFracs[var][orig].cd(iPtBin+1).SetLogy()
                hFrame.GetYaxis().SetRangeUser(1.e-3, 1.2)
                hFrame.GetYaxis().SetTitle('fraction')
                hFrame.DrawCopy()
                for cand in cands[candType]:
                    if cand != candType:
                        if iPtBin == 1 and legChannels[candType].GetNRows() < len(cands[candType])-1:
                            legChannels[candType].AddEntry(hVarFracs[cand][var][orig][iPtBin],
                                                           legChannelNames[cand], 'pl')
                        hVarFracs[cand][var][orig][iPtBin].DrawCopy('same')
                legChannels[candType].Draw()
            cFracs[var][orig].SaveAs(f'c{candType}{orig}{var}_Frac.pdf')

input('Press enter to exit')
