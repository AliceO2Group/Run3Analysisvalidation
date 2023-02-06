//////////////////////////////////////////////////////////////
//  Macro to plot deltaphi projections of two-particle correlations and yields
//
//  For different bins in pT (of trigger and associated particles) plot
//  1) the projection on deltaphi axis of the near-side ridge region (excluding the jet peak)
//    + the fit with Fourier expansion
//  2) the integrated yield as a function of multiplicity
//
//  Input: file with projection histograms produced by yieldExtraction.C macro in this folder
//
//  Usage: root -l plotYield.C
//
//  Parameters:
//  - inFileName: name of the input file
//  - savePlots: flag to save the plots
//
//  Contributors:
//    Katarina Krizkova Gajdosova <katarina.gajdosova@cern.ch>
//    Gian Michele Innocenti <gian.michele.innocenti@cern.ch>
//    Jan Fiete Grosse-Oetringhaus <Jan.Fiete.Grosse-Oetringhaus@cern.ch>
//////////////////////////////////////////////////////////////

float ptTrig[] = {1.0, 2.0, 3.0, 4.0};
float ptAssoc[] = {1.0, 2.0, 3.0, 4.0};
static Double_t Nch[] = {0.0, 2.750, 5.250, 7.750, 12.750, 17.750, 22.750, 27.750, 32.750, 37.750, 42.750, 47.750, 52.750, 57.750, 62.750, 67.750, 72.750, 77.750, 82.750, 87.750, 92.750, 97.750, 250.1};

void drawText(double xmin, double ymin, double textSize, TString text);

