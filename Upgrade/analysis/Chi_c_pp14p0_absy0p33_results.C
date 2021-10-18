void Chi_c_pp14p0_absy0p33_results()
{
//=========Macro generated from canvas: canvas/A Simple Graph Example
//=========  (Mon Oct 18 11:24:12 2021) by ROOT version 6.20/08
   TCanvas *canvas = new TCanvas("canvas", "A Simple Graph Example",779,196,667,636);
   gStyle->SetOptStat(0);
   canvas->ToggleEventStatus();
   canvas->SetHighLightColor(2);
   canvas->Range(-1.25,-1.625,11.25,4.625);
   canvas->SetFillColor(0);
   canvas->SetBorderMode(0);
   canvas->SetBorderSize(2);
   canvas->SetLogy();
   canvas->SetFrameBorderMode(0);
   canvas->SetFrameBorderMode(0);
   
   TH2F *hempty__1 = new TH2F("hempty__1","",100,0,10,100,0.1,10000);
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
   histosignf__2->SetBinContent(1,2.913476);
   histosignf__2->SetBinContent(2,10.88932);
   histosignf__2->SetBinContent(3,50.82542);
   histosignf__2->SetBinContent(4,64.56687);
   histosignf__2->SetBinContent(5,56.56986);
   histosignf__2->SetBinContent(6,45.24265);
   histosignf__2->SetBinContent(7,50.16793);
   histosignf__2->SetBinContent(8,52.5973);
   histosignf__2->SetBinContent(9,45.29062);
   histosignf__2->SetBinContent(10,2.147081e-09);
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
   TLatex *   tex = new TLatex(0.2,0.85,"ALICE3 projection, with IRIS, no PID, pp #sqrt{s}= 14 TeV");
tex->SetNDC();
   tex->SetTextAlign(12);
   tex->SetTextFont(42);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.2,0.8,"Pythia 8, monash tune, N_{ev} = 1260000.000000 10^{9}");
tex->SetNDC();
   tex->SetTextAlign(12);
   tex->SetTextFont(42);
   tex->SetTextSize(0.03);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.2,0.75,"#chi_{c1} #rightarrow J/#psi #gamma, |y| < 0.33, BR=2.04%");
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
