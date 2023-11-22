// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

// Comparison of AliPhysics and O2 histograms

#include "../exec/utilitiesValidation.h"

Int_t Compare(TString pathFileO2 = "AnalysisResults_O2.root", TString pathFileAli = "AnalysisResults_ALI.root", TString options = "", bool doRatio = false)
{
  TString pathListAli = "ChJetSpectraAliAnalysisTaskEmcalJetValidation/AliAnalysisTaskEmcalJetValidation";

  // Histogram specification: axis label, AliPhysics name, O2Physics path/name, rebin, log scale histogram, log scale ratio, projection axis

  VecSpecHis vecHisEvents;

  VecSpecHis vecHisJets;
  AddHistogram(vecHisJets, "#it{p}_{T, ch jet} (GeV/#it{c})", "jetPt", "jet-validation-track-collision-qa/jetPt", 1, 1, 0);
  AddHistogram(vecHisJets, "#it{#varphi}_{jet}", "jetPhi", "jet-validation-track-collision-qa/jetPhi", 1, 0, 0);
  AddHistogram(vecHisJets, "#it{#eta}_{jet}", "jetEta", "jet-validation-track-collision-qa/jetEta", 1, 0, 0);
  AddHistogram(vecHisJets, "#it{p}_{T, track} (GeV/#it{c})", "jetTrackPt", "jet-validation-track-collision-qa/selectedTrackPt", 1, 1, 0);
  AddHistogram(vecHisJets, "#it{#varphi}_{track}", "jetTrackPhi", "jet-validation-track-collision-qa/selectedTrackPhi", 1, 0, 0);
  AddHistogram(vecHisJets, "#it{#eta}_{track}", "jetTrackEta", "jet-validation-track-collision-qa/selectedTrackEta", 1, 0, 0);

  // vector of specifications of vectors: name, VecSpecHis, pads X, pads Y
  VecSpecVecSpec vecSpecVecSpec;

  // Add vector specifications in the vector.
  // if (options.Contains(" events "))
  //  vecSpecVecSpec.push_back(std::make_tuple("events", vecHisEvents, 4, 2));
  if (options.Contains("jets"))
    vecSpecVecSpec.push_back(std::make_tuple("jets", vecHisJets, 3, 2));

  return MakePlots(vecSpecVecSpec, pathFileO2, pathFileAli, pathListAli, doRatio);
}
