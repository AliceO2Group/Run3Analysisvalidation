// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

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

#include "../exec/utilitiesPlot.h"

void SetProperAxisRange(TH1F** histo, int NIteration, float marginHigh, float marginLow, bool logScaleH);

Int_t PlotEfficiencyRecoStep(TString pathFile = "AnalysisResults_O2.root", TString particles = "d0")
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
  double yMin = 999.;
  double yMax = -999.;
  int nRec, nGen;
  int colours[] = {1, 2, 3, 4};
  int markers[] = {24, 25, 46, 28};
  float markersize[] = {1., 1., 1., 1.};
  int lineWidth = 1;

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
  TString recoStep[4] = {"RecoHFFlag", "RecoTopol", "RecoCand", "RecoPID"};
  TString labelsSteps[4] = {"skimming", "topol. gen.", "topol. conj.", "PID"};
  const int NRecoStep = sizeof(recoStep) / sizeof(recoStep[0]);
  TString partType[3] = {"Incl", "Prompt", "NonPrompt"};
  const int NPartType = sizeof(partType) / sizeof(partType[0]);

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

  TCanvas* canEffSbySPt = new TCanvas(Form("canEffSbySPt_%s", particle.Data()), "EffPt", 800, 600);
  canEffSbySPt->Divide(2, 2);

  TCanvas* canEffSbySY = new TCanvas(Form("canEffSbySY_%s", particle.Data()), "EffY", 800, 600);
  canEffSbySY->Divide(2, 2);

  // Efficiency (inclusive)
  TH1F** hEffPtIncl = new TH1F*[NRecoStep];
  TH1F** hEffYIncl = new TH1F*[NRecoStep];

  // Efficiency (prompt)
  TH1F** hEffPtPrompt = new TH1F*[NRecoStep];
  TH1F** hEffYPrompt = new TH1F*[NRecoStep];

  // Efficiency (non-propmt)
  TH1F** hEffPtNonPrompt = new TH1F*[NRecoStep];
  TH1F** hEffYNonPrompt = new TH1F*[NRecoStep];

  TLegend* legendSbyS = new TLegend(0.15, 0.65, 0.45, 0.9);
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
    TString nameHistgen = outputDir + "/hPtVsYGen"; // generator level pT of generated particles
    TH2F* hPtVsYGenIncl = (TH2F*)file->Get(nameHistgen.Data());
    if (!hPtVsYGenIncl) {
      Printf("Error: Failed to load %s from %s", nameHistgen.Data(), pathFile.Data());
      return 1;
    }
    canGenSbyS->cd(1);
    hPtVsYGenIncl->Draw("colz");

    TH1F* hGenPtIncl = (TH1F*)hPtVsYGenIncl->ProjectionX("hGen_Pt", 1, hPtVsYGenIncl->GetXaxis()->GetLast(), "e");
    TH1F* hGenYIncl = (TH1F*)hPtVsYGenIncl->ProjectionY("hGen_Y", 1, hPtVsYGenIncl->GetYaxis()->GetLast(), "e");

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
    TString nameHistgenPrompt = outputDir + "/hPtVsYGenPrompt"; // generator level pT of generated particles
    TH2F* hPtVsYGenPrompt = (TH2F*)file->Get(nameHistgenPrompt.Data());
    if (!hPtVsYGenPrompt) {
      Printf("Error: Failed to load %s from %s", nameHistgenPrompt.Data(), pathFile.Data());
      return 1;
    }
    canGenSbyS->cd(2);
    hPtVsYGenPrompt->Draw("colz");

    TH1F* hGenPtPrompt = (TH1F*)hPtVsYGenPrompt->ProjectionX("hGenPrompt_Pt", 1, hPtVsYGenPrompt->GetXaxis()->GetLast(), "e");
    TH1F* hGenYPrompt = (TH1F*)hPtVsYGenPrompt->ProjectionY("hGenPrompt_Y", 1, hPtVsYGenPrompt->GetYaxis()->GetLast(), "e");

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
    TString nameHistgenNonPrompt = outputDir + "/hPtVsYGenNonPrompt"; // generator level pT of generated particles
    TH2F* hPtVsYGenNonPrompt = (TH2F*)file->Get(nameHistgenNonPrompt.Data());
    if (!hPtVsYGenNonPrompt) {
      Printf("Error: Failed to load %s from %s", nameHistgenNonPrompt.Data(), pathFile.Data());
      return 1;
    }
    canGenSbyS->cd(3);
    hPtVsYGenNonPrompt->Draw("colz");

    TH1F* hGenPtNonPrompt = (TH1F*)hPtVsYGenNonPrompt->ProjectionX("hGenNonPrompt_Pt", 1, hPtVsYGenNonPrompt->GetXaxis()->GetLast(), "e");
    TH1F* hGenYNonPrompt = (TH1F*)hPtVsYGenNonPrompt->ProjectionY("hGenNonPrompt_Y", 1, hPtVsYGenNonPrompt->GetYaxis()->GetLast(), "e");

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
      TString nameHistRec = outputDir + "/hPtVsYRecSig" + recoStep[iRs]; // reconstruction level pT of matched candidates
      cout << nameHistRec.Data() << endl;

      TH2F* hPtVsYRecIncl = (TH2F*)file->Get(nameHistRec.Data());
      if (!hPtVsYRecIncl) {
        Printf("Error: Failed to load %s from %s", nameHistRec.Data(), pathFile.Data());
        return 1;
      }
      hPtVsYRecIncl->SetTitle(Form("k%s", recoStep[iRs].Data()));
      canRecoSbyS->cd(iRs + 1);
      hPtVsYRecIncl->Draw("colz");

      TH1F* hRecoPtIncl = (TH1F*)hPtVsYRecIncl->ProjectionX(Form("h%s_Pt", recoStep[iRs].Data()), 1, hPtVsYRecIncl->GetXaxis()->GetLast(), "e");
      canRecoSbySPt->cd(iRs + 1);
      hRecoPtIncl->Draw("pe");

      TH1F* hRecoYIncl = (TH1F*)hPtVsYRecIncl->ProjectionY(Form("h%s_Y", recoStep[iRs].Data()), 1, hPtVsYRecIncl->GetYaxis()->GetLast(), "e");
      canRecoSbySY->cd(iRs + 1);
      hRecoYIncl->Draw("pe");

      if (iNRebin > 1) {
        hRecoPtIncl->Rebin(iNRebin);
        hRecoYIncl->Rebin(iNRebin);
      } else if (NRebin > 1) {
        hRecoPtIncl = (TH1F*)hRecoPtIncl->Rebin(NRebin, "hRecoPtInclR", dRebin);
        hRecoYIncl = (TH1F*)hRecoYIncl->Rebin(NRebin, "hRecoYInclR", dRebin);
      }

      hEffPtIncl[iRs] = (TH1F*)hRecoPtIncl->Clone(Form("hEffPtIncl%s", recoStep[iRs].Data()));
      hEffPtIncl[iRs]->Divide(hEffPtIncl[iRs], hGenPtIncl, 1., 1., "B");
      hEffPtIncl[iRs]->SetTitle("inclusive ;#it{p}^{rec.}_{T} (GeV/#it{c}); efficiency");
      SetHistogramStyle(hEffPtIncl[iRs], colours[iRs], markers[iRs], markersize[iRs], lineWidth);
      legendSbyS->AddEntry(hEffPtIncl[iRs], labelsSteps[iRs].Data(), "P");

      hEffYIncl[iRs] = (TH1F*)hRecoYIncl->Clone(Form("hEffYIncl%s", recoStep[iRs].Data()));
      hEffYIncl[iRs]->Divide(hEffYIncl[iRs], hGenYIncl, 1., 1., "B");
      hEffYIncl[iRs]->SetTitle("inclusive ;#it{y}; efficiency");
      SetHistogramStyle(hEffYIncl[iRs], colours[iRs], markers[iRs], markersize[iRs], lineWidth);

      //prompt
      TString nameHistRecPrompt = outputDir + "/hPtVsYRecSigPrompt" + recoStep[iRs]; // reconstruction level pT of matched candidates
      TH2F* hPtVsYRecPrompt = (TH2F*)file->Get(nameHistRecPrompt.Data());
      if (!hPtVsYRecPrompt) {
        Printf("Error: Failed to load %s from %s", nameHistRecPrompt.Data(), pathFile.Data());
        return 1;
      }
      hPtVsYRecPrompt->SetTitle(Form("k%s prompt", recoStep[iRs].Data()));
      canRecoSbySPrompt->cd(iRs + 1);
      hPtVsYRecPrompt->Draw("colz");

      TH1F* hRecoPtPrompt = (TH1F*)hPtVsYRecPrompt->ProjectionX(Form("h%s_PtPrompt", recoStep[iRs].Data()), 1, hPtVsYRecPrompt->GetXaxis()->GetLast(), "e");
      canRecoSbySPromptPt->cd(iRs + 1);
      hRecoPtPrompt->Draw("pe");

      TH1F* hRecoYPrompt = (TH1F*)hPtVsYRecPrompt->ProjectionY(Form("h%s_YPrompt", recoStep[iRs].Data()), 1, hPtVsYRecPrompt->GetYaxis()->GetLast(), "e");
      canRecoSbySPromptY->cd(iRs + 1);
      hRecoYPrompt->Draw("pe");

      if (iNRebin > 1) {
        hRecoPtPrompt->Rebin(iNRebin);
        hRecoYPrompt->Rebin(iNRebin);
      } else if (NRebin > 1) {
        hRecoPtPrompt = (TH1F*)hRecoPtPrompt->Rebin(NRebin, "hRecoPtPromptR", dRebin);
        hRecoYPrompt = (TH1F*)hRecoYPrompt->Rebin(NRebin, "hRecoYPromptR", dRebin);
      }

      hEffPtPrompt[iRs] = (TH1F*)hRecoPtPrompt->Clone(Form("hEffPtPrompt%s", recoStep[iRs].Data()));
      hEffPtPrompt[iRs]->Divide(hEffPtPrompt[iRs], hGenPtPrompt, 1., 1., "B");
      hEffPtPrompt[iRs]->SetTitle("prompt ;#it{p}^{rec.}_{T} (GeV/#it{c}); efficiency");
      SetHistogramStyle(hEffPtPrompt[iRs], colours[iRs], markers[iRs], markersize[iRs], lineWidth);

      hEffYPrompt[iRs] = (TH1F*)hRecoYPrompt->Clone(Form("hEffYPrompt%s", recoStep[iRs].Data()));
      hEffYPrompt[iRs]->Divide(hEffYPrompt[iRs], hGenYPrompt, 1., 1., "B");
      hEffYPrompt[iRs]->SetTitle("prompt ;#it{y}; efficiency");
      SetHistogramStyle(hEffYPrompt[iRs], colours[iRs], markers[iRs], markersize[iRs], lineWidth);

      // non-prompt
      TString nameHistRecNonPrompt = outputDir + "/hPtVsYRecSigNonPrompt" + recoStep[iRs]; // reconstruction level pT of matched candidates
      TH2F* hPtVsYRecNonPrompt = (TH2F*)file->Get(nameHistRecNonPrompt.Data());
      if (!hPtVsYRecNonPrompt) {
        Printf("Error: Failed to load %s from %s", nameHistRecNonPrompt.Data(), pathFile.Data());
        return 1;
      }
      hPtVsYRecNonPrompt->SetTitle(Form("k%s non-prompt", recoStep[iRs].Data()));
      canRecoSbySNonPrompt->cd(iRs + 1);
      hPtVsYRecNonPrompt->Draw("colz");

      TH1F* hRecoPtNonPrompt = (TH1F*)hPtVsYRecNonPrompt->ProjectionX(Form("h%s_PtNonPrompt", recoStep[iRs].Data()), 1, hPtVsYRecNonPrompt->GetXaxis()->GetLast(), "e");
      canRecoSbySNonPromptPt->cd(iRs + 1);
      hRecoPtNonPrompt->Draw("pe");

      TH1F* hRecoYNonPrompt = (TH1F*)hPtVsYRecNonPrompt->ProjectionY(Form("h%s_YNonPrompt", recoStep[iRs].Data()), 1, hPtVsYRecNonPrompt->GetYaxis()->GetLast(), "e");
      canRecoSbySNonPromptY->cd(iRs + 1);
      hRecoYNonPrompt->Draw("pe");

      if (iNRebin > 1) {
        hRecoPtNonPrompt->Rebin(iNRebin);
        hRecoYNonPrompt->Rebin(iNRebin);
      } else if (NRebin > 1) {
        hRecoPtNonPrompt = (TH1F*)hRecoPtNonPrompt->Rebin(NRebin, "hRecoPtNonPromptR", dRebin);
        hRecoYNonPrompt = (TH1F*)hRecoYNonPrompt->Rebin(NRebin, "hRecoYNonPromptR", dRebin);
      }

      hEffPtNonPrompt[iRs] = (TH1F*)hRecoPtNonPrompt->Clone(Form("hEffPtNonPrompt%s", recoStep[iRs].Data()));
      hEffPtNonPrompt[iRs]->Divide(hEffPtNonPrompt[iRs], hGenPtNonPrompt, 1., 1., "B");
      hEffPtNonPrompt[iRs]->SetTitle("non-prompt ;#it{p}^{rec.}_{T} (GeV/#it{c}); efficiency");
      SetHistogramStyle(hEffPtNonPrompt[iRs], colours[iRs], markers[iRs], markersize[iRs], lineWidth);

      hEffYNonPrompt[iRs] = (TH1F*)hRecoYNonPrompt->Clone(Form("hEffYNonPrompt%s", recoStep[iRs].Data()));
      hEffYNonPrompt[iRs]->Divide(hEffYNonPrompt[iRs], hGenYNonPrompt, 1., 1., "B");
      hEffYNonPrompt[iRs]->SetTitle("non-prompt ;#it{y}; efficiency");
      SetHistogramStyle(hEffYNonPrompt[iRs], colours[iRs], markers[iRs], markersize[iRs], lineWidth);
    }
  }

  // Finding yMax and yMin for the different efficiency histograms
  SetProperAxisRange(hEffPtIncl, NRecoStep, marginRHigh, marginRLow, logScaleH);
  SetProperAxisRange(hEffYIncl, NRecoStep, marginRHigh, marginRLow, logScaleH);
  SetProperAxisRange(hEffPtPrompt, NRecoStep, marginRHigh, marginRLow, logScaleH);
  SetProperAxisRange(hEffYPrompt, NRecoStep, marginRHigh, marginRLow, logScaleH);
  SetProperAxisRange(hEffPtNonPrompt, NRecoStep, marginRHigh, marginRLow, logScaleH);
  SetProperAxisRange(hEffYNonPrompt, NRecoStep, marginRHigh, marginRLow, logScaleH);

  // Plotting
  for (int iRs = 0; iRs < NRecoStep; iRs++) {
    auto padEffSbySPt = canEffSbySPt->cd(1);
    if (iRs == 0) {
      SetPad(padEffSbySPt, logScaleH);
      hEffPtIncl[iRs]->Draw("pe");
    } else
      hEffPtIncl[iRs]->Draw("pesame");
    legendSbyS->Draw("same");

    auto padEffSbySY = canEffSbySY->cd(1);
    if (iRs == 0) {
      SetPad(padEffSbySY, logScaleH);
      hEffYIncl[iRs]->Draw("pe");
    } else
      hEffYIncl[iRs]->Draw("pesame");
    legendSbyS->Draw("same");

    auto padEffPromptSbySPt = canEffSbySPt->cd(2);
    if (iRs == 0) {
      SetPad(padEffPromptSbySPt, logScaleH);
      hEffPtPrompt[iRs]->Draw("pe");
    } else
      hEffPtPrompt[iRs]->Draw("pesame");
    legendSbyS->Draw("same");

    auto padEffPromptSbySY = canEffSbySY->cd(2);
    if (iRs == 0) {
      SetPad(padEffPromptSbySY, logScaleH);
      hEffYPrompt[iRs]->Draw("pe");
    } else
      hEffYPrompt[iRs]->Draw("pesame");
    legendSbyS->Draw("same");

    auto padEffNonPromptSbySPt = canEffSbySPt->cd(3);
    if (iRs == 0) {
      SetPad(padEffNonPromptSbySPt, logScaleH);
      hEffPtNonPrompt[iRs]->Draw("pe");
    } else
      hEffPtNonPrompt[iRs]->Draw("pesame");
    legendSbyS->Draw("same");

    auto padEffNonPromptSbySY = canEffSbySY->cd(3);
    if (iRs == 0) {
      SetPad(padEffNonPromptSbySY, logScaleH);
      hEffYNonPrompt[iRs]->Draw("pe");
    } else
      hEffYNonPrompt[iRs]->Draw("pesame");
    legendSbyS->Draw("same");
  }

  canEffSbySPt->SaveAs(Form("MC_%s_eff_stepbystep_pT.pdf", particle.Data()));
  canEffSbySY->SaveAs(Form("MC_%s_eff_stepbystepY.pdf", particle.Data()));
  canEffSbySPt->SaveAs(Form("MC_%s_eff_stepbystep_pT.png", particle.Data()));
  canEffSbySY->SaveAs(Form("MC_%s_eff_stepbystepY.png", particle.Data()));

  delete arrayParticle;
  return 0;
}

void SetProperAxisRange(TH1F** histo, int NIteration, float marginHigh, float marginLow, bool logScaleH)
{
  double yMin = 999.;
  double yMax = -999.;
  for (int iRs = 0; iRs < NIteration; iRs++) {

    if (histo[iRs]->GetMinimum(0) < yMin) {
      yMin = histo[iRs]->GetMinimum(0);
    }
    if (histo[iRs]->GetMaximum() > yMax) {
      yMax = histo[iRs]->GetMaximum();
    }
    SetHistogram(histo[0], yMin, yMax, marginLow, marginHigh, logScaleH);
  }
}