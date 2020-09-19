Int_t Compare(TString filerun3 = "AnalysisResults.root", TString filerun1 = "Vertices2prong-ITS1.root", double mass = 1.8, bool donorm = false)
{
  gStyle->SetOptStat(0);
  gStyle->SetPalette(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);

  TFile* fRun3 = new TFile(filerun3.Data());
  TFile* fRun1 = new TFile(filerun1.Data());

  const int nhisto = 15;
  TString histonameRun1[nhisto] = {"hpt_nocuts",
                                   "hpt_cuts",
                                   "hdcatoprimxy_cuts",
                                   "hvx",
                                   "hvy",
                                   "hvz",
                                   "hdecayxyz",
                                   "hdecayxy",
                                   "hptD0",
                                   "hptprong0",
                                   "hptprong1",
                                   "hd0",
                                   "hd0d0",
                                   "hmass0",
                                   "hmassP"};
  TString histonameRun3[nhisto] = {"hf-produce-sel-track/hpt_nocuts",
                                   "hf-produce-sel-track/hpt_cuts",
                                   "hf-produce-sel-track/hdcatoprimxy_cuts",
                                   "hf-track-index-skims-creator/hvtx_x",
                                   "hf-track-index-skims-creator/hvtx_y",
                                   "hf-track-index-skims-creator/hvtx_z",
                                   "hf-task-d0/declength",
                                   "hf-task-d0/declengthxy",
                                   "hf-task-d0/hptcand",
                                   "hf-task-d0/hptprong0",
                                   "hf-task-d0/hptprong1",
                                   "hf-task-d0/hd0",
                                   "hf-task-d0/hd0d0",
                                   "hf-task-d0/hmass",
                                   "hf-track-index-skims-creator/hmass3"};
  TString xaxis[nhisto] = {"#it{p}_{T} before selections",
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
                           "d0 (cm)",
                           "d0d0 (cm^{2})",
                           "2-prong mass (#pi K)",
                           "3-prong mass (#pi K #pi)"};
  int rebin[nhisto] = {2, 2, 2, 5, 5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2};
  TH1F* hRun1[nhisto];
  TH1F* hRun3[nhisto];
  TH1F* hRatio[nhisto];
  for (int index = 0; index < nhisto; index++) {
    hRun1[index] = (TH1F*)fRun1->Get(histonameRun1[index].Data());
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

  Float_t marginHigh = 0.05;
  Float_t marginLow = 0.05;
  Float_t k = 1. - marginHigh - marginLow;
  Float_t yMin, yMax, yRange;

  TCanvas* cv = new TCanvas("cv", "Histos", 3000, 3000);
  cv->Divide(3, 5);
  TCanvas* cr = new TCanvas("cr", "Ratios", 3000, 3000);
  cr->Divide(3, 5);
  Int_t nRun1, nRun3;
  for (int index = 0; index < nhisto; index++) {
    nRun1 = hRun1[index]->Integral(0, -1);
    nRun3 = hRun3[index]->Integral(0, -1);
    cv->cd(index + 1);
    if (donorm){
      hRun1[index]->Scale(1./hRun1[index]->GetEntries());
      hRun3[index]->Scale(1./hRun3[index]->GetEntries());
    }
    hRun1[index]->Rebin(rebin[index]);
    hRun3[index]->Rebin(rebin[index]);
    hRun1[index]->SetLineColor(1);
    hRun1[index]->SetLineWidth(2);
    hRun3[index]->SetLineColor(2);
    hRun3[index]->SetLineWidth(1);
    hRun1[index]->SetTitle(Form("Entries: Run1: %d, Run3: %d;%s;Entries", nRun1, nRun3, xaxis[index].Data()));
    hRun1[index]->GetYaxis()->SetMaxDigits(3);
    double minyval = TMath::Min(hRun3[index]->GetMinimum(), hRun1[index]->GetMinimum());
    double maxyval = TMath::Max(hRun3[index]->GetMaximum(), hRun1[index]->GetMaximum());
    hRun1[index]->GetYaxis()->SetRangeUser(minyval, maxyval*1.1);
    hRun1[index]->Draw();
    hRun3[index]->Draw("same");
    TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
    legend->AddEntry(hRun1[index], "Run1", "L");
    legend->AddEntry(hRun3[index], "Run3", "L");
    legend->Draw();
    cr->cd(index + 1);
    hRatio[index] = (TH1F*)hRun3[index]->Clone(Form("hRatio%d", index));
    hRatio[index]->Divide(hRun1[index]);
    hRatio[index]->SetTitle(Form("Entries ratio: %g;%s;Run3/Run1", (double)nRun3/(double)nRun1, xaxis[index].Data()));
    yMin = hRatio[index]->GetMinimum(0);
    yMax = hRatio[index]->GetMaximum();
    yRange = yMax - yMin;
    hRatio[index]->GetYaxis()->SetRangeUser(yMin - marginLow/k * yRange, yMax + marginHigh/k * yRange);
    hRatio[index]->Draw();
  }
  cv->SaveAs("comparison_histos.pdf");
  cr->SaveAs("comparison_ratios.pdf");
  return 0;
}
