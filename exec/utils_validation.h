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

#ifndef EXEC_UTILS_VALIDATION_H_
#define EXEC_UTILS_VALIDATION_H_

#include <tuple>
#include <vector>

// vectors of histogram specifications: axis label, AliPhysics name, O2Physics path/name, rebin, log scale histogram, log scale ratio, projection axis
using VecSpecHis = std::vector<std::tuple<TString, TString, TString, int, bool, bool, TString>>;

// vector of specifications of vectors: name, VecSpecHis, pads X, pads Y
using VecSpecVecSpec = std::vector<std::tuple<TString, VecSpecHis, int, int>>;

// Add histogram specification in the vector.
void AddHistogram(VecSpecHis& vec, TString label, TString nameAli, TString nameO2, int rebin, bool logH, bool logR, TString proj = "x")
{
  vec.push_back(std::make_tuple(label, nameAli, nameO2, rebin, logH, logR, proj));
}

#endif // EXEC_UTILS_VALIDATION_H_
