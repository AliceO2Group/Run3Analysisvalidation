#!/usr/bin/env python3

'''
Copyright CERN and copyright holders of ALICE O2. This software is
distributed under the terms of the GNU General Public License v3 (GPL
Version 3), copied verbatim in the file "COPYING".

See http://alice-o2.web.cern.ch/license for full licensing information.

In applying this license CERN does not waive the privileges and immunities
granted to it by virtue of its status as an Intergovernmental Organization
or submit itself to any jurisdiction.

file: HfAnalysisUtils.py
brief: script with miscellanea utils methods for the HF analyses
author: Fabrizio Grosa <fabrizio.grosa@cern.ch>, CERN
'''

import numpy as np

def ComputeCrossSection(rawY, uncRawY, frac, effTimesAcc,
                        deltaPt, deltaY, sigmaMB, nEv, BR, methodFrac='Nb'):
  '''
  Method to compute cross section and its statistical uncertainty
  Only the statistical uncertainty on the raw yield and prompt (non-prompt)
  fraction are considered (the others are systematics)

  Parameters
  ----------
  - rawY: raw yield
  - uncRawY: raw-yield statistical uncertainty
  - frac: either prompt or non-prompt fraction
  - effTimesAcc: efficiency times acceptance for prompt or non-prompt
  - deltaPt: pT interval
  - deltaY: Y interval
  - sigmaMB: hadronic cross section for MB
  - nEv: number of events
  - BR: branching ratio of the decay channel
  - methodFrac: method used to compute frac needed to propoer compute uncertainty

  Returns
  ----------
  - crossSection: cross section
  - crossSecUnc: cross-section statistical uncertainty
  '''

  crossSection = rawY * frac * sigmaMB / (2 * deltaPt * deltaY * effTimesAcc * nEv * BR)
  if methodFrac == 'Nb':
    crossSecUnc = uncRawY / (rawY * frac) * crossSection
  else:
    crossSecUnc = uncRawY / rawY * crossSection

  return crossSection, crossSecUnc  


# pylint: disable=too-many-branches
def ComputeFractionFc(accEffPrompt, accEffFD, crossSecPrompt, crossSecFD, raaPrompt=1., raaFD=1.):
  '''
  Method to get fraction of prompt / FD fraction with fc method

  Parameters
  ----------
  - accEffPrompt: efficiency times acceptance of prompt D
  - accEffFD: efficiency times acceptance of non-prompt D
  - crossSecPrompt: list of production cross sections (cent, min, max) of prompt D in pp collisions from theory
  - crossSecFD: list of production cross sections (cent, min, max) of non-prompt D in pp collisions from theory
  - raaPrompt: list of nuclear modification factors (cent, min, max) of prompt D from theory
  - raaFD: list of nuclear modification factors of (cent, min, max) non-prompt D from theory

  Returns
  ----------
  - fracPrompt: list of fraction of prompt D (central, min, max)
  - fracFD: list of fraction of non-prompt D (central, min, max)
  '''

  if not isinstance(crossSecPrompt, list) and isinstance(crossSecPrompt, float):
    crossSecPrompt = [crossSecPrompt]
  if not isinstance(crossSecFD, list) and isinstance(crossSecFD, float):
    crossSecFD = [crossSecFD]
  if not isinstance(raaPrompt, list) and isinstance(raaPrompt, float):
    raaPrompt = [raaPrompt]
  if not isinstance(raaFD, list) and isinstance(raaFD, float):
    raaFD = [raaFD]

  fracPrompt, fracFD = [], []
  if accEffPrompt == 0:
    fracFDCent = 1.
    fracPromptCent = 0.
    fracPrompt = [fracPromptCent, fracPromptCent, fracPromptCent]
    fracFD = [fracFDCent, fracFDCent, fracFDCent]
    return fracPrompt, fracFD
  if accEffFD == 0:
    fracFDCent = 0.
    fracPromptCent = 1.
    fracPrompt = [fracPromptCent, fracPromptCent, fracPromptCent]
    fracFD = [fracFDCent, fracFDCent, fracFDCent]
    return fracPrompt, fracFD

  for iSigma, (sigmaP, sigmaF) in enumerate(zip(crossSecPrompt, crossSecFD)):
    for iRaa, (raaP, raaF) in enumerate(zip(raaPrompt, raaFD)):
      if iSigma == 0 and iRaa == 0:
        fracPromptCent = 1. / (1 + accEffFD / accEffPrompt * sigmaF / sigmaP * raaF / raaP)
        fracFDCent = 1. / (1 + accEffPrompt / accEffFD * sigmaP / sigmaF * raaP / raaF)
      else:
        fracPrompt.append(1. / (1 + accEffFD / accEffPrompt * sigmaF / sigmaP * raaF / raaP))
        fracFD.append(1. / (1 + accEffPrompt / accEffFD * sigmaP / sigmaF * raaP / raaF))

  if fracPrompt and fracFD:
    fracPrompt.sort()
    fracFD.sort()
    fracPrompt = [fracPromptCent, fracPrompt[0], fracPrompt[-1]]
    fracFD = [fracFDCent, fracFD[0], fracFD[-1]]
  else:
    fracPrompt = [fracPromptCent, fracPromptCent, fracPromptCent]
    fracFD = [fracFDCent, fracFDCent, fracFDCent]

  return fracPrompt, fracFD


