void AddHistogram(std::vector<std::tuple<TString, TString, TString, int>>& vec, TString label, TString nameRun1, TString nameRun3, int rebin)
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
  std::vector<std::tuple<TString, TString, TString, int>> vecHisSpec;

  AddHistogram(vecHisSpec, "#it{p}_{T} before selections", "hPtAllTracks", "hf-produce-sel-track/hpt_nocuts", 2);
  AddHistogram(vecHisSpec, "#it{p}_{T} after selections", "hPtSelTracks", "hf-produce-sel-track/hpt_cuts_2prong", 2);
  AddHistogram(vecHisSpec, "DCA XY to prim vtx after selections", "hImpParSelTracks", "hf-produce-sel-track/hdcatoprimxy_cuts_2prong", 2);
  AddHistogram(vecHisSpec, "secondary vtx x", "h2ProngVertX", "hf-track-index-skims-creator/hvtx2_x", 5);
  AddHistogram(vecHisSpec, "secondary vtx y", "h2ProngVertY", "hf-track-index-skims-creator/hvtx2_y", 5);
  AddHistogram(vecHisSpec, "secondary vtx z", "h2ProngVertZ", "hf-track-index-skims-creator/hvtx2_z", 5);
  AddHistogram(vecHisSpec, "decay length", "hDecLenD0", "hf-task-d0/hdeclength", 2);
  AddHistogram(vecHisSpec, "decay length XY", "hDecLenXYD0", "hf-task-d0/hdeclengthxy", 2);
  AddHistogram(vecHisSpec, "#it{p}_{T} D^{0}", "hPtD0", "hf-task-d0/hptcand", 2);
  AddHistogram(vecHisSpec, "#it{p}_{T} prong 0", "hPtD0Dau0", "hf-task-d0/hptprong0", 2);
  AddHistogram(vecHisSpec, "#it{p}_{T} prong 1", "hPtD0Dau1", "hf-task-d0/hptprong1", 2);
  AddHistogram(vecHisSpec, "d0 prong 0 (cm)", "hImpParD0Dau0", "hf-task-d0/hd0Prong0", 2);
  AddHistogram(vecHisSpec, "d0 prong 1 (cm)", "hImpParD0Dau1", "hf-task-d0/hd0Prong1", 2);
  AddHistogram(vecHisSpec, "d0d0 (cm^{2})", "hd0Timesd0", "hf-task-d0/hd0d0", 2);
  AddHistogram(vecHisSpec, "2-prong mass (#pi K)", "hInvMassD0", "hf-task-d0/hmass", 2);
  AddHistogram(vecHisSpec, "3-prong mass (#pi K #pi)", "hInvMassDplus", "hf-task-dplus/hMass", 2);
  /*
  AddHistogram(vecHisSpec, "impact parameter error", "hImpParErr", "hf-task-d0/hImpParErr", 1);
  AddHistogram(vecHisSpec, "decay length error", "hDecLenErr", "hf-task-d0/hDecLenErr", 1);
  AddHistogram(vecHisSpec, "decay length XY error", "hDecLenXYErr", "hf-task-d0/hDecLenXYErr", 1);
  AddHistogram(vecHisSpec, "XX element of PV cov. matrix", "hCovPVXX", "hf-cand-creator-2prong/hCovPVXX", 1);
  AddHistogram(vecHisSpec, "XX element of SV cov. matrix", "hCovSVXX", "hf-cand-creator-2prong/hCovSVXX", 1);
  */
  AddHistogram(vecHisSpec, "secondary vtx x - 3prong", "hDplusVertX", "hf-track-index-skims-creator/hvtx3_x", 5);
  AddHistogram(vecHisSpec, "secondary vtx y - 3prong", "hDplusVertY", "hf-track-index-skims-creator/hvtx3_y", 5);
  AddHistogram(vecHisSpec, "secondary vtx z - 3prong", "hDplusVertZ", "hf-track-index-skims-creator/hvtx3_z", 5);

  const int nhisto = vecHisSpec.size();
  const int nhisto_2prong = 15;

  TH1F* hRun1[nhisto];
  TH1F* hRun3[nhisto];
  TH1F* hRatio[nhisto];
  TString nameHis = "";
  for (int index = 0; index < nhisto; index++) {
    nameHis = std::get<1>(vecHisSpec[index]);
    hRun1[index] = (TH1F*)lRun1->FindObject(nameHis.Data());
    if (!hRun1[index]) {
      printf("Failed to load %s from %s\n", nameHis.Data(), filerun1.Data());
      return 1;
    }
    nameHis = std::get<2>(vecHisSpec[index]);
    hRun3[index] = (TH1F*)fRun3->Get(nameHis.Data());
    if (!hRun3[index]) {
      printf("Failed to load %s from %s\n", nameHis.Data(), filerun3.Data());
      return 1;
    }
  }

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

  TCanvas* cv2 = new TCanvas("cv2", "Histos", 3000, 1600);
  cv2->Divide(5, 3);
  TCanvas* cr2 = new TCanvas("cr2", "Ratios", 3000, 1600);
  cr2->Divide(5, 3);

  TCanvas* cv3 = new TCanvas("cv3", "Histos", 3000, 1600);
  cv3->Divide(5, 3);
  TCanvas* cr3 = new TCanvas("cr3", "Ratios", 3000, 1600);
  cr3->Divide(5, 3);

  TCanvas* canHis = cv2;
  TCanvas* canRat = cr2;

  for (int index = 0, indexPad = 1; index < nhisto; index++, indexPad++) {
    nRun1 = hRun1[index]->GetEntries();
    nRun3 = hRun3[index]->GetEntries();
    /*
    Printf("%d: bins: %d, %d, ranges: %g-%g, %g-%g",
      index, hRun1[index]->GetNbinsX(), hRun3[index]->GetNbinsX(),
      hRun1[index]->GetXaxis()->GetBinLowEdge(1), hRun1[index]->GetXaxis()->GetBinUpEdge(hRun1[index]->GetNbinsX()),
      hRun3[index]->GetXaxis()->GetBinLowEdge(1), hRun3[index]->GetXaxis()->GetBinUpEdge(hRun3[index]->GetNbinsX()));
    */
    if (index == nhisto_2prong) {
      canHis = cv3;
      canRat = cr3;
      indexPad-=nhisto_2prong;
    }

    Printf("Index: %d, pad index: %d", index, indexPad);

    // Histograms
    auto pad = canHis->cd(indexPad);
    if (donorm) {
      hRun1[index]->Scale(1. / nRun1);
      hRun3[index]->Scale(1. / nRun3);
    }
    hRun1[index]->Rebin(std::get<3>(vecHisSpec[index]));
    hRun3[index]->Rebin(std::get<3>(vecHisSpec[index]));
    hRun1[index]->SetLineColor(1);
    hRun1[index]->SetLineWidth(2);
    hRun3[index]->SetLineColor(2);
    hRun3[index]->SetLineWidth(1);
    hRun1[index]->SetTitle(Form("Entries: Run1: %d, Run3: %d;%s;Entries", nRun1, nRun3, std::get<0>(vecHisSpec[index]).Data()));
    hRun1[index]->GetYaxis()->SetMaxDigits(3);
    yMin = TMath::Min(hRun3[index]->GetMinimum(0), hRun1[index]->GetMinimum(0));
    yMax = TMath::Max(hRun3[index]->GetMaximum(), hRun1[index]->GetMaximum());
    if (LogScale && yMin > 0 && yMax > 0) {
      yRange = yMax / yMin;
      hRun1[index]->GetYaxis()->SetRangeUser(yMin / std::pow(yRange, marginLow / k), yMax * std::pow(yRange, marginHigh / k));
      pad->SetLogy();
    } else {
      yRange = yMax - yMin;
      hRun1[index]->GetYaxis()->SetRangeUser(yMin - marginLow / k * yRange, yMax + marginHigh / k * yRange);
    }
    hRun1[index]->Draw();
    hRun3[index]->Draw("same");
    TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
    legend->AddEntry(hRun1[index], "Run1", "L");
    legend->AddEntry(hRun3[index], "Run3", "L");
    legend->Draw();

    // Ratio
    auto padR = canRat->cd(indexPad);
    hRatio[index] = (TH1F*)hRun3[index]->Clone(Form("hRatio%d", index));
    hRatio[index]->Divide(hRun1[index]);
    hRatio[index]->SetTitle(Form("Entries ratio: %g;%s;Run3/Run1", (double)nRun3 / (double)nRun1, std::get<0>(vecHisSpec[index]).Data()));
    yMin = hRatio[index]->GetMinimum(0);
    yMax = hRatio[index]->GetMaximum();
    if (LogScaleR && yMin > 0 && yMax > 0) {
      yRange = yMax / yMin;
      hRatio[index]->GetYaxis()->SetRangeUser(yMin / std::pow(yRange, marginLow / k), yMax * std::pow(yRange, marginHigh / k));
      padR->SetLogy();
    } else {
      yRange = yMax - yMin;
      hRatio[index]->GetYaxis()->SetRangeUser(yMin - marginRLow / kR * yRange, yMax + marginRHigh / kR * yRange);
    }
    hRatio[index]->Draw();
  }

  cv2->SaveAs("comparison_histos_2prong.pdf");
  cr2->SaveAs("comparison_ratios_2prong.pdf");
  cv3->SaveAs("comparison_histos_3prong.pdf");
  cr3->SaveAs("comparison_ratios_3prong.pdf");
  return 0;
}
