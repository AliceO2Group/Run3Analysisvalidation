Bool_t Compare(TString filerun3="AnalysisResults.root", TString filerun1="Vertices2prong-ITS1.root"){

  gROOT->SetStyle("Plain");	
  gStyle->SetOptStat(0);
  gStyle->SetOptStat(0000);
  gStyle->SetPalette(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);
  gStyle->SetOptTitle(0);

  TFile *fRun3 = new TFile(filerun3.Data());
  TFile *fRun1 = new TFile(filerun1.Data());

  TH1F* hpt_cuts_Run3 = (TH1F*)fRun3->Get("vertexerhf-task/hpt_cuts");
  TH1F* hpt_cuts_Run1 = (TH1F*)fRun1->Get("hpt_cuts");

  TH1F* htgl_cuts_Run3 = (TH1F*)fRun3->Get("vertexerhf-task/htgl_cuts");
  TH1F* htgl_cuts_Run1 = (TH1F*)fRun1->Get("htgl_cuts");
 
  TH1F* hvtx_Run3 = (TH1F*)fRun3->Get("vertexerhf-task/hvtx_x");
  TH1F* hvtx_Run1 = (TH1F*)fRun1->Get("hvx");
  
  TH1F* hvty_Run3 = (TH1F*)fRun3->Get("vertexerhf-task/hvtx_y");
  TH1F* hvty_Run1 = (TH1F*)fRun1->Get("hvy");
  
  TH1F* hvtz_Run3 = (TH1F*)fRun3->Get("vertexerhf-task/hvtx_z");
  TH1F* hvtz_Run1 = (TH1F*)fRun1->Get("hvz");
  
  TH1F* hitsmap_Run3 = (TH1F*)fRun3->Get("vertexerhf-task/hitsmap");
  TH1F* hitsmap_Run1 = (TH1F*)fRun1->Get("hitsmap");
  
  TH1F* hdecayxyz_Run3 = (TH1F*)fRun3->Get("vertexerhf-task/hdecayxyz");
  TH1F* hdecayxyz_Run1 = (TH1F*)fRun1->Get("hdecayxyz");
  
  TH1F* hdecayxy_Run3 = (TH1F*)fRun3->Get("vertexerhf-task/hdecayxy");
  TH1F* hdecayxy_Run1 = (TH1F*)fRun1->Get("hdecayxy");
  
  TH1F* hmass_Run3 = (TH1F*)fRun3->Get("vertexerhf-task/hmass");
  TH1F* hmass_Run1 = (TH1F*)fRun1->Get("hmass");
  
  TLegend * legend = new TLegend(0.5,0.7,0.8,0.9);
  //legend->SetHeader("Legend","C"); // option "C" allows to center the header


  TCanvas* cv=new TCanvas("cv","Vertex",1600,1600);
  cv->Divide(3,3);
  cv -> cd(1);
  gPad-> SetLogy();
  hpt_cuts_Run1->GetXaxis()->SetTitle("#it{p}_{T} (GeV)");
  hpt_cuts_Run1->SetLineColor(2);
  hpt_cuts_Run1->SetLineWidth(2);
  hpt_cuts_Run1->Draw();
  legend->AddEntry(hpt_cuts_Run1,"Run1","f");
  legend->AddEntry(hpt_cuts_Run3,"Run3","f");
  hpt_cuts_Run3->Draw("same");
  legend->Draw();
  cv -> cd(2);
  gPad-> SetLogy();
  htgl_cuts_Run1->GetXaxis()->SetTitle("Tgl param.");
  htgl_cuts_Run1->SetLineColor(2);
  htgl_cuts_Run1->SetLineWidth(2);
  htgl_cuts_Run1->Draw();
  htgl_cuts_Run3->Draw("same");
  legend->Draw();
  cv -> cd(3);
  hmass_Run3->GetXaxis()->SetTitle("Invariant mass K#pi");
  hmass_Run1->SetLineColor(2);
  hmass_Run1->SetLineWidth(2);
  hmass_Run1->Draw();
  hmass_Run3->Draw("same");
  legend->Draw();
  legend->Draw();
  cv -> cd(4);
  gPad-> SetLogy();
  hvtx_Run1->GetXaxis()->SetTitle("D^{0} vertex x resolution (cm)");
  hvtx_Run1->SetLineColor(2);
  hvtx_Run1->SetLineWidth(2);
  hvtx_Run1->Draw();
  hvtx_Run3->Draw("same");
  legend->Draw();
  cv -> cd(5);
  gPad-> SetLogy();
  hvty_Run1->GetXaxis()->SetTitle("D^{0} vertex y resolution (cm)");
  hvty_Run1->SetLineColor(2);
  hvty_Run1->SetLineWidth(2);
  hvty_Run1->Draw();
  hvty_Run3->Draw("same");
  legend->Draw();
  cv -> cd(6);
  gPad-> SetLogy();
  hvtz_Run1->GetXaxis()->SetTitle("D^{0} vertex z resolution (cm)");
  hvtz_Run1->SetLineColor(2);
  hvtz_Run1->SetLineWidth(2);
  hvtz_Run1->Draw();
  hvtz_Run3->Draw("same");
  legend->Draw();
  cv -> cd(7);
  gPad-> SetLogy();
  hitsmap_Run1->GetXaxis()->SetTitle("ITS cluster map");
  hitsmap_Run1->SetLineColor(2);
  hitsmap_Run1->SetLineWidth(2);
  hitsmap_Run1->Draw();
  hitsmap_Run3->Draw("same");
  legend->Draw();
  cv -> cd(8);
  gPad-> SetLogy();
  hdecayxy_Run1->GetXaxis()->SetTitle("decay length xy");
  hdecayxy_Run1->SetLineColor(2);
  hdecayxy_Run1->SetLineWidth(2);
  hdecayxy_Run1->Draw();
  hdecayxy_Run3->Draw("same");
  legend->Draw();
  cv -> cd(9);
  gPad-> SetLogy();
  hdecayxyz_Run1->GetXaxis()->SetTitle("decay length xyz");
  hdecayxyz_Run1->SetLineColor(2);
  hdecayxyz_Run1->SetLineWidth(2);
  hdecayxyz_Run1->Draw();
  hdecayxyz_Run3->Draw("same");
  legend->Draw();
  
  cv->SaveAs("cv.pdf");
  return true; 
}



