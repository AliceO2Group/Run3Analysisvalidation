Int_t PlotEfficiency(TString pathFile = "AnalysisResults.root", TString particles = "d0")
{
  gStyle->SetOptStat(0);
  gStyle->SetPalette(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);

  // vertical margins of the pT spectra plot
  Float_t marginHigh = 0.05;
  Float_t marginLow = 0.05;
  Float_t k = 1. - marginHigh - marginLow;
  bool LogScale = true;
  // vertical margins of the efficiency plot
  Float_t marginRHigh = 0.05;
  Float_t marginRLow = 0.05;
  Float_t kR = 1. - marginRHigh - marginRLow;
  bool LogScaleR = false;
  Float_t yMin, yMax, yRange;
  Int_t nRec, nGen;
  // binning
  Int_t iNRebin = 4;
  Double_t dRebin[] = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 5, 6, 8, 10};
  const Int_t NRebin = sizeof(dRebin) / sizeof(dRebin[0]) - 1;

  TFile* file = new TFile(pathFile.Data());
  if (file->IsZombie()) {
    Printf("Error: Failed to open file %s", pathFile.Data());
    return 1;
  }

  TObjArray* arrayParticle = 0;
  arrayParticle = particles.Tokenize("-");
  TString particle = "";

  // loop over particles
  for (int iP = 0; iP < arrayParticle->GetEntriesFast(); iP++) {
    particle = ((TObjString*)(arrayParticle->At(iP)))->GetString();
    if (!particle.Length()) {
      Printf("Error: Empty particle string");
      return 1;
    }
    Printf("\nPlotting efficiency for: %s", particle.Data());

    TString histonameRec = Form("hf-task-%s-mc/hPtRecSig", particle.Data());
    TH1F* hPtRec = (TH1F*)file->Get(histonameRec.Data());
    if (!hPtRec) {
      Printf("Error: Failed to load %s from %s", histonameRec.Data(), pathFile.Data());
      return 1;
    }

    TString histonameGen = Form("hf-task-%s-mc/hPtGen", particle.Data());
    TH1F* hPtGen = (TH1F*)file->Get(histonameGen.Data());
    if (!hPtGen) {
      Printf("Error: Failed to load %s from %s", histonameGen.Data(), pathFile.Data());
      return 1;
    }

    TCanvas* canPt = new TCanvas(Form("canPt_%s", particle.Data()), "Pt", 1200, 1000);
    TCanvas* canEff = new TCanvas(Form("canEff_%s", particle.Data()), "Eff", 1200, 1000);

    nGen = hPtGen->GetEntries();
    nRec = hPtRec->GetEntries();

    // pT spectra
    canPt->cd();
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
      canPt->SetLogy();
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

    // efficiency
    canEff->cd();
    TH1F* hEff = (TH1F*)hPtRec->Clone("hEff");
    hEff->Divide(hPtGen);
    hEff->SetTitle(Form("Entries ratio: %g;#it{p}_{T} (GeV/#it{c});efficiency", (double)nRec / (double)nGen));
    yMin = hEff->GetMinimum(0);
    yMax = hEff->GetMaximum();
    if (LogScaleR && yMin > 0 && yMax > 0) {
      yRange = yMax / yMin;
      hEff->GetYaxis()->SetRangeUser(yMin / std::pow(yRange, marginRLow / kR), yMax * std::pow(yRange, marginRHigh / kR));
      canEff->SetLogy();
    } else {
      yRange = yMax - yMin;
      hEff->GetYaxis()->SetRangeUser(yMin - marginRLow / kR * yRange, yMax + marginRHigh / kR * yRange);
    }
    hEff->Draw();

    canPt->SaveAs(Form("MC_%s_pT.pdf", particle.Data()));
    canEff->SaveAs(Form("MC_%s_eff.pdf", particle.Data()));
  }
  delete arrayParticle;
  return 0;
}
