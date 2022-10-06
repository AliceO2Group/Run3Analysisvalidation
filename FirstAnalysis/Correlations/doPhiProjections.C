//////////////////////////////////////////////////////////////
//  Macro to obtain projections of two-particle correlations for flow studies
//
//  For reference and pT-differential flow do
//  1) a projection on deltaeta axis of the away and near side region separately
//  2) a projection on deltaphi axis of the near-side ridge region (excluding the jet peak)
//
//  Input: file with 2D correlation histograms produced by extract2D.C macro in this folder
//
//  Usage: root -l doPhiProjections.C
//
//  Parameters:
//  - inFileName: name of the input file
//  - absDeltaEtaMin: lower edge of deltaeta range when integrating the near-side ridge region
//  - absDeltaEtaMax: upper edge of deltaeta range when integrating the near-side ridge region
//  - outFileName: name of the output file with histograms of projections
//  - outputPlotsName: name of the folder to store plots
//  - drawPlots: flag to draw the projections
//  - savePlots: flag to save the drawn projections
//
//  Contributors:
//    Katarina Krizkova Gajdosova <katarina.gajdosova@cern.ch>
//    Gian Michele Innocenti <gian.michele.innocenti@cern.ch>
//    Jan Fiete Grosse-Oetringhaus <Jan.Fiete.Grosse-Oetringhaus@cern.ch>
//////////////////////////////////////////////////////////////

bool wingCorrection = false; // correct for increase of correlation signal at large deltaeta values

// Note:  if a canvas is drawn with an empty pad, it is probably because
//        the TFile took ownership of the histogram and deleted it when
//        it went out of scope
//        To fix it, call h->SetDirectory(0) before drawing

int nBinspTtrig = 6;
double binspTtrig[] = {0.2, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0};

int nBinspTref = 1;
double binspTref[] = {0.2, 3.0};

