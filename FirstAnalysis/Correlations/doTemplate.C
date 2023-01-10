///////////////////////////////////////////////////////////////////////////
//  Macro to perform a template fit on 1D histograms of projected correlations on delta phi
//
//  This macro allows you to fit the correlation vs. delta phi and extract V_nDelta coefficients
//  for both reference and differential flow.
//  v_n = sqrt{V_nDelta} -> this is done in the final macro getFlow.C
//
//  Input: file with histograms produced by doPhiProjections.C
//
//  Usage: root -l doTemplate.C
//
//  Parameters:
//  - inputFileName: input file
//  - outputFileName: output file containing V_nDelta results
//  - outputPlotsName: name of the folder to store plots
//  - drawSeparatevn: flag to draw the template fit result for v2 and v3 separately instead of sum of vn
//  - drawTemplate: flag to draw the result of the template fit with the correlation
//  - savePlots: flag to save drawn plots
//
//  Contributors:
//    Katarina Krizkova Gajdosova <katarina.gajdosova@cern.ch>
//    Gian Michele Innocenti <gian.michele.innocenti@cern.ch>
///////////////////////////////////////////////////////////////////////////

int nBinspTtrig = 6;
double binspTtrig[] = {0.2, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0};

int nBinspTref = 1;
double binspTref[] = {0.2, 3.0};

const int nBinsMult = 7;
double binsMult[] = {0, 10, 20, 30, 40, 50, 60, 80, 100, 200};

TFile* outputFile;

TH1D *hminuit, *hminuit_periph;

TH1D* hDifferentialV2[nBinsMult];

void tempminuit(double* fParamVal, double* fParamErr);
void minuitfcn(int& npar, double* gin, double& ff, double* par, int iflag);
double fun_template(double* x, double* par);

