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

#include "../exec/utils_plot.h"

// vectors of histogram specifications: axis label, AliPhysics name, O2Physics path/name, rebin, log scale histogram, log scale ratio, projection axis
using VecSpecHis = std::vector<std::tuple<TString, TString, TString, int, bool, bool, TString>>;

// vector of specifications of vectors: name, VecSpecHis, pads X, pads Y
using VecSpecVecSpec = std::vector<std::tuple<TString, VecSpecHis, int, int>>;

// Add histogram specification in the vector.
void AddHistogram(VecSpecHis& vec, TString label, TString nameAli, TString nameO2, int rebin, bool logH, bool logR, TString proj = "x")
{
  vec.push_back(std::make_tuple(label, nameAli, nameO2, rebin, logH, logR, proj));
}

// Make validation plots.
Int_t MakePlots(const VecSpecVecSpec& vecSpecVecSpec,
                TString pathFileO2 = "AnalysisResults_O2.root",
                TString pathFileAli = "AnalysisResults_ALI.root",
                TString pathListAli = "list",
                bool doRatio = false)
{
  gStyle->SetOptStat(0);
  gStyle->SetPalette(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);

  TFile* fO2 = new TFile(pathFileO2.Data());
  if (fO2->IsZombie()) {
    printf("Failed to open file %s\n", pathFileO2.Data());
    return 1;
  }
  TFile* fAli = new TFile(pathFileAli.Data());
  if (fAli->IsZombie()) {
    printf("Failed to open file %s\n", pathFileAli.Data());
    return 1;
  }

  TList* lAli = nullptr;
  fAli->GetObject(pathListAli.Data(), lAli);
  if (!lAli) {
    printf("Failed to load list %s from %s\n", pathListAli.Data(), pathFileAli.Data());
    return 1;
  }

  // Histogram plot vertical margins
  Float_t marginHigh = 0.05;
  Float_t marginLow = 0.05;
  bool logScaleH = false;
  // Ratio plot vertical margins
  Float_t marginRHigh = 0.05;
  Float_t marginRLow = 0.05;
  bool logScaleR = false;
  Float_t yMin, yMax;
  Int_t nAli, nO2, rebin;

  TH1F* hAli = nullptr;
  TH1D* hO2 = nullptr;
  TH1F* hRatio = nullptr;
  TString labelAxis = "";
  TString nameHisAli = "";
  TString nameHisO2 = "";
  TString projAx = "";
  TCanvas* canHis = nullptr;
  TCanvas* canRat = nullptr;

  // loop over lists
  for (const auto& specVecSpec : vecSpecVecSpec) {
    auto nameSpec = std::get<0>(specVecSpec); // list name
    auto vecSpec = std::get<1>(specVecSpec);  // list of histogram specs.
    int nPadsX = std::get<2>(specVecSpec);    // number of horizontal pads
    int nPadsY = std::get<3>(specVecSpec);    // number of vertical pads
    Printf("\nProcessing histogram list: %s (%lu)", nameSpec.Data(), vecSpec.size());
    if (nPadsX * nPadsY < vecSpec.size()) {
      Printf("Not enough pads (%d)", nPadsX * nPadsY);
      return 1;
    }

    canHis = new TCanvas(Form("canHis_%s", nameSpec.Data()), Form("Histos_%s", nameSpec.Data()), 3000, 1600);
    SetCanvas(canHis, nPadsX, nPadsY);
    if (doRatio) {
      canRat = new TCanvas(Form("canRat_%s", nameSpec.Data()), Form("Ratios_%s", nameSpec.Data()), 3000, 1600);
      SetCanvas(canRat, nPadsX, nPadsY);
    }

    // loop over histograms
    for (int index = 0; index < vecSpec.size(); index++) {
      auto spec = vecSpec[index];
      labelAxis = std::get<0>(spec);
      nameHisAli = std::get<1>(spec);
      nameHisO2 = std::get<2>(spec);
      rebin = std::get<3>(spec);
      logScaleH = std::get<4>(spec);
      logScaleR = std::get<5>(spec);
      projAx = std::get<6>(spec);

      // Get AliPhysics histogram.
      hAli = reinterpret_cast<TH1F*>(lAli->FindObject(nameHisAli.Data()));
      if (!hAli) {
        printf("Failed to load %s from %s\n", nameHisAli.Data(), pathFileAli.Data());
        return 1;
      }

      // Get O2 histogram.
      auto oO2 = fO2->Get(nameHisO2.Data());
      if (!oO2) {
        printf("Failed to load %s from %s\n", nameHisO2.Data(), pathFileO2.Data());
        return 1;
      }

      if (oO2->InheritsFrom("TH3")) {
        if (projAx == "x") {
          hO2 = (reinterpret_cast<TH3D*>(oO2))->ProjectionX();
        } else if (projAx == "y") {
          hO2 = (reinterpret_cast<TH3D*>(oO2))->ProjectionY();
        }
      } else if (oO2->InheritsFrom("TH2")) {
        if (projAx == "x") {
          hO2 = (reinterpret_cast<TH2D*>(oO2))->ProjectionX();
        } else if (projAx == "y") {
          hO2 = (reinterpret_cast<TH2D*>(oO2))->ProjectionY();
        }
      } else {
        hO2 = reinterpret_cast<TH1D*>(oO2);
      }

      Printf("%d (%s, %s): bins: %d, %d, ranges: %g-%g, %g-%g",
             index, nameHisAli.Data(), nameHisO2.Data(),
             hAli->GetNbinsX(), hO2->GetNbinsX(),
             hAli->GetXaxis()->GetBinLowEdge(1), hAli->GetXaxis()->GetBinUpEdge(hAli->GetNbinsX()),
             hO2->GetXaxis()->GetBinLowEdge(1), hO2->GetXaxis()->GetBinUpEdge(hO2->GetNbinsX()));

      nAli = hAli->GetEntries();
      nO2 = hO2->GetEntries();

      // Histograms
      auto padH = canHis->cd(index + 1);
      hAli->Rebin(rebin);
      hO2->Rebin(rebin);
      hAli->SetLineColor(1);
      hAli->SetLineWidth(2);
      hO2->SetLineColor(2);
      hO2->SetLineWidth(1);
      hAli->SetTitle(Form("Entries: Ali: %d, O^{2}: %d;%s;Entries", nAli, nO2, labelAxis.Data()));
      hAli->GetYaxis()->SetMaxDigits(3);
      yMin = TMath::Min(hO2->GetMinimum(0), hAli->GetMinimum(0));
      yMax = TMath::Max(hO2->GetMaximum(), hAli->GetMaximum());
      SetHistogram(hAli, yMin, yMax, marginLow, marginHigh, logScaleH);
      SetPad(padH, logScaleH);
      hAli->Draw();
      hO2->Draw("same");
      TLegend* legend = new TLegend(0.8, 0.72, 1., 0.92);
      legend->AddEntry(hAli, "Ali", "L");
      legend->AddEntry(hO2, "O^{2}", "L");
      legend->Draw();

      // Ratio
      if (doRatio) {
        auto padR = canRat->cd(index + 1);
        hRatio = reinterpret_cast<TH1F*>(hO2->Clone(Form("hRatio%d", index)));
        hRatio->Divide(hAli);
        hRatio->SetTitle(Form("Entries ratio: %g;%s;O^{2}/Ali", static_cast<float>(nO2) / static_cast<float>(nAli), labelAxis.Data()));
        yMin = hRatio->GetMinimum(0);
        yMax = hRatio->GetMaximum();
        SetHistogram(hRatio, yMin, yMax, marginRLow, marginRHigh, logScaleR);
        SetPad(padR, logScaleR);
        hRatio->Draw();
      }
    }
    canHis->SaveAs(Form("comparison_histos_%s.pdf", nameSpec.Data()));
    canHis->SaveAs(Form("comparison_histos_%s.png", nameSpec.Data()));
    if (doRatio) {
      canRat->SaveAs(Form("comparison_ratios_%s.pdf", nameSpec.Data()));
      canRat->SaveAs(Form("comparison_ratios_%s.png", nameSpec.Data()));
    }
    delete canHis;
    delete canRat;
  }
  return 0;
}

#endif // EXEC_UTILS_VALIDATION_H_