//////////////////////////////////////////////////////////////
//  main function
//////////////////////////////////////////////////////////////
void plotYield(const char* inFileName = "yield.root", bool savePlots = false)
{

  gStyle->SetLegendBorderSize(-1);

  TFile* infile = new TFile(inFileName, "read");
  TFile* infileAll = new TFile("yieldAll.root", "read");

  //  plot deltaphi projections in bins of Nch
  const uint ntrig = 1;
  const uint nassoc = 1;
  const uint nmult = 13;
  TCanvas* cdphiRidge[ntrig][nassoc];
  TCanvas* cdphiRidge2[ntrig][nassoc];
  TH1D* hdphiRidge[ntrig][nassoc][nmult];
  TF1* fdphiRidge[ntrig][nassoc][nmult];
  TCanvas* cRidge[ntrig][nassoc];
  TGraphErrors* gRidge[ntrig][nassoc];

  for (uint itrig = 0; itrig < ntrig; itrig++) {
    for (uint iassoc = 0; iassoc < nassoc; iassoc++) {

      //  since we have a lot mult bins, we define 2 canvases with 4x2 pads
      cdphiRidge[itrig][iassoc] = new TCanvas(Form("cdphiRidge_%u_%u", itrig, iassoc), "", 1200, 600);
      cdphiRidge[itrig][iassoc]->Divide(4, 2);
      cdphiRidge2[itrig][iassoc] = new TCanvas(Form("cdphiRidge2_%u_%u", itrig, iassoc), "", 1200, 600);
      cdphiRidge2[itrig][iassoc]->Divide(4, 2);

      for (uint imult = 0; imult < nmult; imult++) {

        //  get the deltaphi projection and its fit (the fit was performed in yieldExtraction.C)
        hdphiRidge[itrig][iassoc][imult] = (TH1D*)infile->Get(Form("proj_dphi_%u_%u_%u", itrig, iassoc, imult));
        fdphiRidge[itrig][iassoc][imult] = (TF1*)infile->Get(Form("fit_%u_%u_%u", itrig, iassoc, imult));

        //  draw a line that shows 0 baseline
        double xbinlowedge = hdphiRidge[itrig][iassoc][imult]->GetBinLowEdge(1);
        double xbinupperedge = hdphiRidge[itrig][iassoc][imult]->GetBinLowEdge(hdphiRidge[itrig][iassoc][imult]->GetNbinsX() + 1);
        TLine* line = new TLine(xbinlowedge, 0, xbinupperedge, 0);
        line->SetLineStyle(2);
        line->SetLineColor(kGray + 2);

        //  first pad of the first canvas will be emtpy, used for legend only
        if (imult == 0) {
          cdphiRidge[itrig][iassoc]->cd(imult + 1);
          drawText(0.2, 0.8, 0.06, Form("%.1f < p_{T,trig} < %.1f", ptTrig[itrig], ptTrig[itrig + 1]));
          drawText(0.2, 0.7, 0.06, Form("%.1f < p_{T,assoc} < %.1f", ptAssoc[iassoc], ptAssoc[iassoc + 1]));

          TLegend* leg = new TLegend(0.2, 0.4, 0.6, 0.65);
          leg->SetTextSize(0.06);
          leg->AddEntry(hdphiRidge[itrig][iassoc][imult], "Data", "lp");
          leg->AddEntry(fdphiRidge[itrig][iassoc][imult], "Fit", "l");
          leg->Draw("same");

          continue;
        }

        // first pad of the second canvas will be empty, used for legend only
        if (imult == 8) {
          cdphiRidge2[itrig][iassoc]->cd((imult + 1) - 8);
          drawText(0.2, 0.8, 0.06, Form("%.1f < p_{T,trig} < %.1f", ptTrig[itrig], ptTrig[itrig + 1]));
          drawText(0.2, 0.7, 0.06, Form("%.1f < p_{T,assoc} < %.1f", ptAssoc[iassoc], ptAssoc[iassoc + 1]));

          TLegend* leg = new TLegend(0.2, 0.4, 0.6, 0.65);
          leg->SetTextSize(0.06);
          leg->AddEntry(hdphiRidge[itrig][iassoc][imult], "Data", "lp");
          leg->AddEntry(fdphiRidge[itrig][iassoc][imult], "Fit", "l");
          leg->Draw("same");
        }

        //  draw the projectionsa and fits on different pads of the same canvas
        if (imult < 8) {
          cdphiRidge[itrig][iassoc]->cd(imult + 1);
          hdphiRidge[itrig][iassoc][imult]->SetStats(0);
          hdphiRidge[itrig][iassoc][imult]->SetTitle("");
          hdphiRidge[itrig][iassoc][imult]->Draw();
          fdphiRidge[itrig][iassoc][imult]->Draw("same");
          line->Draw("same");
          drawText(0.2, 0.8, 0.06, Form("#LT N #GT = %.1f", (Nch[imult + 1] + Nch[imult]) / 2.0));
        } else {
          cdphiRidge2[itrig][iassoc]->cd((imult + 1) - 7);
          hdphiRidge[itrig][iassoc][imult]->SetStats(0);
          hdphiRidge[itrig][iassoc][imult]->SetTitle("");
          hdphiRidge[itrig][iassoc][imult]->Draw();
          fdphiRidge[itrig][iassoc][imult]->Draw("same");
          line->Draw("same");
          drawText(0.2, 0.8, 0.06, Form("#LT N #GT = %.1f", (Nch[imult + 1] + Nch[imult]) / 2.0));
        }

      } // end of loop over multiplicity

      if (savePlots) {
        cdphiRidge[itrig][iassoc]->SaveAs(Form("./plots/dPhiRidge_%u_%u.png", itrig, iassoc));
        cdphiRidge2[itrig][iassoc]->SaveAs(Form("./plots/dPhiRidge2_%u_%u.png", itrig, iassoc));
      }

      //  plot the yield vs. multiplicity
      gRidge[itrig][iassoc] = (TGraphErrors*)infile->Get(Form("ridgeYield_%u_%u", itrig, iassoc));

      cRidge[itrig][iassoc] = new TCanvas(Form("cRidge_%u_%u", itrig, iassoc), "", 700, 500);

      TH1D* histempty = new TH1D("histempty", "", 100, 0, 100);
      histempty->GetXaxis()->SetTitle("#LT N #GT");
      histempty->GetYaxis()->SetTitle("Y_{ridge}");
      histempty->SetMaximum(0.2);
      histempty->SetStats(0);
      histempty->Draw();

      gRidge[itrig][iassoc]->SetTitle("");
      gRidge[itrig][iassoc]->SetMarkerStyle(kFullCircle);
      gRidge[itrig][iassoc]->SetMarkerColor(kBlue + 1);
      gRidge[itrig][iassoc]->SetLineColor(kBlue + 1);
      gRidge[itrig][iassoc]->Draw("Psame");

      if (savePlots)
        cRidge[itrig][iassoc]->SaveAs(Form("./plots/ridge_%u_%u.png", itrig, iassoc));
    }
  }
}
//////////////////////////////////////////////////////////////
void drawText(double xmin, double ymin, double textSize, TString text)
{
  TLatex* tl = new TLatex(xmin, ymin, Form("%s", text.Data()));
  tl->SetNDC();
  tl->SetTextFont(42);
  tl->SetTextSize(textSize);
  tl->Draw();
}