///////////////////////////////////////////////////////////////////////////
//  Main function
///////////////////////////////////////////////////////////////////////////
void doTemplate(
  const char* inputFileName = "./phi_proj.root",
  const char* outputFileName = "./templateResult.root",
  const char* outputPlotsName = "./plots",
  bool drawSeparatevn = false,
  bool drawTemplate = true,
  bool savePlots = true)
{

  TFile* inFile = TFile::Open(Form("%s", inputFileName), "read");

  //  V_nDelta (v_2^2) for reference flow vs. multiplicity
  TH1D* hReferenceV2 = new TH1D("hReferenceV2", "v_{2#delta}; Multiplicity; v_{2#Delta}", nBinsMult, binsMult);

  //  perform template for reference and differential flow in respective bins of pTtrig, pTassoc and mult
  for (int iMult = 0; iMult < nBinsMult; iMult++) {

    //  do reference flow here
    //  get the histograms projected to delta phi,
    //  we need to distinguish histogram with desired (high) multiplicity
    //  from a histogram with low multiplicity used as the peripheral baseline
    hminuit = (TH1D*)inFile->Get(Form("proj_dphi_ref_%d", iMult))->Clone("hminuit");
    hminuit_periph = (TH1D*)inFile->Get("proj_dphi_ref_0")->Clone("hminuit_periph");

    //  do the template fit
    double par[4], parerr[4];
    tempminuit(par, parerr);

    //  fill the resulting V_2delta into histogram vs. pT
    hReferenceV2->SetBinContent(iMult + 1, par[2]);
    hReferenceV2->SetBinError(iMult + 1, parerr[2]);

    //  draw the result of the template fit
    if (drawTemplate) {

      //  F*Y_peripheral(deltaphi) + Y_ridge = template fit
      TF1* fTemplate = new TF1("fTemplate", fun_template, -0.5 * TMath::Pi() + 1e-6, 1.5 * TMath::Pi() - 1e-6, 4);
      fTemplate->SetParameters(par); //  set the parameters obtained from template fit above using tempminuit()
      //fTemplate->SetLineStyle(kSolid);
      //fTemplate->SetLineColor(kBlue+1);

      TH1F* hTemplate = (TH1F*)hminuit->Clone();
      hTemplate->Reset();
      for (int iBin = 1; iBin < hTemplate->GetNbinsX() + 1; iBin++) {
        hTemplate->SetBinContent(iBin, fTemplate->Eval(hTemplate->GetBinCenter(iBin)));
      }
      hTemplate->SetLineColor(kBlue + 1);
      hTemplate->SetLineWidth(3);
      hTemplate->SetLineStyle(kSolid);

      // F*Y_peripheral(0) + Y_ridge  v2
      TF1* fRidge;
      if (drawSeparatevn) {
        fRidge = new TF1("fRidge", "[0]*[3] + [1]*(1 + 2*[2]*cos(2*x))", -5, 5);
      } else {
        fRidge = new TF1("fRidge", "[0]*[4] + [1]*(1 + 2*[2]*cos(2*x) + 2*[3]*cos(3*x))", -5, 5);
      }
      fRidge->SetParameter(0, par[0]);                                                       // F
      fRidge->SetParameter(1, par[1]);                                                       // G
      fRidge->SetParameter(2, par[2]);                                                       // v_2^2
      fRidge->SetParameter(3, par[3]);                                                       // v_3^2
      fRidge->SetParameter(4, hminuit_periph->GetBinContent(hminuit_periph->FindFixBin(0))); // Y_peripheral(0)
      fRidge->SetLineStyle(kSolid);
      fRidge->SetLineWidth(3);
      fRidge->SetLineColor(kRed + 1);

      if (drawSeparatevn) {
        // F*Y_peripheral(0) + Y_ridge  v3
        TF1* fRidgev3 = new TF1("fRidgev3", "[0]*[3] + [1]*(1 + 2*[2]*cos(3*x))", -5, 5);
        fRidgev3->SetParameter(0, par[0]);                                                       // F
        fRidgev3->SetParameter(1, par[1]);                                                       // G
        fRidgev3->SetParameter(2, par[3]);                                                       // v_3^2
        fRidgev3->SetParameter(3, hminuit_periph->GetBinContent(hminuit_periph->FindFixBin(0))); // Y_peripheral(0)
        fRidgev3->SetLineStyle(kDashed);
        fRidgev3->SetLineWidth(3);
        fRidgev3->SetLineColor(kRed + 1);
      }

      //  F*Y_peripheral(deltaphi) + G
      TF1* fPeripheral = new TF1("fPeripheral", fun_template, -0.5 * TMath::Pi() + 1e-6, 1.5 * TMath::Pi() - 1e-6, 5);
      par[2] = 0; // v2^2 = 0
      par[3] = 0; // v3^2 = 0
      fPeripheral->SetParameters(par);
      fPeripheral->SetLineStyle(kSolid);
      fPeripheral->SetLineWidth(3);
      fPeripheral->SetLineColor(kGreen + 2);

      TH1F* hPeripheral = (TH1F*)hminuit->Clone();
      hPeripheral->Reset();
      for (int iBin = 1; iBin < hPeripheral->GetNbinsX() + 1; iBin++) {
        hPeripheral->SetBinContent(iBin, fPeripheral->Eval(hPeripheral->GetBinCenter(iBin)));
      }
      hPeripheral->SetLineColor(kGreen + 2);
      hPeripheral->SetLineWidth(2);
      hPeripheral->SetLineStyle(kSolid);
      hPeripheral->SetMarkerStyle(kOpenSquare);
      hPeripheral->SetMarkerColor(kGreen + 2);
      hPeripheral->SetMarkerSize(1.2);

      //  draw the HM projection with the template fits together
      TCanvas* cTemplate = new TCanvas("cTemplate", "", 1200, 800);
      gPad->SetMargin(0.12, 0.01, 0.12, 0.01);
      hminuit->SetTitle("");
      hminuit->SetStats(0);
      hminuit->GetYaxis()->SetTitleOffset(1.1);
      hminuit->GetXaxis()->SetTitleSize(0.05);
      hminuit->GetYaxis()->SetTitle("Y(#Delta#varphi)");
      hminuit->GetYaxis()->SetTitleSize(0.05);
      hminuit->SetLineColor(kBlack);
      hminuit->SetMarkerStyle(kFullCircle);
      hminuit->SetMarkerColor(kBlack);
      hminuit->SetMarkerSize(1.4);
      hminuit->Draw("");
      hTemplate->Draw("hist same");
      hPeripheral->Draw("hist same");
      fRidge->Draw("same");
      if (drawSeparatevn) {
        fRidgev3->Draw("same");
      }

      TLegend* legend = new TLegend(0.15, 0.55, 0.5, 0.75);
      legend->SetFillColor(0);
      legend->SetBorderSize(0);
      legend->SetTextSize(0.035);

      legend->AddEntry(hTemplate, "FY(#Delta#varphi)^{peri} + G(1 + #Sigma_{n=2}^{3} 2V_{n#Delta} cos(n#Delta#varphi))", "L");
      legend->AddEntry(hPeripheral, "FY(#Delta#varphi)^{peri} + G", "L");
      legend->AddEntry(fRidge, "FY(0)^{peri} + G(1 + #Sigma_{n=2}^{3} 2V_{n#Delta} cos(n#Delta#varphi))", "L");
      legend->Draw("same");

      TLatex* latex = 0;
      latex = new TLatex();
      latex->SetTextSize(0.038);
      latex->SetTextFont(42);
      latex->SetTextAlign(21);
      latex->SetNDC();

      latex->DrawLatex(0.3, 0.93, "pp #sqrt{s} = 13 TeV");
      latex->DrawLatex(0.3, 0.86, Form("%.1f < p_{T, trig, assoc} < %.1f", binspTref[0], binspTref[1]));
      latex->DrawLatex(0.3, 0.79, Form("%.1f < N_{ch} < %.1f", binsMult[iMult], binsMult[iMult + 1]));
      //latex->DrawLatex(0.3,0.72,Form("%.1f < #Delta#eta < %.1f",etaMin,etaMax));

      if (savePlots)
        cTemplate->SaveAs(Form("%s/template_ref_%d.png", outputPlotsName, iMult));
    }

    //  do differential flow here
    //  V_nDelta (v_2^2) for differential flow vs. pT and in bins of multiplicity
    hDifferentialV2[iMult] = new TH1D(Form("hDifferentialV2_%d", iMult), "v_{2#delta}; p_T; v_{2#Delta}", nBinspTtrig, binspTtrig);

    for (int ipTtrig = 0; ipTtrig < nBinspTtrig; ipTtrig++) {

      //  get the histograms projected to delta phi,
      //  we need to distinguish histogram with desired (high) multiplicity
      //  from a histogram with low multiplicity used as the peripheral baseline
      hminuit = (TH1D*)inFile->Get(Form("proj_dphi_%d_0_%d", ipTtrig, iMult))->Clone("hminuit");
      hminuit_periph = (TH1D*)inFile->Get(Form("proj_dphi_%d_0_0", ipTtrig))->Clone("hminuit_periph");

      //  do the template fit
      double par[4], parerr[4];
      tempminuit(par, parerr);

      //  fill the resulting V_2delta into histogram vs. pT
      hDifferentialV2[iMult]->SetBinContent(ipTtrig + 1, par[2]);
      hDifferentialV2[iMult]->SetBinError(ipTtrig + 1, parerr[2]);

      //  draw the result of the template fit
      if (drawTemplate) {

        //  F*Y_peripheral(deltaphi) + Y_ridge = template fit
        TF1* fTemplate = new TF1("fTemplate", fun_template, -0.5 * TMath::Pi() + 1e-6, 1.5 * TMath::Pi() - 1e-6, 4);
        fTemplate->SetParameters(par); //  set the parameters obtained from template fit above using tempminuit()
        fTemplate->SetLineStyle(kSolid);
        fTemplate->SetLineColor(kRed);

        TH1F* hTemplate = (TH1F*)hminuit->Clone();
        hTemplate->Reset();
        for (int iBin = 1; iBin < hTemplate->GetNbinsX() + 1; iBin++) {
          hTemplate->SetBinContent(iBin, fTemplate->Eval(hTemplate->GetBinCenter(iBin)));
        }
        hTemplate->SetLineColor(kBlue + 1);
        hTemplate->SetLineWidth(3);
        hTemplate->SetLineStyle(kSolid);

        // F*Y_peripheral(0) + Y_ridge
        TF1* fRidge = new TF1("fRidge", "[0]*[4] + [1]*(1 + 2*[2]*cos(2*x) + 2*[3]*cos(3*x))", -5, 5);
        fRidge->SetParameter(0, par[0]);                                                       // F
        fRidge->SetParameter(1, par[1]);                                                       // G
        fRidge->SetParameter(2, par[2]);                                                       // v_2^2
        fRidge->SetParameter(3, par[3]);                                                       // v_3^2
        fRidge->SetParameter(4, hminuit_periph->GetBinContent(hminuit_periph->FindFixBin(0))); // Y_peripheral(0)
        fRidge->SetLineStyle(kSolid);
        fRidge->SetLineWidth(3);
        fRidge->SetLineColor(kRed + 1);

        //  F*Y_peripheral(deltaphi) + G
        TF1* fPeripheral = new TF1("fPeripheral", fun_template, -0.5 * TMath::Pi() + 1e-6, 1.5 * TMath::Pi() - 1e-6, 5);
        par[2] = 0; // v2^2 = 0
        par[3] = 0; // v3^2 = 0
        fPeripheral->SetParameters(par);
        fPeripheral->SetLineStyle(kSolid);
        fPeripheral->SetLineColor(kMagenta);

        TH1F* hPeripheral = (TH1F*)hminuit->Clone();
        hPeripheral->Reset();
        for (int iBin = 1; iBin < hPeripheral->GetNbinsX() + 1; iBin++) {
          hPeripheral->SetBinContent(iBin, fPeripheral->Eval(hPeripheral->GetBinCenter(iBin)));
        }
        hPeripheral->SetLineColor(kGreen + 2);
        hPeripheral->SetLineWidth(2);
        hPeripheral->SetLineStyle(kSolid);
        hPeripheral->SetMarkerStyle(kOpenSquare);
        hPeripheral->SetMarkerColor(kGreen + 2);
        hPeripheral->SetMarkerSize(1.2);

        //  draw the HM projection with the template fits together
        TCanvas* cTemplate = new TCanvas("cTemplate", "", 1200, 800);
        gPad->SetMargin(0.12, 0.01, 0.12, 0.01);
        hminuit->SetTitle("");
        hminuit->SetStats(0);
        hminuit->GetYaxis()->SetTitleOffset(1.1);
        hminuit->GetXaxis()->SetTitleSize(0.05);
        hminuit->GetYaxis()->SetTitle("Y(#Delta#varphi)");
        hminuit->GetYaxis()->SetTitleSize(0.05);
        hminuit->SetLineColor(kBlack);
        hminuit->SetMarkerStyle(kFullCircle);
        hminuit->SetMarkerColor(kBlack);
        hminuit->SetMarkerSize(1.4);
        hminuit->Draw("");
        hTemplate->Draw("hist same");
        hPeripheral->Draw("hist same");
        fRidge->Draw("same");

        TLegend* legend = new TLegend(0.15, 0.55, 0.5, 0.75);
        legend->SetFillColor(0);
        legend->SetBorderSize(0);
        legend->SetTextSize(0.035);

        legend->AddEntry(hTemplate, "FY(#Delta#varphi)^{peri} + G(1 + #Sigma_{n=2}^{3} 2V_{n#Delta} cos(n#Delta#varphi))", "L");
        legend->AddEntry(hPeripheral, "FY(#Delta#varphi)^{peri} + G", "L");
        legend->AddEntry(fRidge, "FY(0)^{peri} + G(1 + #Sigma_{n=2}^{3} 2V_{n#Delta} cos(n#Delta#varphi))", "L");
        legend->Draw("same");

        TLatex* latex = 0;
        latex = new TLatex();
        latex->SetTextSize(0.038);
        latex->SetTextFont(42);
        latex->SetTextAlign(21);
        latex->SetNDC();

        latex->DrawLatex(0.3, 0.93, "pp #sqrt{s} = 13 TeV");
        latex->DrawLatex(0.3, 0.86, Form("%.1f < p_{T, trig} < %.1f", binspTtrig[ipTtrig], binspTtrig[ipTtrig + 1]));
        latex->DrawLatex(0.3, 0.79, Form("%.1f < N_{ch} < %.1f", binsMult[iMult], binsMult[iMult + 1]));
        //latex->DrawLatex(0.3,0.72,Form("%.1f < #Delta#eta < %.1f",etaMin,etaMax));

        if (savePlots)
          cTemplate->SaveAs(Form("%s/template_%d_%d.png", outputPlotsName, ipTtrig, iMult));
      }
    } // end of pT trig loop
  }   // end of mult loop

  //  save the results
  outputFile = new TFile(Form("%s", outputFileName), "recreate");
  hReferenceV2->Write();
  for (int iMult = 0; iMult < nBinsMult; iMult++) {
    hDifferentialV2[iMult]->Write();
  }
  outputFile->Close();

} // end of main function

