void SOverBComp() {

  TH1D *sig_sc3;
  TH1D *sig_sc2;
  
  Int_t nBinsMerge = 6;
  Double_t xAxis1[7] = {0,1,2,4,6,8,10};
 
  TFile *fileSig3 = TFile::Open("Scenario3/foutputLambda_c.root","READ");
  cout<<"ok"<<endl;
  sig_sc3 = new TH1D(*((TH1D*)fileSig3->Get("histosigoverbkg")));
 
  TFile *fileSig2 = TFile::Open("Scenario2/foutputLambda_c.root","READ");
  cout<<"ok"<<endl;
  sig_sc2 = new TH1D(*((TH1D*)fileSig2->Get("histosigoverbkg")));

  int textfont = 42;
  float textsize = 0.047;
  float labelsize = 0.04;

  TLatex* lat = new TLatex();
  lat->SetNDC();
  lat->SetTextFont(textfont);
  lat->SetTextSize(textsize);

  TLatex* lat2 = new TLatex();
  lat2->SetNDC();
  lat2->SetTextFont(textfont);
  lat2->SetTextSize(textsize*1.0);

  TLatex* lat3 = new TLatex();
  lat3->SetNDC();
  lat3->SetTextFont(textfont);
  lat3->SetTextSize(textsize*1.0);

  TCanvas *canvas3a = new TCanvas("canvas3a", "",443,100,700,700);
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(textfont);
  gStyle->SetLegendTextSize(textsize);
  
  canvas3a->Range(0,0,1,1);
  canvas3a->SetFillColor(0);
  canvas3a->SetBorderMode(0);
  canvas3a->SetBorderSize(2);
  canvas3a->SetFrameBorderMode(0);

 
  // ------------>Primitives in pad: canvas3a_1
  TPad *canvas3a_1 = new TPad("canvas3a_1", "canvas3a_1",0.01,0.3,0.99,0.99);
  canvas3a_1->Draw();
  canvas3a_1->cd();
  canvas3a_1->Range(-0.3750001,-2248.875,13.375,20249.88);
  canvas3a_1->SetFillColor(0);
  canvas3a_1->SetBorderMode(0);
  canvas3a_1->SetBorderSize(2);
  //canvas3a_1->SetGridx();
  //canvas3a_1->SetGridy();
  canvas3a_1->SetLeftMargin(0.15);
  canvas3a_1->SetFrameBorderMode(0);
  canvas3a_1->SetFrameBorderMode(0);

  cout<<"ok"<<endl;

  //sig_sc3->SetMarkerSize(1.25);
  //sig_sc3->SetMinimum(2.27);
  //sig_sc3->SetMaximum(2.30);
  sig_sc3->SetEntries(7);
  sig_sc3->SetStats(0);
  sig_sc3->SetLineWidth(2);
  sig_sc3->SetLineColor(4);
  //sig_sc3->SetMarkerColor(1);
  //sig_sc3->SetMarkerStyle(21);
  sig_sc3->GetXaxis()->SetTitle("#it{p}_{T}(#Lambda_{c}) [GeV/#it{c}]");
  sig_sc3->GetXaxis()->SetNdivisions(511);
  sig_sc3->GetXaxis()->SetRangeUser(0,10);
  sig_sc3->GetXaxis()->SetLabelFont(42);
  sig_sc3->GetXaxis()->SetLabelSize(0.04);
  sig_sc3->GetXaxis()->SetTitleSize(0.04);
  sig_sc3->GetXaxis()->SetTitleOffset(1.0);
  sig_sc3->GetYaxis()->SetRangeUser(0.000003,5);
  sig_sc3->GetXaxis()->SetTitleFont(42);
 
  // sig_sc3->GetYaxis()->SetTitle("#frac{d^{2}#sigma}{dy dp_{T}} [#frac{#mu b}{GeV/c}]");
  sig_sc3->GetYaxis()->SetTitle("Signal/Background (3#sigma)");
  sig_sc3->GetYaxis()->SetLabelFont(42);
  sig_sc3->GetYaxis()->SetTitleFont(42);
  sig_sc3->GetYaxis()->SetLabelSize(0.05);
  sig_sc3->GetYaxis()->SetTitleSize(0.07);
  sig_sc3->GetYaxis()->SetTitleOffset(0.9);
  sig_sc3->GetYaxis()->SetDecimals(kTRUE);

  canvas3a_1->SetLogy();
  sig_sc3->Draw("");

  sig_sc2->SetLineWidth(2);
  sig_sc2->SetLineColor(2);
  // sig_sc2->SetMarkerStyle(21);
  // sig_sc2->SetMarkerSize(1.25);
  //sig_sc2->SetMarkerColor(2);
  sig_sc2->Draw("same");

  lat->DrawLatex(0.2,0.83,"ALICE3 projection, pp #sqrt{s}= 14 TeV");
  lat2->DrawLatex(0.2,0.76,"Pythia 8, mode = 2");
  lat3->DrawLatex(0.2,0.69,"#Lambda_{c} #rightarrow pK#pi, |y| < 1.44, BR=6.28%");
  

  /////////////////////////////////////////////////////////////////////// RATIO /////////////////////////////////////////////////////////////////////

  TLegend *leg = new TLegend(0.53,0.25,0.88,0.40,0,"NDC");
  leg->AddEntry(sig_sc3,"Scenario3","lp");
  leg->AddEntry(sig_sc2,"Scenario2","lp");
  leg->SetFillStyle(0);
  leg->SetBorderSize(0);
  leg->SetFillColor(0);
  //leg->SetTextSize(22);

  leg->Draw();

  canvas3a_1->Modified();
  canvas3a->cd();


  // ------------>Primitives in pad: canvas3a_2
  TPad *canvas3a_2 = new TPad("canvas3a_2", "canvas3a_2",0.01,0.01,0.99,0.365);
  canvas3a_2->Draw();
  canvas3a_2->cd();
  canvas3a_2->Range(-0.3750001,-0.7452094,13.375,0.6383566);
  canvas3a_2->SetFillColor(0);
  canvas3a_2->SetBorderMode(0);
  canvas3a_2->SetBorderSize(2);
  //canvas3a_2->SetGridx();
  canvas3a_2->SetGridy();
  canvas3a_2->SetLeftMargin(0.15);
  canvas3a_2->SetBottomMargin(0.2);
  canvas3a_2->SetFrameBorderMode(0);
  canvas3a_2->SetFrameBorderMode(0);

  TH1F *SigRatio = new TH1F("SigRatio","",6,xAxis1);
  SigRatio->Divide(sig_sc3,sig_sc2,1.,1.);
  
  //SigRatio->SetMarkerStyle(1);
  //SigRatio->SetMarkerSize(1);
  SigRatio->SetMinimum(0.99);
  SigRatio->SetMaximum(1.01);
  SigRatio->SetEntries(7);
  SigRatio->SetStats(0);
  SigRatio->SetLineWidth(2);
  SigRatio->SetLineColor(4);
  SigRatio->GetXaxis()->SetTitle("p_{T}(#Lambda_{c}) [GeV/c]");
  //SigRatio->GetXaxis()->SetNdivisions(511);
  SigRatio->GetXaxis()->SetLabelFont(42);
  SigRatio->GetXaxis()->SetLabelSize(0.085);
  SigRatio->GetXaxis()->SetTitleSize(0.095);
  SigRatio->GetXaxis()->SetTitleOffset(1.0);
  SigRatio->GetXaxis()->SetRangeUser(0,10);  
  SigRatio->GetYaxis()->SetRangeUser(0.,15);
  SigRatio->GetXaxis()->SetTitleFont(42);
  SigRatio->GetYaxis()->SetTitle("Scenario3 / Scenario2");
  SigRatio->GetYaxis()->SetNdivisions(106);   
  SigRatio->GetYaxis()->SetLabelFont(42);
  SigRatio->GetYaxis()->SetLabelSize(0.085);
  SigRatio->GetYaxis()->SetTitleSize(0.08);
  SigRatio->GetYaxis()->SetTitleOffset(0.7);
  SigRatio->GetYaxis()->SetTitleFont(42);
  SigRatio->GetYaxis()->SetDecimals(kTRUE);
  SigRatio->Draw("");

  TLine *line = new TLine(0,1,10.,1);
  line->SetLineColor(1);
  line->SetLineWidth(4);
  line->SetLineStyle(2);
  line->Draw("same");

  canvas3a_2->Modified();
  canvas3a->cd();
  canvas3a->Modified();
  canvas3a->cd();
  canvas3a->SetSelected(canvas3a);
  
}
