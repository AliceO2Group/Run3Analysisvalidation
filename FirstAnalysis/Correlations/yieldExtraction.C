bool plotResults = true;
bool savePlots = false;

void yieldExtraction(const char* inFileName = "dphi_corr.root", double absDeltaEtaMin = 1.4, double absDeltaEtaMax = 1.8, const char* outFileName = "yield.root")
{
  //  Nch represents the multiplicity interval of the analysis, here given in ranges of Ntracks. 
  //  TODO: check with Jasper some reasoonable binning
  static Double_t Nch[] = {0.0, 2.750, 5.250, 7.750, 12.750, 17.750, 22.750, 27.750, 32.750, 37.750, 42.750, 47.750, 52.750, 57.750, 62.750, 67.750, 72.750, 77.750, 82.750, 87.750, 92.750, 97.750, 250.1};
  //  Nbins is the number of multiplicity bins
  static const uint Nbins = sizeof(Nch) / sizeof(Nch[0]);
  const double absDeltaPhi = 1.3; // y-projection range (it is only needed for jet yield calculation from delta eta projection)

  TFile* infile = new TFile(inFileName, "read");
  TFile* outfile = new TFile(outFileName, "recreate");
  outfile->cd();

  const uint trigCount = 2; //4
  //TH1D* pltf1[trigCount][trigCount][Nbins] = {0};

  for (uint itrig = 1; itrig < trigCount; ++itrig) {
    for (uint iassoc = 1; iassoc <= itrig; ++iassoc) {

      TGraphErrors* pridgeYield = new TGraphErrors(Nbins);
      TGraphErrors* pfarYield = new TGraphErrors(Nbins);
      TGraphErrors* pfragYield = new TGraphErrors(Nbins);

      for (uint imult = 3; imult < 4; ++imult) {
      //for (uint imult = 0; imult < Nbins; ++imult) {

        // 2D histogram after mixed event subtraction 
        TH2D* hdphidetaRidge = (TH2D*)infile->Get(Form("dphi_%u_%u_%u", itrig, iassoc, imult));
        if (!hdphidetaRidge) {
          printf("No histograms corresponding mult bin %u. (itrig=%u, iassoc=%u)\n", imult, itrig, iassoc);
          continue;
        } // if histogram not existing
        auto c1 = new TCanvas;
        hdphidetaRidge->Draw("colz");

        // Clone hdphidetaJet: hdphidetaRidge will be used for phi projection; hdphidetaJet for eta projection
	      TH2D* hdphidetaJet = (TH2D*)hdphidetaRidge->Clone();

        //  Normalise hdphidetaRidge used for delta phi projection with the width of the long-range region
        double norm = 2.0 * (absDeltaEtaMax - absDeltaEtaMin);
        hdphidetaRidge->Scale(1.0 / norm);
        auto c2 = new TCanvas;
        hdphidetaRidge->Draw("surf");
/*
        //  projection of the away-side ridge on delta eta axis
        int ajetaway = hdphidetaJet->GetXaxis()->FindBin(TMath::Pi() - 1.5);
        int bjetaway = hdphidetaJet->GetXaxis()->FindBin(TMath::Pi() + 1.5);
        TH1D* hdphidetaJetAway = hdphidetaJet->ProjectionY(Form("proj_deta_%u_%u_%u", itrig, iassoc, imult), ajetaway, bjetaway, "e");
        outfile->cd();
        hdphidetaJetAway->Write();

        //  projection of the near-side region (peak+ridge) on delta eta axis
        int ajetpeak = hdphidetaJet->GetXaxis()->FindBin(-1.5);
        int bjetpeak = hdphidetaJet->GetXaxis()->FindBin(+1.5);
        TH1D* hdphidetaJetPeak = hdphidetaJet->ProjectionY(Form("proj_detaJetPeak_%u_%u_%u", itrig, iassoc, imult), ajetpeak, bjetpeak, "e");
        outfile->cd();
        hdphidetaJetPeak->Write();
*/

        //  projection of near-side ridge on delta phi axis (positive side of the jet peak)
        int aridgeP = hdphidetaRidge->GetYaxis()->FindBin(absDeltaEtaMin);
        int bridgeP = hdphidetaRidge->GetYaxis()->FindBin(absDeltaEtaMax);
        TH1D* hdphiRidgeP = hdphidetaRidge->ProjectionX(Form("proj_dphi_P_%u_%u_%u", itrig, iassoc, imult), aridgeP, bridgeP, "e");
        auto c3 = new TCanvas;
        hdphiRidgeP->Draw("ep");
        c3->SaveAs("test.png");
/*
        //  projection of near-side ridge on delta phi axis (negative side of the jet peak)
        int aridgeN = hdphidetaRidge->GetYaxis()->FindBin(-absDeltaEtaMax);
        int bridgeN = hdphidetaRidge->GetYaxis()->FindBin(-absDeltaEtaMin);
        TH1D* hdphiRidgeN = hdphidetaRidge->ProjectionX(Form("proj_dphi_N_%u_%u_%u", itrig, iassoc, imult), aridgeN, bridgeN, "e");
        auto c4 = new TCanvas;
        hdphiRidgeN->Draw();

        //  add the projections positive + negative
        TH1D* hdphiRidge = (TH1D*)hdphiRidgeP->Clone(Form("proj_dphi_%u_%u_%u", itrig, iassoc, imult));
        hdphiRidge->Add(hdphiRidgeP, hdphiRidgeN, 0.5, 0.5);
        //pltf1[itrig][iassoc][imult] = (TH1D*)hdphiRidge->Clone();
        auto c5 = new TCanvas;
        hdphiRidge->Draw();

        //  fit the projection to get ZYAM
        TF1* fdphiRidge = new TF1(Form("fit_%u_%u_%u", itrig, iassoc, imult), "[0]+[1]*(1+2*[2]*TMath::Cos(x)+2*[3]*TMath::Cos(2*x)+2*[4]*TMath::Cos(3*x))");
        fdphiRidge->SetParNames("czyam", "c", "v1", "v2", "v3");
        fdphiRidge->FixParameter(0, 0.0);
        TFitResultPtr r = hdphiRidge->Fit(fdphiRidge, "0QSE", "", -TMath::Pi() / 2.0, 3.0 / 2.0 * TMath::Pi());
        c5->cd();
        fdphiRidge->Draw("same");

        if (plotResults) {
          TCanvas* cdphiRidge = new TCanvas(Form("cdphiRidge_%u_%u_%u", itrig, iassoc, imult), "", 700, 500);
          hdphiRidge->Draw("EP");
          fdphiRidge->Draw("same");
          if (savePlots) cdphiRidge->SaveAs(Form("./plots/dphiproj_%u_%u_%u.png", itrig, iassoc, imult));
        }
*/
/*
        outfile->cd();
        hdphiRidge->Write();

        //  get C_ZYAM: value at bin with minimum
        double phiMinX = fdphiRidge->GetMinimumX(-TMath::Pi() / 2.0, 3.0 / 2.0 * TMath::Pi());
        double phiMin = fdphiRidge->Eval(phiMinX);
        double czyam = phiMin;

        double fitErr;
        r->GetConfidenceIntervals(1, 1, 1, &phiMinX, &fitErr, 0.683, false);

        //  subtract the C_ZYAM
        for (uint idphi = 1; idphi < hdphiRidge->GetXaxis()->GetNbins() + 1; ++idphi) {
          double y = hdphiRidge->GetBinContent(idphi);
          double yerr = hdphiRidge->GetBinError(idphi);
          hdphiRidge->SetBinContent(idphi, TMath::Max(y - czyam, 0.0));
          hdphiRidge->SetBinError(idphi, TMath::Sqrt(yerr * yerr + fitErr * fitErr));
        }	

        int phiIntShift = 0;  //  TODO: check what was the meaning of this

        //  near-side yield integration -> Y_ridge^near
        a = hdphiRidge->GetXaxis()->FindBin(-TMath::Abs(phiMinX)) + phiIntShift;
        b = hdphiRidge->GetXaxis()->FindBin(TMath::Abs(phiMinX)) + phiIntShift;
        double YridgeErr;
        double Yridge = hdphiRidge->IntegralAndError(a, b, YridgeErr, "width");
        printf("<N> = %.1lf, C_ZYAM = %.4f, Yridge = %.4lf pm %.6lf\n", (Nch[imult+1]-Nch[imult])/2.0, czyam, Yridge, YridgeErr);

        pridgeYield->SetPoint(imult, (Nch[imult+1]-Nch[imult])/2.0, Yridge); // saving in the bin center (though later it should be properly corrected)
        pridgeYield->SetPointError(imult, 0.0, YridgeErr);
*/
      } // loop over the index of the multiplicity interval
/*
      if (plotResults) {
        TCanvas *canyield = new TCanvas(Form("canyield_%u_%u", itrig, iassoc), "", 700, 500);
        TH1F *histempty = new TH1F("histempty", "", 100, 0, 100);
        histempty->Draw();
        pridgeYield->Draw("Psame");

        if (savePlots) canyield->SaveAs(Form("./plots/yield_%u_%u.png", itrig, iassoc));
      }

      outfile->cd();
      pridgeYield->Write(Form("ridgeYield_%u_%u", itrig, iassoc)); 
*/
    } // loop over the index of the associated particle
  } // loop over the index of the trigger particle

  outfile->Close();

}// end of processYield