///////////////////////////////////////////////////////////////////////////
//  template fit function used for drawing the result of the minimization
///////////////////////////////////////////////////////////////////////////
double fun_template(double* x, double* par)
{
  float xx = x[0];

  int iBin = hminuit_periph->GetXaxis()->FindBin(xx);
  double Y_periph = hminuit_periph->GetBinContent(iBin);

  //  Explanation of the parameters:
  //    par[0] = F
  //    Y_periph = Y_peripheral (delta phi)
  //    par[3] = G
  //    par[1] = V_2delta = v_2,2 = v_2^2
  //    par[2] = V_3delta = v_3,3 = v_3^2
  return par[0] * Y_periph + par[1] * (1 + 2 * par[2] * cos(2 * xx) + 2 * par[3] * cos(3 * xx));
}

///////////////////////////////////////////////////////////////////////////
//  this function provides the first initial value of chi2
//  to start the minimization procedure done in tempminuit()
///////////////////////////////////////////////////////////////////////////
void minuitfcn(int& npar, double* gin, double& ff, double* par, int iflag)
{
  TH1D* h = (TH1D*)hminuit->Clone("h");
  TH1D* hperi = (TH1D*)hminuit_periph->Clone("hperi");

  double f = par[0];
  double gv = par[1];
  double v2 = par[2];
  double v3 = par[3];

  double lnQ = 0;

  //  get chi2 as a starting point
  for (int ibin = 1; ibin < h->GetNbinsX(); ibin++) {
    double x = h->GetBinCenter(ibin + 1);                                                                                                  // delta phi
    double data = h->GetBinContent(ibin + 1);                                                                                              // value of the correlation (high-mult)
    double exp = f * hperi->GetBinContent(ibin + 1) + gv * (1. + 2. * v2 * cos(2. * x) + 2. * v3 * cos(3. * x));                           // template fit prescription
    double hoge = sqrt(hperi->GetBinError(ibin + 1) * hperi->GetBinError(ibin + 1) + h->GetBinError(ibin + 1) * h->GetBinError(ibin + 1)); // error:sqrt(sig*sig + peri*peri)
    lnQ += (data - exp) * (data - exp) / (hoge * hoge);                                                                                    // chi2
  }

  ff = lnQ;
}

