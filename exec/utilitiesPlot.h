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

// Plotting utilities

#ifndef EXEC_UTILITIESPLOT_H_
#define EXEC_UTILITIESPLOT_H_

#include <algorithm> // std::min, std::max

void SetCanvas(TCanvas* can, int nPadsX, int nPadsY)
{
  if (!can) {
    Fatal("SetCanvas", "Invalid canvas");
    return;
  }
  can->Divide(nPadsX, nPadsY, 0.005, 0.005);
}

void SetPad(TVirtualPad* pad, bool logScale)
{
  if (!pad) {
    Fatal("SetPad", "Invalid pad");
    return;
  }
  pad->SetBottomMargin(0.11);
  pad->SetLeftMargin(0.1);
  pad->SetTopMargin(0.08);
  pad->SetRightMargin(0.08);
  if (logScale) {
    pad->SetLogy();
  }
}

/// @brief Get maximum and minimum of a histograms with non-negative bin contents and errors
/// @param his  histogram
/// @param yMin  variable to set minimum
/// @param yMax  variable to set maximum
/// @param onlyPositive  make sure that yMin, yMax are positive
/// @param includeErrors  consider heights of error bars
void GetYRange(TH1* his, Float_t& yMin, Float_t& yMax, bool onlyPositive = true, bool includeErrors = true)
{
  if (!his) {
    Fatal("GetYRange", "Invalid histogram");
    return;
  }
  yMin = his->GetMinimum(0);
  yMax = his->GetMaximum();
  if (onlyPositive) {
    if (yMin <= 0.) {
      yMin = 1.;
    }
    if (yMax <= 0.) {
      yMax = 1.;
    }
  }
  if (includeErrors) {
    Float_t cont, err, yLow;
    for (Int_t i = 0; i < his->GetNbinsX(); i++) {
      cont = his->GetBinContent(i + 1);
      if (cont <= 0.) {
        continue;
      }
      err = his->GetBinError(i + 1);
      yLow = cont - err;
      if (onlyPositive && yLow <= 0.) {
        yLow = cont;
      }
      yMin = std::min(yMin, yLow);
      yMax = std::max(yMax, cont + err);
    }
  }
}

/// @brief Set plotting properties of a histogram
/// @param his  histogram
/// @param yMin  minimum y value to display
/// @param yMax  maximum y value to display
/// @param marginLow  margin to keep below yMin (expressed as a fraction of the full y-axis plotting range)
/// @param marginHigh  margin to keep above yMax (expressed as a fraction of the full y-axis plotting range)
/// @note The full y-axis plotting range is calculated from yMin, yMax, marginLow, marginHigh, logScale.
void SetHistogram(TH1* his, Float_t yMin, Float_t yMax, Float_t marginLow, Float_t marginHigh, bool& logScale)
{
  if (!his) {
    Fatal("SetHistogram", "Invalid histogram");
    return;
  }
  Float_t textsize = 0.05;
  his->GetYaxis()->SetTitleSize(textsize);
  his->GetXaxis()->SetTitleSize(textsize);
  his->GetYaxis()->SetTitleOffset(1.0);
  his->GetXaxis()->SetTitleOffset(1.0);
  Float_t k = 1. - marginHigh - marginLow;
  Float_t yRange;
  if (logScale && yMin > 0 && yMax > 0) {
    yRange = yMax / yMin;
    his->GetYaxis()->SetRangeUser(yMin / std::pow(yRange, marginLow / k), yMax * std::pow(yRange, marginHigh / k));
  } else {
    logScale = false;
    yRange = yMax - yMin;
    his->GetYaxis()->SetRangeUser(yMin - marginLow / k * yRange, yMax + marginHigh / k * yRange);
  }
}

void SetHistogramStyle(TH1* his, Int_t colour = 1, Int_t markerStyle = 1, Float_t markerSize = 1, Float_t lineWidth = 1)
{
  if (!his) {
    Fatal("SetHistogramStyle", "Invalid histogram");
    return;
  }
  his->SetLineColor(colour);
  his->SetLineWidth(lineWidth);
  his->SetMarkerColor(colour);
  his->SetMarkerStyle(markerStyle);
  his->SetMarkerSize(markerSize);
}

#endif // EXEC_UTILITIESPLOT_H_
