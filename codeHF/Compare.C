Int_t Compare(TString filerun3 = "AnalysisResults.root", TString filerun1 = "Vertices2prong-ITS1.root", double mass = 1.8)
{
  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  gStyle->SetOptStat(0000);
  gStyle->SetPalette(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);
  gStyle->SetOptTitle(0);

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

  TCanvas* cv = new TCanvas("cv", "Vertex", 3000, 1600);
  cv->Divide(5, 3);
  gPad->SetLogy();
  for (int index = 0; index < nhisto; index++) {
    cv->cd(index + 1);
    hRun1[index]->SetLineColor(1);
    hRun1[index]->SetLineWidth(3);
    hRun3[index]->SetLineColor(2);
    hRun3[index]->SetLineWidth(1);
    hRun1[index]->GetXaxis()->SetTitle(xaxis[index].Data());
    hRun1[index]->Draw();
    hRun3[index]->Draw("same");
    TLegend* legend = new TLegend(0.5, 0.7, 0.8, 0.9);
    legend->AddEntry(hRun1[index], "Run1", "f");
    legend->AddEntry(hRun3[index], "Run3", "f");
    legend->Draw();
  }
  cv->SaveAs("comparison.pdf");
  return 0;
}
