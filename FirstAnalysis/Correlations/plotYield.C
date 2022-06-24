float ptTrig[] = {0.5, 1.0, 2.0, 3.0};
float ptAssoc[] = {0.5, 1.0, 2.0, 3.0};
static Double_t Nch[] = {0.0, 2.750, 5.250, 7.750, 12.750, 17.750, 22.750, 27.750, 32.750, 37.750, 42.750, 47.750, 52.750, 57.750, 62.750, 67.750, 72.750, 77.750, 82.750, 87.750, 92.750, 97.750, 250.1};

void DrawText(double xmin, double ymin, double textSize, TString text);

void plotYield(const char* inFileName = "yield.root", bool savePlots = false)
{

  gStyle->SetLegendBorderSize(-1);

  TFile *infile = new TFile(inFileName, "read");
  
  const uint ntrig = 2;
  const uint nassoc = 2;
  const uint nmult = 14;
  TCanvas* cdphiRidge[ntrig][nassoc];
  TH1D* hdphiRidge[ntrig][nassoc][nmult];
  TF1* fdphiRidge[ntrig][nassoc][nmult];
  TCanvas* cRidge[ntrig][nassoc];
  TGraphErrors* gRidge[ntrig][nassoc];

  for (uint itrig = 1; itrig < ntrig; itrig++) {
    for (uint iassoc = 1; iassoc < nassoc; iassoc++) {

      cdphiRidge[itrig][iassoc] = new TCanvas(Form("cdphiRidge_%u_%u", itrig, iassoc), "", 1200, 600);
      cdphiRidge[itrig][iassoc]->Divide(4,2);

      for (uint imult = 0; imult < nmult; imult++) {

        hdphiRidge[itrig][iassoc][imult] = (TH1D*)infile->Get(Form("proj_dphi_%u_%u_%u", itrig, iassoc, imult));
        fdphiRidge[itrig][iassoc][imult] = (TF1*)infile->Get(Form("fit_%u_%u_%u", itrig, iassoc, imult));

        double xbinlowedge = hdphiRidge[itrig][iassoc][imult]->GetBinLowEdge(1);
        double xbinupperedge = hdphiRidge[itrig][iassoc][imult]->GetBinLowEdge(hdphiRidge[itrig][iassoc][imult]->GetNbinsX()+1);
        TLine *line = new TLine(xbinlowedge, 0, xbinupperedge, 0);
        line->SetLineStyle(2);
        line->SetLineColor(kGray+2);

        if (imult == 0) {
          cdphiRidge[itrig][iassoc]->cd(imult+1);
          DrawText(0.2, 0.8, 0.06, Form("%.1f < p_{T,trig} < %.1f", ptTrig[itrig], ptTrig[itrig+1]));
          DrawText(0.2, 0.7, 0.06, Form("%.1f < p_{T,assoc} < %.1f", ptAssoc[iassoc], ptAssoc[iassoc+1]));

          TLegend *leg = new TLegend(0.2, 0.4, 0.6, 0.65);
          leg->SetTextSize(0.06);
          leg->AddEntry(hdphiRidge[itrig][iassoc][imult], "Data", "lp");
          leg->AddEntry(fdphiRidge[itrig][iassoc][imult], "Fit", "l");
          leg->Draw("same");

          continue;
        }

        cdphiRidge[itrig][iassoc]->cd(imult+1);
        hdphiRidge[itrig][iassoc][imult]->SetStats(0);
        hdphiRidge[itrig][iassoc][imult]->SetTitle("");
        hdphiRidge[itrig][iassoc][imult]->Draw();
        fdphiRidge[itrig][iassoc][imult]->Draw("same");
        line->Draw("same");

        DrawText(0.2, 0.8, 0.06, Form("#LT N #GT = %.1f", (Nch[imult+1]+Nch[imult])/2.0));

      }// end of loop over multiplicity

      //  plot the yield
      gRidge[itrig][iassoc] = (TGraphErrors*)infile->Get(Form("ridgeYield_%u_%u", itrig, iassoc));

      cRidge[itrig][iassoc] = new TCanvas(Form("cRidge_%u_%u", itrig, iassoc), "", 700, 500);            

      TH1D *histempty = new TH1D("histempty", "", 100, 0, 100);
      histempty->GetXaxis()->SetTitle("#LT N #GT");
      histempty->GetYaxis()->SetTitle("Y_{ridge}");
      histempty->SetMaximum(0.2);
      histempty->SetStats(0);
      histempty->Draw();

      gRidge[itrig][iassoc]->SetTitle("");
      gRidge[itrig][iassoc]->SetMarkerStyle(kFullCircle);
      gRidge[itrig][iassoc]->SetMarkerColor(kBlue+1);
      gRidge[itrig][iassoc]->SetLineColor(kBlue+1);
      gRidge[itrig][iassoc]->Draw("Psame");

    }
  }

}
//---------------------
void DrawText(double xmin, double ymin, double textSize, TString text)
{
  TLatex *tl = new TLatex(xmin, ymin, Form("%s", text.Data()));
	tl->SetNDC();
	tl->SetTextFont(42);
	tl->SetTextSize(textSize);
	tl->Draw();
}