# pylint: disable=too-many-arguments, too-many-branches
def ComputeFractionNb(rawYield, accEffSame, accEffOther, crossSec, deltaPt, deltaY, BR, nEvents,
                      sigmaMB, raaRatio=1., taa=1.):
  '''
  Method to get fraction of prompt / FD fraction with Nb method

  Parameters
  ----------
  - accEffSame: efficiency times acceptance of prompt (non-prompt) D
  - accEffOther: efficiency times acceptance of non-prompt (prompt) D
  - crossSec: list of production cross sections (cent, min, max) of non-prompt (prompt)
    D in pp collisions from theory
  - deltaPt: width of pT interval
  - deltaY: width of Y interval
  - BR: branching ratio for the chosen decay channel
  - nEvents: number of events corresponding to the raw yields
  - sigmaMB: MB cross section
  - raaRatio: list of D nuclear modification factor ratios
    non-prompt / prompt (prompt / non-prompt) (cent, min, max) (=1 in case of pp)
  - taa: average nuclear overlap function (=1 in case of pp)

  Returns
  ----------
  - frac: list of fraction of prompt (non-prompt) D (central, min, max)
  '''

  if not isinstance(crossSec, list) and isinstance(crossSec, float):
    crossSec = [crossSec]

  if not isinstance(raaRatio, list) and isinstance(raaRatio, float):
    raaRatio = [raaRatio]

  frac = []
  for iSigma, sigma in enumerate(crossSec):
    for iRaaRatio, raaRat in enumerate(raaRatio):
      raaOther = 1.
      if iSigma == 0 and iRaaRatio == 0:
        if raaRat == 1. and taa == 1.: # pp
          fracCent = 1 - sigma * deltaPt * deltaY * accEffOther * BR * nEvents * 2 / rawYield / sigmaMB
        else: # p-Pb or Pb-Pb: iterative evaluation of Raa needed
          deltaRaa = 1.
          while deltaRaa > 1.e-3:
            fracCent = 1 - taa * raaRat * raaOther * sigma * deltaPt * deltaY * accEffOther * BR * nEvents * 2 / rawYield
            raaOtherOld = raaOther
            raaOther = fracCent * rawYield * sigmaMB / (2 * accEffSame * deltaPt * deltaY * BR * nEvents)
            deltaRaa = abs((raaOther-raaOtherOld) / raaOther)
      else:
        if raaRat == 1. and taa == 1.: # pp
          frac.append(1 - sigma * deltaPt * deltaY * accEffOther * BR * nEvents * 2 / rawYield / sigmaMB)
        else:
          deltaRaa = 1.
          fracTmp = 1.
          while deltaRaa > 1.e-3:
            fracTmp = 1 - taa * raaRat * raaOther * sigma * deltaPt * deltaY * accEffOther * BR * nEvents * 2 / rawYield
            raaOtherOld = raaOther
            raaOther = fracTmp * rawYield * sigmaMB / (2 * accEffSame * deltaPt * deltaY * BR * nEvents)
            deltaRaa = abs((raaOther-raaOtherOld) / raaOther)
          frac.append(fracTmp)

  if frac:
    frac.sort()
    frac = [fracCent, frac[0], frac[-1]]
  else:
    frac = [fracCent, fracCent, fracCent]

  return frac


def GetHistoBinLimits(histo):
  '''
  Method to retrieve bin limits of ROOT.TH1 

  Parameters
  ----------
  - histo: ROOT.TH1

  Returns
  ----------
  - binLims: numpy array of bin limits
  '''

  if np.array(histo.GetXaxis().GetXbins(), 'd').any(): # variable binning
    binLims = np.array(histo.GetXaxis().GetXbins(), 'd')
  else: # constant binning
      nLims = histo.GetNbinsX() + 1
      lowEdge = histo.GetBinLowEdge(1)
      binWidth = histo.GetBinWidth(1)
      binLims = np.array([lowEdge + iBin * binWidth for iBin in range(nLims)], 'd')

  return binLims