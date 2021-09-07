// Computation and plotting of reconstruction efficiency step-by-step
// Four steps defined: RecoHFFlag, RecoTopol, RecoCand, RecoPID
// RecoHFFlag: candidates properly flagged (e.g. in HFD0CandidateSelector --> hfflag() is D0ToPiK)
// RecoTopol: candidates which satisfy conjugate-independent topological selection
// RecoCand: candidates which satisfy conjugate-dependent topological selection
// RecoPID: candidates which satisfy conjugate-dependent topological selection and track-level PID selection

// Macros input: AnalysisResults.root from O2 with TH2F (pt vs y) distributions for generated and reconstructed candidates
// TString particles can contain multiple particle strings
// .L PlotEfficiencyRecoStep.C
// PlotEfficiencyRecoStep("InputName.root","particlename",true);

#include "utils_plot.h"

Int_t PlotEfficiencyRecoStep(TString pathFile = "AnalysisResults.root", TString particles = "d0")
{
  gStyle->SetOptStat(0);
  gStyle->SetPalette(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);

  // vertical margins of the pT spectra plot
  float marginHigh = 0.05;
  float marginLow = 0.05;
  bool logScaleH = true;
  // vertical margins of the efficiency plot
  float marginRHigh = 0.05;
  float marginRLow = 0.05;
  bool logScaleR = false;
  double yMin, yMax;
  int nRec, nGen;
  int colours[] = {1, 2, 3, 4};
  int markers[] = {24, 25, 46};

  // binning
  Int_t iNRebin = 4;
  //Double_t* dRebin = nullptr;
  //const Int_t NRebin = 1;
  Double_t dRebin[] = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 5, 6, 8, 10};
  const Int_t NRebin = sizeof(dRebin) / sizeof(dRebin[0]) - 1;

  TFile* file = new TFile(pathFile.Data());
  if (file->IsZombie()) {
    Printf("Error: Failed to open file %s", pathFile.Data());
    return 1;
  }

  // get list of particles
  TObjArray* arrayParticle = 0;
  arrayParticle = particles.Tokenize(" ");
  TString particle = "";

  // compute efficiency at each reconstruction step
  TString recostep[4] = {"RecoHFFlag", "RecoTopol", "RecoCand", "RecoPID"};
  const int NRecoStep = sizeof(recostep) / sizeof(recostep[0]);

  // Generated distributions
  TCanvas* canGenSbyS = new TCanvas("canGenSbyS", "canGenSbyS", 800, 600);
  TCanvas* canGenSbySPt = new TCanvas("canGenSbySPt", "canGenSbySPt", 800, 600);
  TCanvas* canGenSbySY = new TCanvas("canGenSbySY", "canGenSbySY", 800, 600);
  // Reconstructed distributions (inclusive)
  TCanvas* canRecoSbyS = new TCanvas("canRecoSbyS", "canRecoSbyS", 800, 600);
  TCanvas* canRecoSbySPt = new TCanvas("canRecoSbySPt", "canRecoSbySPt", 800, 600);
  TCanvas* canRecoSbySY = new TCanvas("canRecoSbySY", "canRecoSbySY", 800, 600);
  // Reconstructed distributions (prompt)
  TCanvas* canRecoSbySPrompt = new TCanvas("canRecoSbySPrompt", "canRecoSbySPrompt", 800, 600);
  TCanvas* canRecoSbySPromptPt = new TCanvas("canRecoSbySPromptPt", "canRecoSbySPromptPt", 800, 600);
  TCanvas* canRecoSbySPromptY = new TCanvas("canRecoSbySPromptY", "canRecoSbySPromptY", 800, 600);
  // Reconstructed distributions (non-prompt)
  TCanvas* canRecoSbySNonPrompt = new TCanvas("canRecoSbySNonPrompt", "canRecoSbySNonPrompt", 800, 600);
  TCanvas* canRecoSbySNonPromptPt = new TCanvas("canRecoSbySNonPromptPt", "canRecoSbySNonPromptPt", 800, 600);
  TCanvas* canRecoSbySNonPromptY = new TCanvas("canRecoSbySNonPromptY", "canRecoSbySNonPromptY", 800, 600);

  if (NRecoStep == 4) {
    canGenSbyS->Divide(2, 2);
    canGenSbySPt->Divide(2, 2);
    canGenSbySY->Divide(2, 2);
    canRecoSbyS->Divide(2, 2);
    canRecoSbySPt->Divide(2, 2);
    canRecoSbySY->Divide(2, 2);
    canRecoSbySPrompt->Divide(2, 2);
    canRecoSbySPromptPt->Divide(2, 2);
    canRecoSbySPromptY->Divide(2, 2);
    canRecoSbySNonPrompt->Divide(2, 2);
    canRecoSbySNonPromptPt->Divide(2, 2);
    canRecoSbySNonPromptY->Divide(2, 2);
  }

  TCanvas* canEffSbySPt = new TCanvas(Form("canEffSbySPt_%s", particle.Data()), "Eff", 800, 600);
  canEffSbySPt->Divide(2, 2);

  TCanvas* canEffSbySY = new TCanvas(Form("canEffSbySY_%s", particle.Data()), "Eff", 800, 600);
  canEffSbySY->Divide(2, 2);

  TLegend* legendSbyS = new TLegend(0.15, 0.7, 0.45, 0.9);
  legendSbyS->SetFillColorAlpha(0, kWhite);
  legendSbyS->SetLineWidth(0);

  // loop over particles
  for (int iP = 0; iP < arrayParticle->GetEntriesFast(); iP++) {
    particle = ((TObjString*)(arrayParticle->At(iP)))->GetString();
    if (!particle.Length()) {
      Printf("Error: Empty particle string");
      return 1;
    }
    Printf("\nPlotting efficiency step-by-step for: %s", particle.Data());

    TString outputDir = Form("hf-task-%s-mc", particle.Data()); // analysis output directory with histograms

    // inclusive generated
    TString nameHistgen = outputDir + "/hPtvsYGen"; // generator level pT of generated particles
    TH2F* hPtvsYGenIncl = (TH2F*)file->Get(nameHistgen.Data());
    if (!hPtvsYGenIncl) {
      Printf("Error: Failed to load %s from %s", nameHistgen.Data(), pathFile.Data());
      return 1;
    }
    canGenSbyS->cd(1);
    hPtvsYGenIncl->Draw("colz");

    TH1F* hGenPtIncl = (TH1F*)hPtvsYGenIncl->ProjectionX("hGen_Pt", 1, hPtvsYGenIncl->GetXaxis()->GetLast(), "e");
    TH1F* hGenYIncl = (TH1F*)hPtvsYGenIncl->ProjectionY("hGen_Y", 1, hPtvsYGenIncl->GetYaxis()->GetLast(), "e");

    if (iNRebin > 1) {
      hGenPtIncl->Rebin(iNRebin);
      hGenYIncl->Rebin(iNRebin);
    } else if (NRebin > 1) {
      hGenPtIncl = (TH1F*)hGenPtIncl->Rebin(NRebin, "hGenPtInclR", dRebin);
      hGenYIncl = (TH1F*)hGenYIncl->Rebin(NRebin, "hGenYInclR", dRebin);
    }

    canGenSbySPt->cd(1);
    hGenPtIncl->Draw("pe");

    canGenSbySY->cd(1);
    hGenYIncl->Draw("pe");

    // prompt generated
    TString nameHistgenPrompt = outputDir + "/hPtvsYGenPrompt"; // generator level pT of generated particles
    TH2F* hPtvsYGenPrompt = (TH2F*)file->Get(nameHistgenPrompt.Data());
    if (!hPtvsYGenPrompt) {
      Printf("Error: Failed to load %s from %s", nameHistgenPrompt.Data(), pathFile.Data());
      return 1;
    }
    canGenSbyS->cd(2);
    hPtvsYGenPrompt->Draw("colz");

    TH1F* hGenPtPrompt = (TH1F*)hPtvsYGenPrompt->ProjectionX("hGenPrompt_Pt", 1, hPtvsYGenPrompt->GetXaxis()->GetLast(), "e");
    TH1F* hGenYPrompt = (TH1F*)hPtvsYGenPrompt->ProjectionY("hGenPrompt_Y", 1, hPtvsYGenPrompt->GetYaxis()->GetLast(), "e");

    if (iNRebin > 1) {
      hGenPtPrompt->Rebin(iNRebin);
      hGenYPrompt->Rebin(iNRebin);
    } else if (NRebin > 1) {
      hGenPtPrompt = (TH1F*)hGenPtPrompt->Rebin(NRebin, "hGenPtPromptR", dRebin);
      hGenYPrompt = (TH1F*)hGenYPrompt->Rebin(NRebin, "hGenYPromptR", dRebin);
    }

    canGenSbySPt->cd(2);
    hGenPtPrompt->Draw("pe");

    canGenSbySY->cd(2);
    hGenYPrompt->Draw("pe");

    // non-prompt generated
    TString nameHistgenNonPrompt = outputDir + "/hPtvsYGenNonPrompt"; // generator level pT of generated particles
    TH2F* hPtvsYGenNonPrompt = (TH2F*)file->Get(nameHistgenNonPrompt.Data());
    if (!hPtvsYGenNonPrompt) {
      Printf("Error: Failed to load %s from %s", nameHistgenNonPrompt.Data(), pathFile.Data());
      return 1;
    }
    canGenSbyS->cd(3);
    hPtvsYGenNonPrompt->Draw("colz");

    TH1F* hGenPtNonPrompt = (TH1F*)hPtvsYGenNonPrompt->ProjectionX("hGenNonPrompt_Pt", 1, hPtvsYGenNonPrompt->GetXaxis()->GetLast(), "e");
    TH1F* hGenYNonPrompt = (TH1F*)hPtvsYGenNonPrompt->ProjectionY("hGenNonPrompt_Y", 1, hPtvsYGenNonPrompt->GetYaxis()->GetLast(), "e");

    if (iNRebin > 1) {
      hGenPtNonPrompt->Rebin(iNRebin);
      hGenYNonPrompt->Rebin(iNRebin);
    } else if (NRebin > 1) {
      hGenPtNonPrompt = (TH1F*)hGenPtNonPrompt->Rebin(NRebin, "hGenPtNonPromptR", dRebin);
      hGenYNonPrompt = (TH1F*)hGenYNonPrompt->Rebin(NRebin, "hGenYNonPromptR", dRebin);
    }

    canGenSbySPt->cd(3);
    hGenPtNonPrompt->Draw("pe");

    canGenSbySY->cd(3);
    hGenYNonPrompt->Draw("pe");

    for (int iRs = 0; iRs < NRecoStep; iRs++) {
      //inclusive
      TString nameHistRec = outputDir + "/hPtvsYRecSig_" + recostep[iRs]; // reconstruction level pT of matched candidates
      cout << nameHistRec.Data() << endl;

      TH2F* hPtvsYRecIncl = (TH2F*)file->Get(nameHistRec.Data());
      if (!hPtvsYRecIncl) {
        Printf("Error: Failed to load %s from %s", nameHistRec.Data(), pathFile.Data());
        return 1;
      }
      hPtvsYRecIncl->SetTitle(Form("k%s", recostep[iRs].Data()));
      canRecoSbyS->cd(iRs + 1);
      hPtvsYRecIncl->Draw("colz");

      TH1F* hRecoPtIncl = (TH1F*)hPtvsYRecIncl->ProjectionX(Form("h%s_Pt", recostep[iRs].Data()), 1, hPtvsYRecIncl->GetXaxis()->GetLast(), "e");
      canRecoSbySPt->cd(iRs + 1);
      hRecoPtIncl->Draw("pe");

      TH1F* hRecoYIncl = (TH1F*)hPtvsYRecIncl->ProjectionY(Form("h%s_Y", recostep[iRs].Data()), 1, hPtvsYRecIncl->GetYaxis()->GetLast(), "e");
      canRecoSbySY->cd(iRs + 1);
      hRecoYIncl->Draw("pe");

      if (iNRebin > 1) {
        hRecoPtIncl->Rebin(iNRebin);
        hRecoYIncl->Rebin(iNRebin);
      } else if (NRebin > 1) {
        hRecoPtIncl = (TH1F*)hRecoPtIncl->Rebin(NRebin, "hRecoPtInclR", dRebin);
        hRecoYIncl = (TH1F*)hRecoYIncl->Rebin(NRebin, "hRecoYInclR", dRebin);
      }

      auto padEffSbySPt = canEffSbySPt->cd(1);

      TH1F* hEffPtIncl = nullptr;
      hEffPtIncl = (TH1F*)hRecoPtIncl->Clone(Form("hEffPtIncl%s", recostep[iRs].Data()));
      hEffPtIncl->Divide(hEffPtIncl, hGenPtIncl, 1., 1., "B");
      hEffPtIncl->SetTitle("inclusive ;#it{p}^{rec.}_{T} (GeV/#it{c}); efficiency");
      yMin = hEffPtIncl->GetMinimum(0);
      yMax = hEffPtIncl->GetMaximum();
      SetHistogramStyle(hEffPtIncl, colours[iRs], 20, 1.5, 2);
      legendSbyS->AddEntry(hEffPtIncl, Form("%s", recostep[iRs].Data()), "P");

      if (iRs == 0) {
        SetHistogram(hEffPtIncl, yMin, yMax, marginRLow, marginRHigh, logScaleH);
        SetPad(padEffSbySPt, logScaleH);
        hEffPtIncl->Draw("pe");
      } else
        hEffPtIncl->Draw("pesame");
      legendSbyS->Draw("same");

      auto padEffSbySY = canEffSbySY->cd(1);

      TH1F* hEffYIncl = nullptr;
      hEffYIncl = (TH1F*)hRecoYIncl->Clone(Form("hEffYIncl%s", recostep[iRs].Data()));
      hEffYIncl->Divide(hEffYIncl, hGenYIncl, 1., 1., "B");
      hEffYIncl->SetTitle("inclusive ;#it{y}; efficiency");
      yMin = hEffYIncl->GetMinimum(0) * 0.2;
      yMax = hEffYIncl->GetMaximum() * 1.5;
      SetHistogramStyle(hEffYIncl, colours[iRs], 20, 1.5, 2);
      if (iRs == 0) {
        SetHistogram(hEffYIncl, yMin, yMax, marginRLow, marginRHigh, logScaleH);
        SetPad(padEffSbySY, logScaleR);
        hEffYIncl->Draw("pe");
      } else
        hEffYIncl->Draw("pesame");
      legendSbyS->Draw("same");

      //prompt
      TString nameHistRecPrompt = outputDir + "/hPtvsYRecSigPrompt_" + recostep[iRs]; // reconstruction level pT of matched candidates
      TH2F* hPtvsYRecPrompt = (TH2F*)file->Get(nameHistRecPrompt.Data());
      if (!hPtvsYRecPrompt) {
        Printf("Error: Failed to load %s from %s", nameHistRecPrompt.Data(), pathFile.Data());
        return 1;
      }
      hPtvsYRecPrompt->SetTitle(Form("k%s prompt", recostep[iRs].Data()));
      canRecoSbySPrompt->cd(iRs + 1);
      hPtvsYRecPrompt->Draw("colz");

      TH1F* hRecoPtPrompt = (TH1F*)hPtvsYRecPrompt->ProjectionX(Form("h%s_PtPrompt", recostep[iRs].Data()), 1, hPtvsYRecPrompt->GetXaxis()->GetLast(), "e");
      canRecoSbySPromptPt->cd(iRs + 1);
      hRecoPtPrompt->Draw("pe");

      TH1F* hRecoYPrompt = (TH1F*)hPtvsYRecPrompt->ProjectionY(Form("h%s_YPrompt", recostep[iRs].Data()), 1, hPtvsYRecPrompt->GetYaxis()->GetLast(), "e");
      canRecoSbySPromptY->cd(iRs + 1);
      hRecoYPrompt->Draw("pe");

      if (iNRebin > 1) {
        hRecoPtPrompt->Rebin(iNRebin);
        hRecoYPrompt->Rebin(iNRebin);
      } else if (NRebin > 1) {
        hRecoPtPrompt = (TH1F*)hRecoPtPrompt->Rebin(NRebin, "hRecoPtPromptR", dRebin);
        hRecoYPrompt = (TH1F*)hRecoYPrompt->Rebin(NRebin, "hRecoYPromptR", dRebin);
      }

      auto padEffPromptSbySPt = canEffSbySPt->cd(2);

      TH1F* hEffPtPrompt = nullptr;
      hEffPtPrompt = (TH1F*)hRecoPtPrompt->Clone(Form("hEffPtPrompt%s", recostep[iRs].Data()));
      hEffPtPrompt->Divide(hEffPtPrompt, hGenPtPrompt, 1., 1., "B");
      hEffPtPrompt->SetTitle("prompt ;#it{p}^{rec.}_{T} (GeV/#it{c}); efficiency");
      yMin = hEffPtPrompt->GetMinimum(0) * 0.2;
      yMax = hEffPtPrompt->GetMaximum() * 1.5;
      SetHistogramStyle(hEffPtPrompt, colours[iRs], 20, 1.5, 2);

      if (iRs == 0) {
        SetHistogram(hEffPtPrompt, yMin, yMax, marginRLow, marginRHigh, logScaleH);
        SetPad(padEffPromptSbySPt, logScaleH);
        hEffPtPrompt->Draw("pe");
      } else
        hEffPtPrompt->Draw("pesame");
      legendSbyS->Draw("same");

      auto padEffPromptSbySY = canEffSbySY->cd(2);

      TH1F* hEffYPrompt = nullptr;
      hEffYPrompt = (TH1F*)hRecoYPrompt->Clone(Form("hEffYPrompt%s", recostep[iRs].Data()));
      hEffYPrompt->Divide(hEffYPrompt, hGenYPrompt, 1., 1., "B");
      hEffYPrompt->SetTitle("prompt ;#it{y}; efficiency");
      yMin = hEffYPrompt->GetMinimum(0) * 0.2;
      yMax = hEffYPrompt->GetMaximum() * 1.5;
      SetHistogramStyle(hEffYPrompt, colours[iRs], 20, 1.5, 2);
      if (iRs == 0) {
        SetHistogram(hEffYPrompt, yMin, yMax, marginRLow, marginRHigh, logScaleH);
        SetPad(padEffPromptSbySY, logScaleR);
        hEffYPrompt->Draw("pe");
      } else
        hEffYPrompt->Draw("pesame");
      legendSbyS->Draw("same");

      // non-prompt
      TString nameHistRecNonPrompt = outputDir + "/hPtvsYRecSigNonPrompt_" + recostep[iRs]; // reconstruction level pT of matched candidates
      TH2F* hPtvsYRecNonPrompt = (TH2F*)file->Get(nameHistRecNonPrompt.Data());
      if (!hPtvsYRecNonPrompt) {
        Printf("Error: Failed to load %s from %s", nameHistRecNonPrompt.Data(), pathFile.Data());
        return 1;
      }
      hPtvsYRecNonPrompt->SetTitle(Form("k%s non-prompt", recostep[iRs].Data()));
      canRecoSbySNonPrompt->cd(iRs + 1);
      hPtvsYRecNonPrompt->Draw("colz");

      TH1F* hRecoPtNonPrompt = (TH1F*)hPtvsYRecNonPrompt->ProjectionX(Form("h%s_PtNonPrompt", recostep[iRs].Data()), 1, hPtvsYRecNonPrompt->GetXaxis()->GetLast(), "e");
      canRecoSbySNonPromptPt->cd(iRs + 1);
      hRecoPtNonPrompt->Draw("pe");

      TH1F* hRecoYNonPrompt = (TH1F*)hPtvsYRecNonPrompt->ProjectionY(Form("h%s_YNonPrompt", recostep[iRs].Data()), 1, hPtvsYRecNonPrompt->GetYaxis()->GetLast(), "e");
      canRecoSbySNonPromptY->cd(iRs + 1);
      hRecoYNonPrompt->Draw("pe");

      if (iNRebin > 1) {
        hRecoPtNonPrompt->Rebin(iNRebin);
        hRecoYNonPrompt->Rebin(iNRebin);
      } else if (NRebin > 1) {
        hRecoPtNonPrompt = (TH1F*)hRecoPtNonPrompt->Rebin(NRebin, "hRecoPtNonPromptR", dRebin);
        hRecoYNonPrompt = (TH1F*)hRecoYNonPrompt->Rebin(NRebin, "hRecoYNonPromptR", dRebin);
      }

      auto padEffNonPromptSbySPt = canEffSbySPt->cd(3);

      TH1F* hEffPtNonPrompt = nullptr;
      hEffPtNonPrompt = (TH1F*)hRecoPtNonPrompt->Clone(Form("hEffPtNonPrompt%s", recostep[iRs].Data()));
      hEffPtNonPrompt->Divide(hEffPtNonPrompt, hGenPtNonPrompt, 1., 1., "B");
      hEffPtNonPrompt->SetTitle("non-prompt ;#it{p}^{rec.}_{T} (GeV/#it{c}); efficiency");
      yMin = hEffPtNonPrompt->GetMinimum(0) * 0.2;
      yMax = hEffPtNonPrompt->GetMaximum() * 1.5;
      SetHistogramStyle(hEffPtNonPrompt, colours[iRs], 20, 1.5, 2);

      if (iRs == 0) {
        SetHistogram(hEffPtNonPrompt, yMin, yMax, marginRLow, marginRHigh, logScaleH);
        SetPad(padEffNonPromptSbySPt, logScaleH);
        hEffPtNonPrompt->Draw("pe");
      } else
        hEffPtNonPrompt->Draw("pesame");
      legendSbyS->Draw("same");

      auto padEffNonPromptSbySY = canEffSbySY->cd(3);

      TH1F* hEffYNonPrompt = nullptr;
      hEffYNonPrompt = (TH1F*)hRecoYNonPrompt->Clone(Form("hEffYNonPrompt%s", recostep[iRs].Data()));
      hEffYNonPrompt->Divide(hEffYNonPrompt, hGenYNonPrompt, 1., 1., "B");
      hEffYNonPrompt->SetTitle("non-prompt ;#it{y}; efficiency");
      yMin = hEffYNonPrompt->GetMinimum(0) * 0.2;
      yMax = hEffYNonPrompt->GetMaximum() * 1.5;
      SetHistogramStyle(hEffYNonPrompt, colours[iRs], 20, 1.5, 2);
      if (iRs == 0) {
        SetHistogram(hEffYNonPrompt, yMin, yMax, marginRLow, marginRHigh, logScaleH);
        SetPad(padEffNonPromptSbySY, logScaleR);
        hEffYNonPrompt->Draw("pe");
      } else
        hEffYNonPrompt->Draw("pesame");
      legendSbyS->Draw("same");
    }
  }
  canEffSbySPt->SaveAs(Form("MC_%s_eff_stepbystep_pT.pdf", particle.Data()));
  canEffSbySY->SaveAs(Form("MC_%s_eff_stepbystepY.pdf", particle.Data()));

  delete arrayParticle;
  return 0;
}