void doPhiProjections(
  const char* inFileName = "dphi_corr.root", 
  double absDeltaEtaMin = 1.4, 
  double absDeltaEtaMax = 1.8, 
  const char* outFileName = "phi_proj.root", 
  const char* outputPlotsName = "./plots", 
  bool drawPlots = false, 
  bool savePlots = false)
{
  //  Nch represents the multiplicity interval of the analysis
  static Double_t Nch[] = {0, 10, 20, 30, 40, 50, 60, 80, 100, 200}; 
  //  Nbins is the number of multiplicity bins
  static const uint Nbins = 9;
  //static const uint Nbins = sizeof(Nch) / sizeof(Nch[0]);
  const double absDeltaPhi = 1.3; // y-projection range (it is only needed for jet yield calculation from delta eta projection)

  TFile* infile = new TFile(inFileName, "read");
  TFile* outfile = new TFile(outFileName, "recreate");

  const uint assocCount = 1;
  const uint trigCount = 6;
  
  for (uint imult = 0; imult < Nbins; ++imult) {

    //  do reference flow

    // 2D histogram of two-particle correlation: same/mixed event ratio (normalised as it should be: Ntrig, B(0,0))
    TH2D* hdphidetaRidge_ref = (TH2D*)infile->Get(Form("dphi_ref_%u", imult));
    if (!hdphidetaRidge_ref) {
      printf("No histograms corresponding mult bin %u \n", imult);
      continue;
    }

    //  projection of near-side ridge on delta phi axis (positive side of the jet peak)
    int aridgeP_ref = hdphidetaRidge_ref->GetYaxis()->FindBin(absDeltaEtaMin);
    int bridgeP_ref = hdphidetaRidge_ref->GetYaxis()->FindBin(absDeltaEtaMax);
    TH1D* hdphiRidgeP_ref = hdphidetaRidge_ref->ProjectionX(Form("proj_dphi_P_ref_%u", imult), aridgeP_ref, bridgeP_ref, "e");
    outfile->cd();
    hdphiRidgeP_ref->Write();

    //  projection of near-side ridge on delta phi axis (negative side of the jet peak)
    int aridgeN_ref = hdphidetaRidge_ref->GetYaxis()->FindBin(-absDeltaEtaMax);
    int bridgeN_ref = hdphidetaRidge_ref->GetYaxis()->FindBin(-absDeltaEtaMin);
    TH1D* hdphiRidgeN_ref = hdphidetaRidge_ref->ProjectionX(Form("proj_dphi_N_ref_%u", imult), aridgeN_ref, bridgeN_ref, "e");
    outfile->cd();
    hdphiRidgeN_ref->Write();

    //  add the projections positive + negative
    TH1D* hdphiRidge_ref = (TH1D*)hdphiRidgeP_ref->Clone(Form("proj_dphi_ref_%u", imult));
    hdphiRidge_ref->Add(hdphiRidgeP_ref, hdphiRidgeN_ref, 0.5, 0.5);

    outfile->cd();
    hdphiRidge_ref->Write();

    if (drawPlots) {

      TCanvas* cTemplate = new TCanvas("cTemplate", "", 1200, 800);
      gPad->SetMargin(0.12,0.01,0.12,0.01);
      hdphiRidge_ref->SetTitle("");
      hdphiRidge_ref->SetStats(0);
      hdphiRidge_ref->GetYaxis()->SetTitleOffset(1.1);
      hdphiRidge_ref->GetXaxis()->SetTitleSize(0.05);
      hdphiRidge_ref->GetYaxis()->SetTitle("Y(#Delta#varphi)");
      hdphiRidge_ref->GetYaxis()->SetTitleSize(0.05);
      hdphiRidge_ref->SetLineColor(kBlue+1);
      hdphiRidge_ref->SetMarkerStyle(kFullCircle);
      hdphiRidge_ref->SetMarkerColor(kBlue+1);
      hdphiRidge_ref->SetMarkerSize(1.3);
      hdphiRidge_ref->Draw("");

      TLatex* latex = 0;
      latex = new TLatex();
      latex->SetTextSize(0.038);
      latex->SetTextFont(42);
      latex->SetTextAlign(21);
      latex->SetNDC();

      latex->DrawLatex(0.3, 0.93, "pp #sqrt{s} = 13 TeV");
      latex->DrawLatex(0.3, 0.86, Form("%.1f < p_{T, trig, assoc} < %.1f", binspTref[0], binspTref[1]));
      latex->DrawLatex(0.3, 0.79, Form("%.1f < N_{ch} < %.1f", Nch[imult], Nch[imult+1]));

      if(savePlots) cTemplate->SaveAs(Form("%s/dphiRidge_ref_%d.png", outputPlotsName, imult));
    }

    //  do pT-differential flow
    for (uint itrig = 0; itrig < trigCount; ++itrig) {
      for (uint iassoc = 0; iassoc < assocCount; ++iassoc) {

        // 2D histogram of two-particle correlation: same/mixed event ratio (normalised as it should be: Ntrig, B(0,0))
        TH2D* hdphidetaRidge = (TH2D*)infile->Get(Form("dphi_%u_%u_%u", itrig, iassoc, imult));
        if (!hdphidetaRidge) {
          printf("No histograms corresponding mult bin %u. (itrig=%u, iassoc=%u)\n", imult, itrig, iassoc);
          continue;
        } // if histogram not existing

        // Clone hdphidetaJet: hdphidetaRidge will be used for phi projection; hdphidetaJet for eta projection
        TH2D* hdphidetaJet = (TH2D*)hdphidetaRidge->Clone("hdphidetaJet");

        //  Normalise hdphidetaRidge used for delta phi projection with the width of the long-range region
        double norm = 2.0 * (absDeltaEtaMax - absDeltaEtaMin);
        hdphidetaRidge->Scale(1.0 / norm);

        //  projection of the away-side ridge on delta eta axis
        int ajetaway = hdphidetaJet->GetXaxis()->FindBin(TMath::Pi() - 1.5);
        int bjetaway = hdphidetaJet->GetXaxis()->FindBin(TMath::Pi() + 1.5);
        TH1D* hdetaJetAway = hdphidetaJet->ProjectionY(Form("proj_deta_%u_%u_%u", itrig, iassoc, imult), ajetaway, bjetaway, "e");
        outfile->cd();
        hdetaJetAway->Write();

        //  projection of the near-side region (peak+ridge) on delta eta axis
        int ajetpeak = hdphidetaJet->GetXaxis()->FindBin(-1.5);
        int bjetpeak = hdphidetaJet->GetXaxis()->FindBin(+1.5);
        TH1D* hdetaJetPeak = hdphidetaJet->ProjectionY(Form("proj_detaJetPeak_%u_%u_%u", itrig, iassoc, imult), ajetpeak, bjetpeak, "e");
        outfile->cd();
        hdetaJetPeak->Write();

        //  wing correction (switched off for now)
        if (wingCorrection) {
          //  project the away side onto delta eta, fit with polynomial, and scale the 2D histogram by the difference
          //  between the projected histogram and the fit
          int a = hdphidetaRidge->GetXaxis()->FindBin(TMath::Pi() - 1.5);
          int b = hdphidetaRidge->GetXaxis()->FindBin(TMath::Pi() + 1.5);
          TH1D* hdetaAwayProj = hdphidetaRidge->ProjectionY(Form("proj_deta_%u_%u_%u_scaler", itrig, iassoc, imult), a, b, "e");
          hdetaAwayProj->Fit("pol0", "0QSE");
          hdetaAwayProj->Divide(hdetaAwayProj->GetFunction("pol0"));
          for (uint i = 1; i < hdphidetaRidge->GetNbinsX(); ++i)
            for (uint j = 1; j < hdphidetaRidge->GetNbinsY(); ++j) {
              double z = hdetaAwayProj->GetBinContent(j);
              if (z <= 0.0)
                continue;
              hdphidetaRidge->SetBinContent(i, j, hdphidetaRidge->GetBinContent(i, j) / z);
              hdphidetaRidge->SetBinError(i, j, hdphidetaRidge->GetBinError(i, j) / z);
            }
        }

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

        if (drawPlots) {
          TCanvas* cTemplate = new TCanvas("cTemplate", "", 1200, 800);
          gPad->SetMargin(0.12,0.01,0.12,0.01);
          hdphiRidge->SetTitle("");
          hdphiRidge->SetStats(0);
          hdphiRidge->GetYaxis()->SetTitleOffset(1.1);
          hdphiRidge->GetXaxis()->SetTitleSize(0.05);
          hdphiRidge->GetYaxis()->SetTitle("Y(#Delta#varphi)");
          hdphiRidge->GetYaxis()->SetTitleSize(0.05);
          hdphiRidge->SetLineColor(kBlue+1);
          hdphiRidge->SetMarkerStyle(kFullCircle);
          hdphiRidge->SetMarkerColor(kBlue+1);
          hdphiRidge->SetMarkerSize(1.3);
          hdphiRidge->Draw("");

          TLatex* latex = 0;
          latex = new TLatex();
          latex->SetTextSize(0.038);
          latex->SetTextFont(42);
          latex->SetTextAlign(21);
          latex->SetNDC();

          latex->DrawLatex(0.3, 0.93, "pp #sqrt{s} = 13 TeV");
          latex->DrawLatex(0.3, 0.86, Form("%.1f < p_{T, trig} < %.1f", binspTtrig[itrig], binspTtrig[itrig+1]));
          latex->DrawLatex(0.3, 0.79, Form("%.1f < N_{ch} < %.1f", Nch[imult], Nch[imult+1]));

          if(savePlots) cTemplate->SaveAs(Form("%s/dphiRidge_%d_%d.png", outputPlotsName, itrig, imult));
        }

      } // loop over the index of the associated particle
    } // loop over the index of the trigger particle
  } // loop over the index of the multiplicity interval

  outfile->Close();

} // end of doPhiProjections
