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

// Validation utilities

#include <tuple>

// vectors of histogram specifications
using VecSpecHis = std::vector<std::tuple<TString, TString, TString, int, bool, bool, TString>>;

// Add histogram specification in the vector.
void AddHistogram(VecSpecHis& vec, TString label, TString nameAli, TString nameO2, int rebin, bool logH, bool logR, TString proj = "x")
{
  vec.push_back(std::make_tuple(label, nameAli, nameO2, rebin, logH, logR, proj));
}
