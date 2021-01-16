// Comparison of AliPhysics and O2 histograms

// vectors of histogram specifications
using VecSpecHis = std::vector<std::tuple<TString, TString, TString, int>>;

// Add histogram specification in the vector.
void AddHistogram(VecSpecHis& vec, TString label, TString nameRun1, TString nameRun3, int rebin)
{
  vec.push_back(std::make_tuple(label, nameRun1, nameRun3, rebin));
}

Int_t Compare(TString filerun3 = "AnalysisResults.root", TString filerun1 = "AnalysisResults.root", double mass = 1.8, bool donorm = false)
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
  TFile* fRun1 = new TFile(filerun1.Data());
  if (fRun1->IsZombie()) {
    printf("Failed to open file %s\n", filerun1.Data());
    return 1;
  }

  TString pathListRun1 = "HFVertices/clistHFVertices";
  TList* lRun1 = nullptr;
  fRun1->GetObject(pathListRun1.Data(), lRun1);
  if (!lRun1) {
    printf("Failed to load list %s from %s\n", pathListRun1.Data(), filerun1.Data());
    return 1;
  }

  // Histogram specification: axis label, Run 1 name, Run 3 path/name, rebin

  VecSpecHis vecHisTracks;
  AddHistogram(vecHisTracks, "#it{p}_{T} before selections", "hPtAllTracks", "hf-produce-sel-track/hpt_nocuts", 2);
  AddHistogram(vecHisTracks, "#it{p}_{T} after selections", "hPtSelTracks", "hf-produce-sel-track/hpt_cuts_2prong", 2);
  AddHistogram(vecHisTracks, "DCA XY to prim vtx after selections", "hImpParSelTracks", "hf-produce-sel-track/hdcatoprimxy_cuts_2prong", 2);

  VecSpecHis vecHisSkim;
  AddHistogram(vecHisSkim, "secondary vtx x - 2prong", "h2ProngVertX", "hf-track-index-skims-creator/hvtx2_x", 5);
  AddHistogram(vecHisSkim, "secondary vtx y - 2prong", "h2ProngVertY", "hf-track-index-skims-creator/hvtx2_y", 5);
  AddHistogram(vecHisSkim, "secondary vtx z - 2prong", "h2ProngVertZ", "hf-track-index-skims-creator/hvtx2_z", 5);
  AddHistogram(vecHisSkim, "secondary vtx x - 3prong", "hDplusVertX", "hf-track-index-skims-creator/hvtx3_x", 5);
  AddHistogram(vecHisSkim, "secondary vtx y - 3prong", "hDplusVertY", "hf-track-index-skims-creator/hvtx3_y", 5);
  AddHistogram(vecHisSkim, "secondary vtx z - 3prong", "hDplusVertZ", "hf-track-index-skims-creator/hvtx3_z", 5);

  VecSpecHis vecHisCand2;
  AddHistogram(vecHisCand2, "XX element of PV cov. matrix", "hCovMatPrimVXX2Prong", "hf-cand-creator-2prong/hCovPVXX", 1);
  AddHistogram(vecHisCand2, "XX element of SV cov. matrix", "hCovMatSecVXX2Prong", "hf-cand-creator-2prong/hCovSVXX", 1);

  VecSpecHis vecHisCand3;
  AddHistogram(vecHisCand3, "XX element of PV cov. matrix", "hCovMatPrimVXX3Prong", "hf-cand-creator-3prong/hCovPVXX", 1);
  AddHistogram(vecHisCand3, "XX element of SV cov. matrix", "hCovMatSecVXX3Prong", "hf-cand-creator-3prong/hCovSVXX", 1);

  VecSpecHis vecHisD0;
  AddHistogram(vecHisD0, "#it{p}_{T} prong 0", "hPtD0Dau0", "hf-task-d0/hptprong0", 2);
  AddHistogram(vecHisD0, "#it{p}_{T} prong 1", "hPtD0Dau1", "hf-task-d0/hptprong1", 2);
  AddHistogram(vecHisD0, "#it{p}_{T} D^{0}", "hPtD0", "hf-task-d0/hptcand", 2);
  AddHistogram(vecHisD0, "2-prong mass (#pi K)", "hInvMassD0", "hf-task-d0/hmass", 2);
  AddHistogram(vecHisD0, "d0 prong 0 (cm)", "hImpParD0Dau0", "hf-task-d0/hd0Prong0", 2);
  AddHistogram(vecHisD0, "d0 prong 1 (cm)", "hImpParD0Dau1", "hf-task-d0/hd0Prong1", 2);
  AddHistogram(vecHisD0, "d0d0 (cm^{2})", "hd0Timesd0", "hf-task-d0/hd0d0", 2);
  AddHistogram(vecHisD0, "impact parameter error", "hImpParErrD0Dau", "hf-task-d0/hImpParErr", 1);
  AddHistogram(vecHisD0, "decay length", "hDecLenD0", "hf-task-d0/hdeclength", 2);
  AddHistogram(vecHisD0, "decay length XY", "hDecLenXYD0", "hf-task-d0/hdeclengthxy", 2);
  AddHistogram(vecHisD0, "decay length error", "hDecLenErrD0", "hf-task-d0/hDecLenErr", 1);
  AddHistogram(vecHisD0, "decay length XY error", "hDecLenXYErrD0", "hf-task-d0/hDecLenXYErr", 1);

  VecSpecHis vecHisDPlus;
  AddHistogram(vecHisDPlus, "3-prong mass (#pi K #pi)", "hInvMassDplus", "hf-task-dplus/hMass", 2);

  // vector of specifications of vectors: name, VecSpecHis
  std::vector<std::tuple<TString, VecSpecHis>> vecSpecVecSpec;

  // Add vector specifications in the vector.
  vecSpecVecSpec.push_back(std::make_tuple("tracks", vecHisTracks));
  vecSpecVecSpec.push_back(std::make_tuple("skim", vecHisSkim));
  vecSpecVecSpec.push_back(std::make_tuple("cand2", vecHisCand2));
  vecSpecVecSpec.push_back(std::make_tuple("cand3", vecHisCand3));
  vecSpecVecSpec.push_back(std::make_tuple("d0", vecHisD0));
  vecSpecVecSpec.push_back(std::make_tuple("dplus", vecHisDPlus));

  // Histogram plot vertical margins
  Float_t marginHigh = 0.05;
  Float_t marginLow = 0.05;
  Float_t k = 1. - marginHigh - marginLow;
  bool LogScale = false;
  // Ratio plot vertical margins
  Float_t marginRHigh = 0.05;
  Float_t marginRLow = 0.05;
  Float_t kR = 1. - marginRHigh - marginRLow;
  bool LogScaleR = false;
  Float_t yMin, yMax, yRange;
  Int_t nRun1, nRun3;

  TH1F* hRun1 = nullptr;
  TH1F* hRun3 = nullptr;
  TH1F* hRatio = nullptr;
  TString nameHisRun1 = "";
  TString nameHisRun3 = "";
  TCanvas* canHis = nullptr;
  TCanvas* canRat = nullptr;

  // loop over lists
  for (const auto& specVecSpec : vecSpecVecSpec) {
    auto nameSpec = std::get<0>(specVecSpec); // list name
    auto vecSpec = std::get<1>(specVecSpec);  // list of histogram specs.
    Printf("\nProcessing histogram list: %s (%d)", nameSpec.Data(), (int)vecSpec.size());

    TCanvas* canHis = new TCanvas(Form("canHis_%s", nameSpec.Data()), Form("Histos_%s", nameSpec.Data()), 3000, 1600);
    canHis->Divide(5, 3);
    TCanvas* canRat = new TCanvas(Form("canRat_%s", nameSpec.Data()), Form("Ratios_%s", nameSpec.Data()), 3000, 1600);
    canRat->Divide(5, 3);

    // loop over histograms
    for (int index = 0; index < vecSpec.size(); index++) {
      auto spec = vecSpec[index];

      // Get AliPhysics histogram.
      nameHisRun1 = std::get<1>(spec);
      hRun1 = (TH1F*)lRun1->FindObject(nameHisRun1.Data());
      if (!hRun1) {
        printf("Failed to load %s from %s\n", nameHisRun1.Data(), filerun1.Data());
        return 1;
      }

      // Get O2 histogram.
      nameHisRun3 = std::get<2>(spec);
      hRun3 = (TH1F*)fRun3->Get(nameHisRun3.Data());
      if (!hRun3) {
        printf("Failed to load %s from %s\n", nameHisRun3.Data(), filerun3.Data());
        return 1;
      }

      Printf("%d (%s, %s): bins: %d, %d, ranges: %g-%g, %g-%g",
             index, nameHisRun1.Data(), nameHisRun3.Data(),
             hRun1->GetNbinsX(), hRun3->GetNbinsX(),
             hRun1->GetXaxis()->GetBinLowEdge(1), hRun1->GetXaxis()->GetBinUpEdge(hRun1->GetNbinsX()),
             hRun3->GetXaxis()->GetBinLowEdge(1), hRun3->GetXaxis()->GetBinUpEdge(hRun3->GetNbinsX()));

      nRun1 = hRun1->GetEntries();
      nRun3 = hRun3->GetEntries();

      // Histograms
      auto pad = canHis->cd(index + 1);
      if (donorm) {
        hRun1->Scale(1. / nRun1);
        hRun3->Scale(1. / nRun3);
      }
      hRun1->Rebin(std::get<3>(spec));
      hRun3->Rebin(std::get<3>(spec));
      hRun1->SetLineColor(1);
      hRun1->SetLineWidth(2);
      hRun3->SetLineColor(2);
      hRun3->SetLineWidth(1);
      hRun1->SetTitle(Form("Entries: Run1: %d, Run3: %d;%s;Entries", nRun1, nRun3, std::get<0>(spec).Data()));
      hRun1->GetYaxis()->SetMaxDigits(3);
      yMin = TMath::Min(hRun3->GetMinimum(0), hRun1->GetMinimum(0));
      yMax = TMath::Max(hRun3->GetMaximum(), hRun1->GetMaximum());
      if (LogScale && yMin > 0 && yMax > 0) {
        yRange = yMax / yMin;
        hRun1->GetYaxis()->SetRangeUser(yMin / std::pow(yRange, marginLow / k), yMax * std::pow(yRange, marginHigh / k));
        pad->SetLogy();
      } else {
        yRange = yMax - yMin;
        hRun1->GetYaxis()->SetRangeUser(yMin - marginLow / k * yRange, yMax + marginHigh / k * yRange);
      }
      hRun1->Draw();
      hRun3->Draw("same");
      TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
      legend->AddEntry(hRun1, "Run1", "L");
      legend->AddEntry(hRun3, "Run3", "L");
      legend->Draw();

      // Ratio
      auto padR = canRat->cd(index + 1);
      hRatio = (TH1F*)hRun3->Clone(Form("hRatio%d", index));
      hRatio->Divide(hRun1);
      hRatio->SetTitle(Form("Entries ratio: %g;%s;Run3/Run1", (double)nRun3 / (double)nRun1, std::get<0>(spec).Data()));
      yMin = hRatio->GetMinimum(0);
      yMax = hRatio->GetMaximum();
      if (LogScaleR && yMin > 0 && yMax > 0) {
        yRange = yMax / yMin;
        hRatio->GetYaxis()->SetRangeUser(yMin / std::pow(yRange, marginLow / k), yMax * std::pow(yRange, marginHigh / k));
        padR->SetLogy();
      } else {
        yRange = yMax - yMin;
        hRatio->GetYaxis()->SetRangeUser(yMin - marginRLow / kR * yRange, yMax + marginRHigh / kR * yRange);
      }
      hRatio->Draw();
    }

    canHis->SaveAs(Form("comparison_histos_%s.pdf", nameSpec.Data()));
    canRat->SaveAs(Form("comparison_ratios_%s.pdf", nameSpec.Data()));
    delete canHis;
    delete canRat;
  }
  return 0;
}
