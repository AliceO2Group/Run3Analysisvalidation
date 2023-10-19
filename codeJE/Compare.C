// Comparison of AliPhysics and O2 histograms

#include "utils_plot.h"

// vectors of histogram specifications
using VecSpecHis = std::vector<std::tuple<TString, TString, TString, int, bool, bool, TString>>;

// Add histogram specification in the vector.
void AddHistogram(VecSpecHis& vec, TString label, TString nameRun2, TString nameRun3, int rebin, bool logH, bool logR, TString proj = "x")
{
  vec.push_back(std::make_tuple(label, nameRun2, nameRun3, rebin, logH, logR, proj));
}

Int_t Compare(TString filerun3 = "AnalysisResults_O2.root", TString filerun2 = "AnalysisResults_ALI.root", TString options = "jets", bool doRatio = true)
{
  gStyle->SetOptStat(0);
  gStyle->SetPalette(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);

  TFile* fRun3 = new TFile(filerun3.Data());
  if (fRun3->IsZombie()) {
    printf("Failed to open file %s\n", filerun3.Data());
    return 1;
  }
  TFile* fRun2 = new TFile(filerun2.Data());
  if (fRun2->IsZombie()) {
    printf("Failed to open file %s\n", filerun2.Data());
    return 1;
  }

  TString pathListRun2 = Form("ChJetSpectraAliAnalysisTaskEmcalJetValidation/AliAnalysisTaskEmcalJetValidation");//
  TList* lRun2 = nullptr;
  fRun2->GetObject(pathListRun2.Data(), lRun2);
  if (!lRun2) {
    printf("Failed to load list %s from %s\n", pathListRun2.Data(), filerun2.Data());
    return 1;
  }

  // Histogram specification: axis label, AliPhysics name, O2Physics path/name, rebin, log scale histogram, log scale ratio, projection axis

  VecSpecHis vecHisEvents;

  VecSpecHis vecHisJets;     //X axis ,                  Hist Name Run2,  Hist  Name in Run3, rebin, logScaleH , logScaleR
  AddHistogram(vecHisJets, "#it{p}_{T,ch jet} (GeV/#it{c}", "jetPt",     "jet-validation-track-collision-qa/jetPt",        1,      1,          0);   //
  AddHistogram(vecHisJets, "#varphi_{jet}",                 "jetPhi",    "jet-validation-track-collision-qa/jetPhi",       1,      0,          0);
  AddHistogram(vecHisJets, "#eta_{jet}",                    "jetEta",    "jet-validation-track-collision-qa/jetEta",       1,      0,          0);
  AddHistogram(vecHisJets, "#it{p}_{T,track} (GeV/#it{c}",  "jetTrackPt",     "jet-validation-track-collision-qa/selectedTrackPt",        1,      1,          0);   //
  AddHistogram(vecHisJets, "#varphi_{track}",               "jetTrackPhi",    "jet-validation-track-collision-qa/selectedTrackPhi",       1,      0,          0);
  AddHistogram(vecHisJets, "#eta_{track}",                  "jetTrackEta",    "jet-validation-track-collision-qa/selectedTrackEta",       1,      0,          0);


  // vector of specifications of vectors: name, VecSpecHis, pads X, pads Y
  std::vector<std::tuple<TString, VecSpecHis, int, int>> vecSpecVecSpec;

  // Add vector specifications in the vector.
  // if (options.Contains(" events "))
  //  vecSpecVecSpec.push_back(std::make_tuple("events", vecHisEvents, 4, 2));
  if (options.Contains("jets"))
    vecSpecVecSpec.push_back(std::make_tuple("jets", vecHisJets, 3, 2));
  // if (options.Contains(" skim "))
  //  vecSpecVecSpec.push_back(std::make_tuple("skim", vecHisSkim, 5, 3));

  // Histogram plot vertical margins
  Float_t marginHigh = 0.05;
  Float_t marginLow = 0.05;
  bool logScaleH = false;
  // Ratio plot vertical margins
  Float_t marginRHigh = 0.05;
  Float_t marginRLow = 0.05;
  bool logScaleR = false;
  Float_t yMin, yMax;
  Int_t nRun2, nRun3, rebin;

  TH1F* hRun2 = nullptr;
  TH1D* hRun3 = nullptr;
  TH1F* hRatio = nullptr;
  TString labelAxis = "";
  TString nameHisRun2 = "";
  TString nameHisRun3 = "";
  TString projAx = "";
  TCanvas* canHis = nullptr;
  TCanvas* canRat = nullptr;

  // loop over lists
  for (const auto& specVecSpec : vecSpecVecSpec) {
    auto nameSpec = std::get<0>(specVecSpec); // list name
    auto vecSpec = std::get<1>(specVecSpec);  // list of histogram specs.
    int nPadsX = std::get<2>(specVecSpec);    // number of horizontal pads
    int nPadsY = std::get<3>(specVecSpec);    // number of vertical pads
    Printf("\nProcessing histogram list: %s (%d)", nameSpec.Data(), (int)vecSpec.size());
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
      nameHisRun2 = std::get<1>(spec);
      nameHisRun3 = std::get<2>(spec);
      rebin = std::get<3>(spec);
      logScaleH = std::get<4>(spec);
      logScaleR = std::get<5>(spec);
      projAx = std::get<6>(spec);

      // Get AliPhysics histogram.
      hRun2 = (TH1F*)lRun2->FindObject(nameHisRun2.Data());
      if (!hRun2) {
        printf("Failed to load %s from %s\n", nameHisRun2.Data(), filerun2.Data());
        return 1;
      }

      // Get O2 histogram.
      auto oRun3 = fRun3->Get(nameHisRun3.Data());
      if (!oRun3) {
        printf("Failed to load %s from %s\n", nameHisRun3.Data(), filerun3.Data());
        return 1;
      }

      if (oRun3->InheritsFrom("TH3")) {
        if (projAx == "x") {
          hRun3 = ((TH3D*)oRun3)->ProjectionX();
        } else if (projAx == "y") {
          hRun3 = ((TH3D*)oRun3)->ProjectionY();
        }
      } else if (oRun3->InheritsFrom("TH2")) {
        if (projAx == "x") {
          hRun3 = ((TH2D*)oRun3)->ProjectionX();
        } else if (projAx == "y") {
          hRun3 = ((TH2D*)oRun3)->ProjectionY();
        }
      } else {
        hRun3 = (TH1D*)oRun3;
      }

      Printf("%d (%s, %s): bins: %d, %d, ranges: %g-%g, %g-%g",
             index, nameHisRun2.Data(), nameHisRun3.Data(),
             hRun2->GetNbinsX(), hRun3->GetNbinsX(),
             hRun2->GetXaxis()->GetBinLowEdge(1), hRun2->GetXaxis()->GetBinUpEdge(hRun2->GetNbinsX()),
             hRun3->GetXaxis()->GetBinLowEdge(1), hRun3->GetXaxis()->GetBinUpEdge(hRun3->GetNbinsX()));

      nRun2 = hRun2->GetEntries();
      nRun3 = hRun3->GetEntries();
      double xmin = 0;
      double xmax = 100;
      // Histograms
      auto padH = canHis->cd(index + 1);

      Printf("histo name: %s", hRun2->GetName());
      if (std::string(hRun2->GetName()) == "jetPt" || std::string(hRun2->GetName()) == "jetTrackPt"){
        xmin = 0;
        xmax = 15;
        rebin = 1;
      }
      if (std::string(hRun2->GetName()) == "jetPhi" || std::string(hRun2->GetName()) == "jetTrackPhi"){
        xmin = 0;
        xmax = 6.3;
        rebin = 8;
      }
      if (std::string(hRun2->GetName()) == "jetEta"){
        xmin = -0.6;
        xmax = 0.6;
        rebin = 6;
      }
      if (std::string(hRun2->GetName()) == "jetTrackEta"){
        xmin = -0.9;
        xmax = 0.9;
        rebin = 8;
      }
      
      hRun2->Rebin(rebin);
      hRun3->Rebin(rebin);
      hRun2->GetXaxis()->SetRangeUser(xmin, xmax);
      hRun3->GetXaxis()->SetRangeUser(xmin, xmax);
      hRun2->SetLineColor(1);
      hRun2->SetLineWidth(2);
      hRun2->SetMarkerStyle(22);
      hRun2->SetMarkerColor(1);
      hRun3->SetLineColor(2);
      hRun3->SetLineWidth(2);
      hRun3->SetMarkerStyle(22);
      hRun3->SetMarkerColor(2);
      //hRun2->SetTitle(Form("Entries: AliPhysics: %d, O2Physics: %d;%s;Entries", nRun2, nRun3, labelAxis.Data()));
      hRun2->SetTitle(" ");
      hRun2->GetYaxis()->SetTitle("number of entries");
      hRun2->GetYaxis()->SetMaxDigits(3);
      yMin = TMath::Min(hRun3->GetMinimum(0), hRun2->GetMinimum(0));
      yMax = TMath::Max(hRun3->GetMaximum(), hRun2->GetMaximum());
      SetHistogram(hRun2, yMin, yMax, marginLow, marginHigh, logScaleH);
      SetPad(padH, logScaleH);
      hRun2->Draw("");
      hRun3->Draw("Esame");

      if (std::string(hRun2->GetName()) == "jetPt" || std::string(hRun2->GetName()) == "jetTrackPt"){
        TLegend* legend = new TLegend(0.2, 0.84, 0.82, 0.92);
        legend->SetNColumns(2);
        legend->AddEntry(hRun2, Form("AliPhysics: %d", nRun2), "L");
        legend->AddEntry(hRun3, Form("O2Physics: %d", nRun3), "L");
        legend->Draw();
      }else{
        TLegend* legend = new TLegend(0.2, 0.92, 0.82, 1.0);
        legend->SetNColumns(2);
        legend->SetBorderSize(0);
        legend->AddEntry(hRun2, Form("AliPhysics: %d", nRun2), "L");
        legend->AddEntry(hRun3, Form("O2Physics: %d", nRun3), "L");
        legend->Draw();
      }



      // Ratio
      if (doRatio) {
        auto padR = canRat->cd(index + 1);
        hRatio = (TH1F*)hRun3->Clone(Form("hRatio%d", index));
        hRatio->Divide(hRun2);
        //hRatio->SetTitle(Form("Entries ratio: %g;%s;AliPhysics/O2Physics", (double) hRatio->GetEntries(), labelAxis.Data()));
        hRatio->SetTitle("");
        hRatio->GetYaxis()->SetTitle("O2Physics/AliPhysics");
        yMin = hRatio->GetMinimum(0)*0.3;
        yMax = hRatio->GetMaximum()*1.3;
        SetHistogram(hRatio, yMin, yMax, marginRLow, marginRHigh, logScaleR);
        SetPad(padR, logScaleR);
        hRatio->Draw();
      }
    }
    canHis->SaveAs(Form("comparison_histos_%s.png", nameSpec.Data()));
    if (doRatio) {
      canRat->SaveAs(Form("comparison_ratios_%s.png", nameSpec.Data()));
    }
//    delete canHis;
    delete canRat;
  }


  return 0;
}
