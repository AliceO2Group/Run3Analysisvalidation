void Chi_c_PbPb5p52_absy1p44_results()
{
//=========Macro generated from canvas: canvas/A Simple Graph Example
//=========  (Mon Oct 18 11:29:13 2021) by ROOT version 6.20/08
   TCanvas *canvas = new TCanvas("canvas", "A Simple Graph Example",779,196,667,636);
   gStyle->SetOptStat(0);
   canvas->ToggleEventStatus();
   canvas->SetHighLightColor(2);
   canvas->Range(-1.25,-3.625,11.25,2.625);
   canvas->SetFillColor(0);
   canvas->SetBorderMode(0);
   canvas->SetBorderSize(2);
   canvas->SetLogy();
   canvas->SetFrameBorderMode(0);
   canvas->SetFrameBorderMode(0);
   
   TH2F *hempty__1 = new TH2F("hempty__1","",100,0,10,100,0.001,100);
   hempty__1->SetStats(0);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#000099");
   hempty__1->SetLineColor(ci);
   hempty__1->GetXaxis()->SetTitle("p_{T} (GeV/c)");
   hempty__1->GetXaxis()->SetLabelFont(42);
   hempty__1->GetXaxis()->SetTitleOffset(1);
   hempty__1->GetXaxis()->SetTitleFont(42);
   hempty__1->GetYaxis()->SetTitle(" Significance(3#sigma)");
   hempty__1->GetYaxis()->SetLabelFont(42);
   hempty__1->GetYaxis()->SetTitleOffset(1.35);
   hempty__1->GetYaxis()->SetTitleFont(42);
   hempty__1->GetZaxis()->SetLabelFont(42);
   hempty__1->GetZaxis()->SetTitleOffset(1);
   hempty__1->GetZaxis()->SetTitleFont(42);
   hempty__1->Draw("");
   Double_t xAxis1[10] = {0, 0.5, 1, 2, 3, 4, 5, 7, 10, 15}; 
   
   TH1F *histosignf__2 = new TH1F("histosignf__2","",9, xAxis1);
   histosignf__2->SetBinContent(1,0.007023328);
   histosignf__2->SetBinContent(2,0.02818377);
   histosignf__2->SetBinContent(3,0.1402838);
   histosignf__2->SetBinContent(4,0.1778309);
   histosignf__2->SetBinContent(5,0.1540644);
   histosignf__2->SetBinContent(6,0.1263784);
   histosignf__2->SetBinContent(7,0.1661939);
   histosignf__2->SetBinContent(8,0.2462514);
   histosignf__2->SetBinContent(9,0.3668575);
   histosignf__2->SetBinContent(10,8.503325e-07);
   histosignf__2->SetEntries(5109);
   histosignf__2->SetLineWidth(2);
   histosignf__2->GetXaxis()->SetTitle("p_{T} (GeV)");
   histosignf__2->GetXaxis()->SetRange(1,100);
   histosignf__2->GetXaxis()->SetLabelFont(42);
   histosignf__2->GetXaxis()->SetTitleOffset(1);
   histosignf__2->GetXaxis()->SetTitleFont(42);
   histosignf__2->GetYaxis()->SetTitle("chic1 d#sigma^{PYTHIA}/dp_{T} (#mu b/GeV)");
   histosignf__2->GetYaxis()->SetLabelFont(42);
   histosignf__2->GetYaxis()->SetTitleFont(42);
   histosignf__2->GetZaxis()->SetLabelFont(42);
   histosignf__2->GetZaxis()->SetTitleOffset(1);
   histosignf__2->GetZaxis()->SetTitleFont(42);
   histosignf__2->Draw("same");
   TLatex *   tex = new TLatex(0.2,0.85,"ALICE3 projection, with IRIS, no PID, PbPb #sqrt{s_{NN}}= 5.52 TeV");
tex->SetNDC();
   tex->SetTextAlign(12);
   tex->SetTextFont(42);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.2,0.8,"SHMC_2021, N_{ev} = 273.000000 10^{9}");
tex->SetNDC();
   tex->SetTextAlign(12);
   tex->SetTextFont(42);
   tex->SetTextSize(0.03);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.2,0.75,"#chi_{c} #rightarrow J/#psi #gamma, |y| < 1.44, BR=1.63%");
tex->SetNDC();
   tex->SetTextAlign(12);
   tex->SetTextFont(42);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   canvas->Modified();
   canvas->cd();
   canvas->SetSelected(canvas);
}
