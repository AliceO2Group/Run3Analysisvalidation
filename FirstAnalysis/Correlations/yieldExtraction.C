void processYield(const char* pinFileFname = "dphi_corr.root", double absDeltaEtaMin = 1.4, double absDeltaEtaMax = 1.8, const char* poutFileName = "yield.root", uint flags = 0)
{
  // Nct represents the multiplicity interval of the analysis, here given in ranges of Ntracks. 
  static Double_t Nct[] = {2.5, 8.0, 13.0, 20.5};
  // Nbins is the number of multiplicity bins
  static const uint Nbins = sizeof(Nct) / sizeof(Nct[0]);
  const double absDeltaPhi = 1.3; // y-projection range

  TFile* pf = new TFile(pinFileFname, "read");
  TFile* pfout = new TFile(poutFileName, "recreate");
  pfout->cd();

  const uint trigCount = 2;
  TH1D* pltf1[trigCount][trigCount][Nbins] = {0};

  for (uint itrig = 0; itrig < trigCount; ++itrig) {
    for (uint iassoc = 0; iassoc <= itrig; ++iassoc) {
      TGraphErrors* pridgeYield = new TGraphErrors(Nbins);
      TGraphErrors* pfarYield = new TGraphErrors(Nbins);
      TGraphErrors* pfragYield = new TGraphErrors(Nbins);
      for (uint ib = 0; ib < Nbins; ++ib) {
        TH2D* ph = (TH2D*)pf->Get(Form("dphi_%u_%u_%u", itrig, iassoc, ib));
        if (!ph) {
          printf("No histograms corresponding mult bin %u. (itrig=%u, iassoc=%u)\n", ib, itrig, iassoc);
          continue;
        } // if histogram not existing

        // phe is the 2D histogram after mixed event subtraction 
	TH2D* phe = (TH2D*)ph->Clone();

        // double norm = ph->GetYaxis()->GetBinWidth(1);
        double norm = 2.0 * (absDeltaEtaMax - absDeltaEtaMin);
        ph->Scale(1.0 / norm);

        int a = phe->GetXaxis()->FindBin(TMath::Pi() - 1.5);
        int b = phe->GetXaxis()->FindBin(TMath::Pi() + 1.5);
        TH1D* phe0 = phe->ProjectionY(Form("proj_deta_%u_%u_%u", itrig, iassoc, ib), a, b, "e");
        phe0->Write();

        a = phe->GetXaxis()->FindBin(-1.5);
        b = phe->GetXaxis()->FindBin(+1.5);
        TH1D* phe0JetPeak = phe->ProjectionY(Form("proj_detaJetPeak_%u_%u_%u", itrig, iassoc, ib), a, b, "e");
        phe0JetPeak->Write();

        a = ph->GetYaxis()->FindBin(absDeltaEtaMin);
        b = ph->GetYaxis()->FindBin(absDeltaEtaMax);
        TH1D* pp = ph->ProjectionX(Form("proj_dphi_P_%u_%u_%u", itrig, iassoc, ib), a, b, "e");

        a = ph->GetYaxis()->FindBin(-absDeltaEtaMax);
        b = ph->GetYaxis()->FindBin(-absDeltaEtaMin);
        TH1D* pn = ph->ProjectionX(Form("proj_dphi_N_%u_%u_%u", itrig, iassoc, ib), a, b, "e");

        TH1D* phf1 = (TH1D*)pp->Clone(Form("proj_dphi_%u_%u_%u", itrig, iassoc, ib));
        phf1->Add(pp, pn, 0.5, 0.5);
        pltf1[itrig][iassoc][ib] = (TH1D*)phf1->Clone();

        TF1* pfit = new TF1(Form("fit_%u_%u_%u", itrig, iassoc, ib), "[0]+[1]*(1+2*[2]*TMath::Cos(x)+2*[3]*TMath::Cos(2*x)+2*[4]*TMath::Cos(3*x))");
        pfit->SetParNames("czyam", "c", "v1", "v2", "v3");
        pfit->FixParameter(0, 0.0);
        TFitResultPtr r = phf1->Fit(pfit, "0QSE", "", -TMath::Pi() / 2.0, 3.0 / 2.0 * TMath::Pi());

        // double phiMinX = pfit->GetMinimumX(-TMath::Pi()/2.0,3.0/2.0*TMath::Pi());
        double phiMinX = pfit->GetMinimumX(-TMath::Pi() / 2.0, 3.0 / 2.0 * TMath::Pi());
        double phiMin = pfit->Eval(phiMinX);
        double czyam = phiMin;
        std::cout<<"CZYAM="<<czyam<<std::endl;
        double fitErr;
        r->GetConfidenceIntervals(1, 1, 1, &phiMinX, &fitErr, 0.683, false);
        for (uint i = 1; i < phf1->GetXaxis()->GetNbins() + 1; ++i) {
          double y = phf1->GetBinContent(i);
          double yerr = phf1->GetBinError(i);
          phf1->SetBinContent(i, TMath::Max(y - czyam, 0.0));
          phf1->SetBinError(i, TMath::Sqrt(yerr * yerr + fitErr * fitErr));
        }	
        int phiIntShift = 0; 

        // near-side integration
        // Y_ridge^near
        a = phf1->GetXaxis()->FindBin(-TMath::Abs(phiMinX)) + phiIntShift;
        b = phf1->GetXaxis()->FindBin(TMath::Abs(phiMinX)) + phiIntShift;
        double YridgeErr;
        double Yridge = phf1->IntegralAndError(a, b, YridgeErr, "width");
        // printf("<N> = %.1lf, C_ZYAM = %.4f, Yridge = %.4lf pm %.6lf\n",Nct[ib],czyam,Yridge,YridgeErr);

        pridgeYield->SetPoint(ib, Nct[ib], Yridge);
        pridgeYield->SetPointError(ib, 0.0, YridgeErr);
      } // loop over the index of the multiplicity interval
      pridgeYield->Write(Form("ridgeYield_%u_%u", itrig, iassoc)); 
    } // loop over the index of the associated particle
  } // loop over the index of the trigger particle
}// end of processYield
