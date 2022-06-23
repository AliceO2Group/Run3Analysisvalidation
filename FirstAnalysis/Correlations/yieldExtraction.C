bool plotResults = true;
bool savePlots = false;

// Note:  if a canvas is drawn with an empty pad, it is probably because 
//        the TFile took ownership of the histogram and deleted it when 
//        it went out of scope 
//        To fix it, call h->SetDirectory(0) before drawing

void yieldExtraction(const char* inFileName = "dphi_corr.root", double absDeltaEtaMin = 1.4, double absDeltaEtaMax = 1.8, const char* outFileName = "yield.root")
{
  //  Nch represents the multiplicity interval of the analysis, here given in ranges of Ntracks. 
  //  TODO: check with Jasper some reasoonable binning
  static Double_t Nch[] = {0.0, 2.750, 5.250, 7.750, 12.750, 17.750, 22.750, 27.750, 32.750, 37.750, 42.750, 47.750, 52.750, 57.750, 62.750, 67.750, 72.750, 77.750, 82.750, 87.750, 92.750, 97.750, 250.1};
  //  Nbins is the number of multiplicity bins
  static const uint Nbins = 14; // tmp for the case when I had low stat.
  //static const uint Nbins = sizeof(Nch) / sizeof(Nch[0]);
  const double absDeltaPhi = 1.3; // y-projection range (it is only needed for jet yield calculation from delta eta projection)

  TFile* infile = new TFile(inFileName, "read");
  TFile* outfile = new TFile(outFileName, "recreate");

  const uint trigCount = 2; //4

  for (uint itrig = 1; itrig < trigCount; ++itrig) {
    for (uint iassoc = 1; iassoc <= itrig; ++iassoc) {

      TGraphErrors* gridgeYield = new TGraphErrors(Nbins);
      TGraphErrors* pfarYield = new TGraphErrors(Nbins);
      TGraphErrors* pfragYield = new TGraphErrors(Nbins);

      for (uint imult = 0; imult < Nbins; ++imult) {

        // 2D histogram after mixed event subtraction 
        TH2D* hdphidetaRidge = (TH2D*)infile->Get(Form("dphi_%u_%u_%u", itrig, iassoc, imult));
        if (!hdphidetaRidge) {
          printf("No histograms corresponding mult bin %u. (itrig=%u, iassoc=%u)\n", imult, itrig, iassoc);
          continue;
        } // if histogram not existing

        // Clone hdphidetaJet: hdphidetaRidge will be used for phi projection; hdphidetaJet for eta projection
	      TH2D* hdphidetaJet = (TH2D*)hdphidetaRidge->Clone();

        //  Normalise hdphidetaRidge used for delta phi projection with the width of the long-range region
        double norm = 2.0 * (absDeltaEtaMax - absDeltaEtaMin);
        hdphidetaRidge->Scale(1.0 / norm);

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

        //  projection of near-side ridge on delta phi axis (positive side of the jet peak)
        int aridgeP = hdphidetaRidge->GetYaxis()->FindBin(absDeltaEtaMin);
        int bridgeP = hdphidetaRidge->GetYaxis()->FindBin(absDeltaEtaMax);
        TH1D* hdphiRidgeP = hdphidetaRidge->ProjectionX(Form("proj_dphi_P_%u_%u_%u", itrig, iassoc, imult), aridgeP, bridgeP, "e");
        outfile->cd();
        hdphiRidgeP->Write();

        //  projection of near-side ridge on delta phi axis (negative side of the jet peak)
        int aridgeN = hdphidetaRidge->GetYaxis()->FindBin(-absDeltaEtaMax);
        int bridgeN = hdphidetaRidge->GetYaxis()->FindBin(-absDeltaEtaMin);
        TH1D* hdphiRidgeN = hdphidetaRidge->ProjectionX(Form("proj_dphi_N_%u_%u_%u", itrig, iassoc, imult), aridgeN, bridgeN, "e");
        outfile->cd();
        hdphiRidgeN->Write();

        //  add the projections positive + negative
        TH1D* hdphiRidge = (TH1D*)hdphiRidgeP->Clone(Form("proj_dphi_%u_%u_%u", itrig, iassoc, imult));
        hdphiRidge->Add(hdphiRidgeP, hdphiRidgeN, 0.5, 0.5);
        outfile->cd();
        hdphiRidge->Write();

        //  fit the projection to get ZYAM
        TF1* fdphiRidge = new TF1(Form("fit_%u_%u_%u", itrig, iassoc, imult), 
                                  "[0]+[1]*(1+2*[2]*TMath::Cos(x)+2*[3]*TMath::Cos(2*x)+2*[4]*TMath::Cos(3*x))", 
                                  -TMath::Pi() / 2.0, 3.0 / 2.0 * TMath::Pi());
        fdphiRidge->SetParNames("czyam", "c", "v1", "v2", "v3");
        fdphiRidge->FixParameter(0, 0.0); // TODO: this is because otherwise it could bias the C_ZYAM extraction? the result doesn't change much
        TFitResultPtr r = hdphiRidge->Fit(fdphiRidge, "0SE", "", -TMath::Pi() / 2.0, 3.0 / 2.0 * TMath::Pi());
        //printf("fit parameter: %.4f \n", fdphiRidge->GetParameter(0));
        outfile->cd();
        fdphiRidge->Write();

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
        int aridge = hdphiRidge->GetXaxis()->FindBin(-TMath::Abs(phiMinX)) + phiIntShift;
        int bridge = hdphiRidge->GetXaxis()->FindBin(TMath::Abs(phiMinX)) + phiIntShift;
        double YridgeErr;
        double Yridge = hdphiRidge->IntegralAndError(aridge, bridge, YridgeErr, "width");
        printf("<N> = %.1lf, C_ZYAM = %.4f, Yridge = %.4lf pm %.6lf\n", (Nch[imult+1]+Nch[imult])/2.0, czyam, Yridge, YridgeErr);

        gridgeYield->SetPoint(imult, (Nch[imult+1]+Nch[imult])/2.0, Yridge); // saving in the bin center (though later it should be properly corrected)
        gridgeYield->SetPointError(imult, 0.0, YridgeErr);

      } // loop over the index of the multiplicity interval

      outfile->cd();
      gridgeYield->Write(Form("ridgeYield_%u_%u", itrig, iassoc)); 

    } // loop over the index of the associated particle
  } // loop over the index of the trigger particle

  outfile->Close();

}// end of processYield
