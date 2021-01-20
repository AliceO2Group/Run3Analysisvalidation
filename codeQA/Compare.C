Bool_t Compare()
{

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  gStyle->SetOptStat(0000);
  gStyle->SetPalette(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);
  gStyle->SetOptTitle(0);

  TFile* fRun3 = new TFile("AnalysisResults.root");
  TFile* fRun1 = new TFile("QA-ITS1.root");

  TH1F* hpt_cuts_Run3 = (TH1F*)fRun3->Get("validation-qa/hpt_nocuts");
  TH1F* hpt_cuts_Run1 = (TH1F*)fRun1->Get("hpt_cuts");

  TLegend* legend = new TLegend(0.5, 0.7, 0.8, 0.9);
  //legend->SetHeader("Legend","C"); // option "C" allows to center the header

  TCanvas* cv = new TCanvas("cv", "Vertex", 1600, 700);
  cv->Divide(3, 2);
  cv->cd(1);
  gPad->SetLogy();
  hpt_cuts_Run1->GetXaxis()->SetTitle("#it{p}_{T} (GeV)");
  hpt_cuts_Run1->SetLineColor(2);
  hpt_cuts_Run1->SetLineWidth(2);
  hpt_cuts_Run1->Draw();
  legend->AddEntry(hpt_cuts_Run1, "Run1", "f");
  legend->AddEntry(hpt_cuts_Run3, "Run3", "f");
  hpt_cuts_Run3->Draw("same");
  legend->Draw();
  cv->SaveAs("cv.pdf");
  return true;
}
