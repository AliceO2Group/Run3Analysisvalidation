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

#ifndef EXEC_UTILS_PLOT_H_
#define EXEC_UTILS_PLOT_H_

void SetCanvas(TCanvas* can, int nPadsX, int nPadsY)
{
  can->Divide(nPadsX, nPadsY, 0.005, 0.005);
}

void SetPad(TVirtualPad* pad, bool logScale)
{
  pad->SetBottomMargin(0.11);
  pad->SetLeftMargin(0.1);
  pad->SetTopMargin(0.08);
  pad->SetRightMargin(0.08);
  if (logScale) {
    pad->SetLogy();
  }
}

void SetHistogram(TH1* his, Float_t yMin, Float_t yMax, Float_t marginLow, Float_t marginHigh, bool& logScale)
{
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
  his->SetLineColor(colour);
  his->SetLineWidth(lineWidth);
  his->SetMarkerColor(colour);
  his->SetMarkerStyle(markerStyle);
  his->SetMarkerSize(markerSize);
}

#endif // EXEC_UTILS_PLOT_H_
