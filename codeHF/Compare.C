Int_t Compare(TString filerun3 = "AnalysisResults.root", TString filerun1 = "Vertices2prong-ITS1.root", double mass = 1.8, bool donorm = false, int particle = 2)
{
  gStyle->SetOptStat(0);
  gStyle->SetPalette(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);
  //=========================================================================================

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

  int nhisto, nhisto_2prong;
  TString histonameRun1[100];
  TString histonameRun3[100];
  TString xaxis[100];
  int rebin[100];

  TH1F* hRun1[100];
  TH1F* hRun3[100];
  TH1F* hRatio[100];
  //=========================================================================================
  // particle 1 Dzero
  if (particle == 1) {
    const int n = 19;
    nhisto_2prong = 15;
    TString tmp1[n] = {
      "hPtAllTracks",
      "hPtSelTracks",
      "hImpParSelTracks",
      "h2ProngVertX",
      "h2ProngVertY",
      "h2ProngVertZ",
      "hDecLenD0",
      "hDecLenXYD0",
      "hPtD0",
      "hPtD0Dau0",
      "hPtD0Dau1",
      "hImpParD0Dau0",
      "hImpParD0Dau1",
      "hd0Timesd0",
      "hInvMassD0",
      "hInvMassDplus",
      "hDplusVertX",
      "hDplusVertY",
      "hDplusVertZ"};
    TString tmp2[n] = {"hf-produce-sel-track/hpt_nocuts",
                       "hf-produce-sel-track/hpt_cuts_2prong",
                       "hf-produce-sel-track/hdcatoprimxy_cuts_2prong",
                       "hf-track-index-skims-creator/hvtx2_x",
                       "hf-track-index-skims-creator/hvtx2_y",
                       "hf-track-index-skims-creator/hvtx2_z",
                       "hf-task-d0/hdeclength",
                       "hf-task-d0/hdeclengthxy",
                       "hf-task-d0/hptcand",
                       "hf-task-d0/hptprong0",
                       "hf-task-d0/hptprong1",
                       "hf-task-d0/hd0Prong0",
                       "hf-task-d0/hd0Prong1",
                       "hf-task-d0/hd0d0",
                       "hf-task-d0/hmass",
                       "hf-task-dplus/hMass",
                       "hf-track-index-skims-creator/hvtx3_x",
                       "hf-track-index-skims-creator/hvtx3_y",
                       "hf-track-index-skims-creator/hvtx3_z"};
    TString tmp3[n] = {"#it{p}_{T} before selections",
                       "#it{p}_{T} after selections",
                       "DCA XY to prim vtx after selections",
                       "secondary vtx x",
                       "secondary vtx y",
                       "secondary vtx z",
                       "decay length",
                       "decay length XY",
                       "#it{p}_{T} D^{0}",
                       "#it{p}_{T} prong 0",
                       "#it{p}_{T} prong 1",
                       "d0 prong 0 (cm)",
                       "d0 prong 1 (cm)",
                       "d0d0 (cm^{2})",
                       "2-prong mass (#pi K)",
                       "3-prong mass (#pi K #pi)",
                       "secondary vtx x - 3prong",
                       "secondary vtx y - 3prong",
                       "secondary vtx z - 3prong"};
    int tmp4[n] = {2, 2, 2, 5, 5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 /*, 1, 1, 1, 1, 1*/, 5, 5, 5};
    for (auto i = 0; i < n; i++) {
      histonameRun1[i] = tmp1[i];
      histonameRun3[i] = tmp2[i];
      xaxis[i] = tmp3[i];
      rebin[i] = tmp4[i];
    }
    nhisto = n;
  }

  //=========================================================================================
  // particle 2 Jpsi to e+e-
  if (particle == 2) {
    const int n = 15;
    nhisto_2prong = 15;
    TString tmp1[n] = {
      "hPtAllTracks",
      "hPtSelTracks",
      "hImpParSelTracks",
      "h2ProngVertX",
      "h2ProngVertY",
      "h2ProngVertZ",
      "hDecLenJpsi",
      "hDecLenXYJpsi",
      "hPtJpsi",
      "hPtJpsiDau0",
      "hPtJpsiDau1",
      "hImpParJpsiDau0",
      "hImpParJpsiDau1",
      "hd0Timesd0",
      "hInvMassJpsi",
    };
    TString tmp2[n] = {
      "hf-produce-sel-track/hpt_nocuts",
      "hf-produce-sel-track/hpt_cuts_2prong",
      "hf-produce-sel-track/hdcatoprimxy_cuts_2prong",
      "hf-track-index-skims-creator/hvtx2_x",
      "hf-track-index-skims-creator/hvtx2_y",
      "hf-track-index-skims-creator/hvtx2_z",
      "hf-task-jpsi/hdeclength",
      "hf-task-jpsi/hdeclengthxy",
      "hf-task-jpsi/hptcand",
      "hf-task-jpsi/hptprong0",
      "hf-task-jpsi/hptprong1",
      "hf-task-jpsi/hd0Prong0",
      "hf-task-jpsi/hd0Prong1",
      "hf-task-jpsi/hd0d0",
      "hf-task-jpsi/hmass",
    };
    TString tmp3[n] = {
      "#it{p}_{T} before selections",
      "#it{p}_{T} after selections",
      "DCA XY to prim vtx after selections",
      "secondary vtx x",
      "secondary vtx y",
      "secondary vtx z",
      "decay length",
      "decay length XY",
      "#it{p}_{T} Jpsi",
      "#it{p}_{T} prong 0",
      "#it{p}_{T} prong 1",
      "d0 prong 0 (cm)",
      "d0 prong 1 (cm)",
      "d0d0 (cm^{2})",
      "2-prong mass (#e+ e-)",

    };
    int tmp4[n] = {2, 2, 2, 5, 5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2};
    for (auto i = 0; i < n; i++) {
      histonameRun1[i] = tmp1[i];
      histonameRun3[i] = tmp2[i];
      xaxis[i] = tmp3[i];
      rebin[i] = tmp4[i];
    }
    nhisto = n;
  }
  //=========================================================================================

  //=========================================================================================
  // particle 3 Lambdac
  if (particle == 3) {
    const int n = 13;
    nhisto_2prong = 10;
    TString tmp1[n] = {"hPtAllTracks",
                       "hPtSelTracks",
                       "hImpParSelTracks",
                       "hPtLc",
                       "hPtLcDau0",
                       "hPtLcDau1",
                       "hPtLcDau2",
                       "hDecLenLc",
                       "hCosPointLc",
                       "hInvMassLc",
                       "hLcpKpiVertX",
                       "hLcpKpiVertY",
                       "hLcpKpiVertZ"};
    TString tmp2[n] = {
      "hf-produce-sel-track/hpt_nocuts",
      "hf-produce-sel-track/hpt_cuts_2prong",
      "hf-produce-sel-track/hdcatoprimxy_cuts_2prong",
      "hf-task-lc/hptcand",
      "hf-task-lc/hptprong0",
      "hf-task-lc/hptprong1",
      "hf-task-lc/hptprong2",
      "hf-task-lc/declength",
      // "hf-task-lc/hd0Prong0",
      // "hf-task-lc/hd0Prong1"
      // "hf-task-lc/hd0Prong2",
      "hf-task-lc/hCPA",
      //"hf-task-lc/hCt",
      //"hf-task-lc/hImpParErr",
      //"hf-task-lc/hEta",
      // "hf-task-lc/hImapa",
      "hf-task-lc/hmass",
      "hf-track-index-skims-creator/hvtx3_x",
      "hf-track-index-skims-creator/hvtx3_y",
      "hf-track-index-skims-creator/hvtx3_z",
    };
    TString tmp3[n] = {"#it{p}_{T} before selections",
                       "#it{p}_{T} after selections",
                       "DCA XY to prim vtx after selections",
                       "#Lambda_{c} pt cand",
                       "#Lambda_{c} pt prong 0",
                       "#Lambda_{c} pt prong 1",
                       "#Lambda_{c} pt prong 2",
                       "#Lambda_{c} Decay Lenght",
                       "#Lambda_{c} CPA",
                       "3-prong mass (p K #pi)",
                       "secondary vtx x - 3prong",
                       "secondary vtx y - 3prong",
                       "secondary vtx z - 3prong"};
    int tmp4[n] = {2, 2, 2, 5, 5, 5, 2, 2, 2, 2, 2, 2, 2};
    for (auto i = 0; i < n; i++) {
      histonameRun1[i] = tmp1[i];
      histonameRun3[i] = tmp2[i];
      xaxis[i] = tmp3[i];
      rebin[i] = tmp4[i];
    }
    nhisto = n;
  }
  //=========================================================================================

  for (int index = 0; index < nhisto; index++) {
    hRun1[index] = (TH1F*)lRun1->FindObject(histonameRun1[index].Data());
    if (!hRun1[index]) {
      printf("Failed to load %s from %s\n", histonameRun1[index].Data(), filerun1.Data());
      return 1;
    }
    hRun3[index] = (TH1F*)fRun3->Get(histonameRun3[index].Data());
    if (!hRun3[index]) {
      printf("Failed to load %s from %s\n", histonameRun3[index].Data(), filerun3.Data());
      return 1;
    }
  }
  //=========================================================================================

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

  TCanvas* cv = new TCanvas("cv", "2 prong Histos", 3000, 1600);
  cv->Divide(5, 3);
  TCanvas* cr = new TCanvas("cr", "2 prong Ratios", 3000, 1600);
  cr->Divide(5, 3);

  for (int index = 0; index < nhisto_2prong; index++) {
    nRun1 = hRun1[index]->GetEntries();
    nRun3 = hRun3[index]->GetEntries();
    auto pad = cv->cd(index + 1);
    if (donorm) {
      hRun1[index]->Scale(1. / nRun1);
      hRun3[index]->Scale(1. / nRun3);
    }
    hRun1[index]->Rebin(rebin[index]);
    hRun3[index]->Rebin(rebin[index]);
    hRun1[index]->SetLineColor(1);
    hRun1[index]->SetLineWidth(2);
    hRun3[index]->SetLineColor(2);
    hRun3[index]->SetLineWidth(1);
    hRun1[index]->SetTitle(Form("Entries: Run1: %d, Run3: %d;%s;Entries", nRun1, nRun3, xaxis[index].Data()));
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
    auto padR = cr->cd(index + 1);
    hRatio[index] = (TH1F*)hRun3[index]->Clone(Form("hRatio%d", index));
    hRatio[index]->Divide(hRun1[index]);
    hRatio[index]->SetTitle(Form("Entries ratio: %g;%s;Run3/Run1", (double)nRun3 / (double)nRun1, xaxis[index].Data()));
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
  cv->SaveAs("comparison_histos_2prong.pdf");
  cr->SaveAs("comparison_ratios_2prong.pdf");
  //=========================================================================================
  if (particle == 3 || particle == 4) {
    TCanvas* cv3 = new TCanvas("cv3", "3 prong Histos", 3000, 1600);
    cv3->Divide(5, 3);
    TCanvas* cr3 = new TCanvas("cr3", "3 prong Ratios", 3000, 1600);
    cr3->Divide(5, 3);

    Int_t i = 0;
    for (int index = nhisto_2prong; index < nhisto; index++) {
      nRun1 = hRun1[index]->GetEntries();
      nRun3 = hRun3[index]->GetEntries();
      auto pad = cv3->cd(i + 1);
      if (donorm) {
        hRun1[index]->Scale(1. / nRun1);
        hRun3[index]->Scale(1. / nRun3);
      }
      hRun1[index]->Rebin(rebin[index]);
      hRun3[index]->Rebin(rebin[index]);
      hRun1[index]->SetLineColor(1);
      hRun1[index]->SetLineWidth(2);
      hRun3[index]->SetLineColor(2);
      hRun3[index]->SetLineWidth(1);
      hRun1[index]->SetTitle(Form("Entries: Run1: %d, Run3: %d;%s;Entries", nRun1, nRun3, xaxis[index].Data()));
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
      auto padR = cr3->cd(i + 1);
      hRatio[index] = (TH1F*)hRun3[index]->Clone(Form("hRatio%d", index));
      hRatio[index]->Divide(hRun1[index]);
      hRatio[index]->SetTitle(Form("Entries ratio: %g;%s;Run3/Run1", (double)nRun3 / (double)nRun1, xaxis[index].Data()));
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
      i = i + 1;
    }
    cv3->SaveAs("comparison_histos_3prong.pdf");
    cr3->SaveAs("comparison_ratios_3prong.pdf");
  }

  //=========================================================================================

  return 0;
}
