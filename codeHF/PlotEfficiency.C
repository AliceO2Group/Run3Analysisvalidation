Int_t PlotEfficiency(TString filerun3 = "AnalysisResults.root")
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

  /*
  TString histonameNRec="hf-cand-creator-2prong-mc/hNRecSig";
  TH1F* hNRec = (TH1F*)fRun3->Get(histonameNRec.Data());
  if (!hNRec) {
    printf("Failed to load %s from %s\n", histonameNRec.Data(), filerun3.Data());
    return 1;
  }

  TString histonameNGen="hf-cand-creator-2prong-mc/hNGen";
  TH1F* hNGen = (TH1F*)fRun3->Get(histonameNGen.Data());
  if (!hNGen) {
    printf("Failed to load %s from %s\n", histonameNGen.Data(), filerun3.Data());
    return 1;
  }

  Printf("Rec. D0(bar) → π± K∓:\t%g", hNRec->GetBinContent(1));
  Printf("Rec. D0 → π+ K−:\t%g", hNRec->GetBinContent(2));
  Printf("Rec. D0bar → π− K+:\t%g", hNRec->GetBinContent(3));

  Printf("Gen. D0(bar):\t%g", hNGen->GetBinContent(1));
  Printf("Gen. D0:\t%g", hNGen->GetBinContent(3));
  Printf("Gen. D0bar:\t%g", hNGen->GetBinContent(5));
  Printf("Gen. D0(bar) → π± K∓:\t%g", hNGen->GetBinContent(2));
  Printf("Gen. D0 → π+ K−:\t%g", hNGen->GetBinContent(4));
  Printf("Gen. D0bar → π− K+:\t%g", hNGen->GetBinContent(6));
  */

  TString histonameRec = "hf-task-d0-mc/hPtRecSig";
  TH1F* hPtRec = (TH1F*)fRun3->Get(histonameRec.Data());
  if (!hPtRec) {
    printf("Failed to load %s from %s\n", histonameRec.Data(), filerun3.Data());
    return 1;
  }

  TString histonameGen = "hf-task-d0-mc/hPtGen";
  TH1F* hPtGen = (TH1F*)fRun3->Get(histonameGen.Data());
  if (!hPtGen) {
    printf("Failed to load %s from %s\n", histonameGen.Data(), filerun3.Data());
    return 1;
  }

  // Histogram plot vertical margins
  Float_t marginHigh = 0.05;
  Float_t marginLow = 0.05;
  Float_t k = 1. - marginHigh - marginLow;
  bool LogScale = true;
  // Ratio plot vertical margins
  Float_t marginRHigh = 0.05;
  Float_t marginRLow = 0.05;
  Float_t kR = 1. - marginRHigh - marginRLow;
  bool LogScaleR = false;
  Float_t yMin, yMax, yRange;
  Int_t nRec, nGen;
  Int_t iNRebin = 4;
  Double_t dRebin[] = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 5, 6, 8, 10};
  const Int_t NRebin = sizeof(dRebin) / sizeof(dRebin[0]) - 1;

  TCanvas* cv = new TCanvas("cv", "Histos", 1200, 1000);
  TCanvas* cr = new TCanvas("cr", "Eff", 1200, 1000);

  nGen = hPtGen->GetEntries();
  nRec = hPtRec->GetEntries();
  cv->cd();
  hPtGen->Rebin(iNRebin);
  hPtRec->Rebin(iNRebin);
  //hPtRec = (TH1F*)hPtRec->Rebin(NRebin, "hPtRecR", dRebin);
  //hPtGen = (TH1F*)hPtGen->Rebin(NRebin, "hPtGenR", dRebin);
  hPtGen->SetLineColor(1);
  hPtGen->SetLineWidth(2);
  hPtRec->SetLineColor(2);
  hPtRec->SetLineWidth(1);
  hPtGen->SetTitle(Form("Entries: Rec: %d, Gen: %d;#it{p}_{T} (GeV/#it{c});entries", nRec, nGen));
  hPtGen->GetYaxis()->SetMaxDigits(3);
  yMin = TMath::Min(hPtRec->GetMinimum(0), hPtGen->GetMinimum(0));
  yMax = TMath::Max(hPtRec->GetMaximum(), hPtGen->GetMaximum());
  if (LogScale && yMin > 0 && yMax > 0) {
    yRange = yMax / yMin;
    hPtGen->GetYaxis()->SetRangeUser(yMin / std::pow(yRange, marginLow / k), yMax * std::pow(yRange, marginHigh / k));
    cv->SetLogy();
  } else {
    yRange = yMax - yMin;
    hPtGen->GetYaxis()->SetRangeUser(yMin - marginLow / k * yRange, yMax + marginHigh / k * yRange);
  }
  hPtGen->Draw();
  hPtRec->Draw("same");
  TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
  legend->AddEntry(hPtRec, "Rec", "L");
  legend->AddEntry(hPtGen, "Gen", "L");
  legend->Draw();
  cr->cd();
  TH1F* hEff = (TH1F*)hPtRec->Clone("hEff");
  hEff->Divide(hPtGen);
  hEff->SetTitle(Form("Entries ratio: %g;#it{p}_{T} (GeV/#it{c});efficiency", (double)nRec / (double)nGen));
  yMin = hEff->GetMinimum(0);
  yMax = hEff->GetMaximum();
  if (LogScaleR && yMin > 0 && yMax > 0) {
    yRange = yMax / yMin;
    hEff->GetYaxis()->SetRangeUser(yMin / std::pow(yRange, marginLow / k), yMax * std::pow(yRange, marginHigh / k));
    cr->SetLogy();
  } else {
    yRange = yMax - yMin;
    hEff->GetYaxis()->SetRangeUser(yMin - marginRLow / kR * yRange, yMax + marginRHigh / kR * yRange);
  }
  hEff->Draw();

  cv->SaveAs("D0_MC_pT.pdf");
  cr->SaveAs("D0_MC_eff.pdf");
  return 0;
}
