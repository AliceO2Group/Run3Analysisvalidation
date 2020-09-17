Int_t Compare(TString filerun3 = "AnalysisResults.root", TString filerun1 = "Vertices2prong-ITS1.root", double mass = 1.8)
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
                                   "hmass0",
                                   "hvx",
                                   "hvy",
                                   "hvz",
                                   "hmassP",
                                   "hdecayxyz",
                                   "hdecayxy",
                                   "hptD0",
                                   "hptprong0",
                                   "hptprong1",
                                   "hd0",
                                   "hd0d0"};
  TString histonameRun3[nhisto] = {"hf-produce-sel-track/hpt_nocuts",
                                   "hf-produce-sel-track/hpt_cuts",
                                   "hf-produce-sel-track/hdcatoprimxy_cuts",
                                   "hf-task-d0/hmass",
                                   "hf-cand-creator-2prong/hvtx_x",
                                   "hf-cand-creator-2prong/hvtx_y",
                                   "hf-cand-creator-2prong/hvtx_z",
                                   "hf-track-index-skims-creator/hmass3",
                                   "hf-task-d0/declength",
                                   "hf-task-d0/declengthxy",
                                   "hf-task-d0/hptcand",
                                   "hf-task-d0/hptprong0",
                                   "hf-task-d0/hptprong1",
                                   "hf-task-d0/hd0",
                                   "hf-task-d0/hd0d0"};
  TString xaxis[nhisto] = {"#it{p}_{T} before selections",
                           "#it{p}_{T} after selections",
                           "DCA XY to prim vtx after selections",
                           "2-prong mass (#pi K)",
                           "secondary vtx x",
                           "secondary vtx y",
                           "secondary vtx z",
                           "3-prong mass (#pi K #pi)",
                           "decay length",
                           "decay length XY",
                           "#it{p}_{T} D^{0}",
                           "#it{p}_{T} prong 0",
                           "#it{p}_{T} prong 1",
                           "d0 (cm)",
                           "d0d0 (cm^{2})"};

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

  TCanvas* cv = new TCanvas("cv", "Histos", 3000, 1600);
  cv->Divide(5, 3);
  TCanvas* cr = new TCanvas("cr", "Ratios", 3000, 1600);
  cr->Divide(5, 3);
  Int_t nRun1, nRun3;
  for (int index = 0; index < nhisto; index++) {
    nRun1 = hRun1[index]->Integral(0, -1);
    nRun3 = hRun3[index]->Integral(0, -1);
    cv->cd(index + 1);
    hRun1[index]->SetLineColor(1);
    hRun1[index]->SetLineWidth(3);
    hRun3[index]->SetLineColor(2);
    hRun3[index]->SetLineWidth(1);
    hRun1[index]->SetTitle(Form("Entries: Run1: %d, Run3: %d;%s;Entries", nRun1, nRun3, xaxis[index].Data()));
    hRun1[index]->GetYaxis()->SetMaxDigits(3);
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
    hRatio[index]->Draw();
  }
  cv->SaveAs("comparison_histos.pdf");
  cr->SaveAs("comparison_ratios.pdf");
  return 0;
}
