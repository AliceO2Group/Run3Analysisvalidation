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
brief: script for computation of pT-differential yields (cross sections)
usage: python3 HfPtSpectrum.py CONFIG
author: Fabrizio Grosa <fabrizio.grosa@cern.ch>, CERN
'''


import sys
import os
import argparse
import numpy as np
import yaml
from hfplot.plot_spec_root import ROOTFigure
from hfplot.style import StyleObject1D
from ROOT import TFile, TH1F, TH1, TGraphAsymmErrors # pylint: disable=import-error,no-name-in-module
from ROOT import gROOT, kAzure, kFullCircle # pylint: disable=import-error,no-name-in-module
from HfAnalysisUtils import ComputeCrossSection, ComputeFractionFc, ComputeFractionNb, GetHistoBinLimits


def LoadInputs(inputCfg):
  '''
  Helper method to load inputs

  Parameters
  ----------
  - inputCfg: dictionary from yaml config file (see example https://github.com/AliceO2Group/Run3Analysisvalidation/blob/master/FirstAnalysis/config/config_ptspectrum_Dplus_pp5TeV.yml)

  Returns
  ----------
  - histos: dictionary with input histos, keys (rawyields, acceffp, acceffnp)
  - norm: dictionary with normalisations, keys (BR, events, sigmaMB)
  '''

  observable = inputCfg['observable']
  if observable not in ['dsigmadpt', 'dNdpt']:
    print(f'\033[91mERROR: observable {observable} not supported. Exit\033[0m')
    sys.exit()

  channel = inputCfg['channel']
  if channel not in ['D0toKpi', 'DplustoKpipi', 'DstoKpipi', 'DstartoD0pi', 'LctopKpi', 'LctopK0S']:
    print(f'\033[91mERROR: channel {channel} not supported. Exit\033[0m')
    sys.exit()

  system = inputCfg['system']
  if system not in ['pp', 'pPb', 'PbPb']:
    print(f'\033[91mERROR: channel {channel} not supported. Exit\033[0m')
    sys.exit()
  if system in ['pPb', 'PbPb'] and observable == 'dsigmadpt':
    print('\033[93mWARNING: switching from dsigmadpt to dNdpt\033[0m')

  energy = inputCfg['energy']
  if energy not in ['5TeV', '13TeV']:
    print(f'\033[91mERROR: energy {energy} not supported. Exit\033[0m')
    sys.exit()

  fracMethod = inputCfg['fraction']
  if fracMethod not in ['Nb', 'fc']:
    print(f'\033[91mERROR: method to subtract nonprompt {fracMethod} not supported. Exit\033[0m')
    sys.exit()

  rawYieldFileName = inputCfg['rawyield']['filename']
  rawYieldHistoName = inputCfg['rawyield']['rawyieldhist']
  normHistoName = inputCfg['rawyield']['normhist']

  effFileName = inputCfg['acceff']['filename']
  effPromptHistName = inputCfg['acceff']['prompthist']
  effNonPromptHistName = inputCfg['acceff']['nonprompthist']

  predFileName = inputCfg['FONLL']

  # load histos from root files
  histos = {}
  inFileRawYields = TFile.Open(rawYieldFileName)
  histos['rawyields'] = inFileRawYields.Get(rawYieldHistoName)
  if not histos['rawyields']:
    print(f'\033[91mERROR: raw-yield histo {rawYieldHistoName} not found in {rawYieldFileName}. Exit\033[0m')
    sys.exit()
  histos['rawyields'].SetDirectory(0)
  hNevents = inFileRawYields.Get(normHistoName)
  if not hNevents:
    print(f'\033[91mERROR: normalisation histo {normHistoName} not found in {rawYieldFileName}. Exit\033[0m')
    sys.exit()
  hNevents.SetDirectory(0)
  inFileRawYields.Close()

  inFileEff = TFile.Open(effFileName)
  histos['acceffp'] = inFileEff.Get(effPromptHistName)
  if not histos['acceffp']:
    print(f'\033[91mERROR: prompt (acc x eff) histo {effPromptHistName} not found in {effFileName}. Exit\033[0m')
    sys.exit()
  histos['acceffp'].SetDirectory(0)
  histos['acceffnp'] = inFileEff.Get(effNonPromptHistName)
  if not histos['acceffnp']:
    print(f'\033[91mERROR: nonprompt (acc x eff) histo {effPromptHistName} not found in {effFileName}. Exit\033[0m')
    sys.exit()
  histos['acceffnp'].SetDirectory(0)
  inFileEff.Close()

  FONLLhistoName = {'D0toKpi': 'hD0Kpi', 'DplustoKpipi': 'hDpluskpipi', 'DstoKKpi': 'hDsPhipitoKkpi',
                    'DstartoD0pi': 'hDstarD0pi', 'LctopKpi': 'hLcpkpi', 'LctopK0S': 'hLcK0sp'}
  histos['FONLL'] = {'prompt': {}, 'nonprompt': {}}
  inFileFONLL = TFile.Open(predFileName)
  for pred in ('central', 'min', 'max'):
    histos['FONLL']['nonprompt'][pred] = inFileFONLL.Get(f'{FONLLhistoName[channel]}fromBpred_{pred}_corr')
    histos['FONLL']['nonprompt'][pred].SetDirectory(0)
    if fracMethod == 'fc':
      histos['FONLL']['prompt'][pred] = inFileFONLL.Get(f'{FONLLhistoName[channel]}pred_{pred}')
      histos['FONLL']['prompt'][pred].SetDirectory(0)
  inFileFONLL.Close()

  # load normalisation info from common database
  norm = {}
  with open('config/norm_database.yml', 'r') as ymlNormDB:
    normDB = yaml.load(ymlNormDB, yaml.FullLoader)
  norm['BR'] = normDB['BR'][channel]['value']
  norm['events'] = hNevents.GetBinContent(1)
  norm['sigmaMB'] = normDB['sigma']['Run2'][system][energy] if observable == 'dsigmadpt' else 1.

  return histos, norm


def Main():
  '''
  Main function
  '''

  parser = argparse.ArgumentParser(description='Arguments to pass')
  parser.add_argument('configFileName', metavar='text',
                      default='config_Dplus_pp5TeV.yml',
                      help='input yaml config file')
  parser.add_argument('--batch', action='store_true',
                      default=False,
                      help='suppress video output')
  args = parser.parse_args()

  if args.batch:
    gROOT.SetBatch(True)

  # load info from config file
  with open(args.configFileName, 'r') as ymlConfigFile:
    cfg = yaml.load(ymlConfigFile, yaml.FullLoader)
  fracMethod = cfg['fraction']

  histos, norm = LoadInputs(cfg)

  # consistency check of bins
  ptLims = {}
  for histo in ['rawyields', 'acceffp', 'acceffnp']:
    ptLims[histo] = GetHistoBinLimits(histos[histo])
    if histo != 'rawyields' and not np.equal(ptLims[histo], ptLims['rawyields']).all():
      print(f'\033[91mERROR: histo binning not consistent. Exit\033[0m')
      sys.exit()

  # compute cross section
  axisTitCross = 'd#sigma/d#it{p}_{T} (pb GeV^{-1} #it{c})'
  axisTitCrossTimesBR = 'd#sigma/d#it{p}_{T} #times BR (pb GeV^{-1} #it{c})'
  axisTitPt = '#it{p}_{T} (GeV/#it{c})'
  axisTitFprompt = '#if{f}_{prompt}'
  gFraction = TGraphAsymmErrors(0)
  gFraction.SetNameTitle('gFraction', f';{axisTitPt};{axisTitFprompt}')
  hPtSpectrum = TH1F('hPtSpectrum', f';{axisTitPt};{axisTitCross}',
                     len(ptLims['rawyields'])-1, ptLims['rawyields'])
  hPtSpectrumWoBR = TH1F('hPtSpectrumWoBR', f';{axisTitPt};{axisTitCrossTimesBR}',
                         len(ptLims['rawyields'])-1, ptLims['rawyields'])

  for iPt, (ptMin, ptMax) in enumerate(zip(ptLims['rawyields'][:-1], ptLims['rawyields'][1:])):
    ptCent = (ptMax + ptMin) / 2
    ptDelta = (ptMax - ptMin)
    rawY = histos['rawyields'].GetBinContent(iPt+1)
    uncRawY = histos['rawyields'].GetBinError(iPt+1)
    effTimesAccPrompt = histos['acceffp'].GetBinContent(iPt+1)
    effTimesAccNonPrompt = histos['acceffnp'].GetBinContent(iPt+1)
    ptMinFONLL = histos['FONLL']['nonprompt']['central'].GetXaxis().FindBin(ptMin*1.0001)
    ptMaxFONLL = histos['FONLL']['nonprompt']['central'].GetXaxis().FindBin(ptMax*0.9999)
    crossSecNonPromptFONLL = [histos['FONLL']['nonprompt'][pred].Integral(ptMinFONLL, ptMaxFONLL, 'width') /
                              (ptMax - ptMin) for pred in histos['FONLL']['nonprompt']]

    # compute prompt fraction
    if fracMethod == 'Nb':
      frac = ComputeFractionNb(rawY, effTimesAccPrompt, effTimesAccNonPrompt, crossSecNonPromptFONLL, ptDelta, 1.,
                               1., norm['events'], norm['sigmaMB']) # BR already included in FONLL prediction
    elif fracMethod == 'fc':
      crossSecPromptFONLL = [histos['FONLL']['prompt'][pred].Integral(ptMinFONLL, ptMaxFONLL, 'width') /
                             (ptMax - ptMin) for pred in histos['FONLL']['prompt']]
      frac = ComputeFractionFc(effTimesAccPrompt, effTimesAccNonPrompt, crossSecPromptFONLL, crossSecNonPromptFONLL)

    # compute cross section times BR
    crossSec, crossSecUnc = ComputeCrossSection(rawY, uncRawY, frac[0], effTimesAccPrompt, ptMax-ptMin, 1.,
                                                norm['sigmaMB'], norm['events'], 1., fracMethod)

    hPtSpectrum.SetBinContent(iPt+1, crossSec / norm['BR'])
    hPtSpectrum.SetBinError(iPt+1, crossSecUnc / norm['BR'])
    hPtSpectrumWoBR.SetBinContent(iPt+1, crossSec)
    hPtSpectrumWoBR.SetBinError(iPt+1, crossSecUnc)
    gFraction.SetPoint(iPt, ptCent, frac[0])
    gFraction.SetPointError(iPt, ptDelta/2, ptDelta/2, frac[0]-frac[1], frac[2]-frac[0])

  # create plots
  styleHist = StyleObject1D()
  styleHist.markercolor = kAzure+4
  styleHist.markerstyle = kFullCircle
  styleHist.markersize = 1
  styleHist.linecolor = kAzure+4
  styleHist.linewidth = 2
  styleHist.draw_options = 'P'

  figCrossSec = ROOTFigure(1, 1, column_margin=(0.14, 0.035), row_margin=(0.1, 0.035), size=(600, 800))
  figCrossSec.axes(label_size=0.025, title_size=0.030)
  figCrossSec.axes('x', title=axisTitPt, title_offset=1.5)
  figCrossSec.axes('y', title=axisTitCrossTimesBR, title_offset=1.8)
  figCrossSec.define_plot(0, 0, y_log=True)
  figCrossSec.add_object(hPtSpectrumWoBR, style=styleHist)
  figCrossSec.create()

  outputDir = cfg['output']['directory']
  if not os.path.exists(outputDir):
      os.makedirs(outputDir)
  figCrossSec.save(os.path.join(outputDir, f'{cfg["output"]["filename"]}.pdf'))

  # save output file
  outFile = TFile(os.path.join(outputDir, f'{cfg["output"]["filename"]}.root'), 'recreate')
  hPtSpectrum.Write()
  hPtSpectrumWoBR.Write()
  gFraction.Write()
  for hist in histos:
    if isinstance(histos[hist], TH1):
      histos[hist].Write()
    else:
      for flav in histos[hist]:
        for pred in histos[hist][flav]:
          histos[hist][flav][pred].Write()
  outFile.Close()

# call main function
Main()