///////////////////////////////////////////////////////////////////////////
//  this function performs the minimization to obtain template fit
//  at the beginning it needs to be provided with initial values
//  of parameters and a chi2 (chi2 value is provided with minuitfcn())
///////////////////////////////////////////////////////////////////////////
void tempminuit(double* fParamVal, double* fParamErr)
{
  TFitter* minimizer = new TFitter(4);
  minimizer->SetFCN(minuitfcn); // set some initial chi2 value?
  minimizer->SetParameter(0, "F", 0, 10, 0, 0);
  minimizer->SetParameter(1, "G", 0, 10, 0, 0);
  minimizer->SetParameter(2, "v2", 0, 1, 0, 0);
  minimizer->SetParameter(3, "v3", 0, 1, 0, 0);

  //  minimizer->ExecuteCommand("SIMPLEX",0,0);
  minimizer->ExecuteCommand("MIGRAD", 0, 0);

  fParamVal[0] = minimizer->GetParameter(0); // F
  fParamVal[1] = minimizer->GetParameter(1); // G
  fParamVal[2] = minimizer->GetParameter(2); // v2
  fParamVal[3] = minimizer->GetParameter(3); // v3

  fParamErr[0] = minimizer->GetParError(0);
  fParamErr[1] = minimizer->GetParError(1);
  fParamErr[2] = minimizer->GetParError(2);
  fParamErr[3] = minimizer->GetParError(3);

  cout << "F  = " << fParamVal[0] << "+/-" << fParamErr[0] << "\n";
  cout << "G  = " << fParamVal[1] << "+/-" << fParamErr[1] << "\n";
  cout << "v2 = " << fParamVal[2] << "+/-" << fParamErr[2] << "\n";
  cout << "v3 = " << fParamVal[3] << "+/-" << fParamErr[3] << "\n";

  delete minimizer;
}
