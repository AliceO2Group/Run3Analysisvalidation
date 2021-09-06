// Computation and plotting of reconstruction efficiency step-by-step
// Four steps defined: RecoHFFlag, RecoTopol, RecoCand, RecoPID
// RecoHFFlag: candidates properly flagged (e.g. in HFD0CandidateSelector --> hfflag() is D0ToPiK)
// RecoTopol: candidates which satisfy conjugate-independent topological selection
// RecoCand: candidates which satisfy conjugate-dependent topological selection
// RecoPID: candidates which satisfy conjugate-dependent topological selection and track-level PID selection

// Macros input: AnalysisResults.root from O2 with TH2F (pt vs y) distributions for generated and reconstructed candidates
// .L PlotEfficiency_recoStep.C
// PlotEfficiencySbyS("InputName.root","particlename",true);

#include "utils_plot.h"

Int_t PlotEfficiencySbyS(TString pathFile = "AnalysisResults.root", TString particles = "d0", bool effSbyS = true)
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
  TCanvas* cgensbys = new TCanvas("cgensbys", "cgensbys", 800, 600);
  TCanvas* cgensbys_pt = new TCanvas("cgensbys_pt", "cgensbys_pt", 800, 600);
  TCanvas* cgensbys_y = new TCanvas("cgensbys_y", "cgensbys_y", 800, 600);
  // Reconstructed distributions (inclusive)
  TCanvas* crecosbys = new TCanvas("crecosbys", "crecosbys", 800, 600);
  TCanvas* crecosbys_pt = new TCanvas("crecosbys_pt", "crecosbys_pt", 800, 600);
  TCanvas* crecosbys_y = new TCanvas("crecosbys_y", "crecosbys_y", 800, 600);
  // Reconstructed distributions (prompt)
  TCanvas* crecosbys_prompt = new TCanvas("crecosbys_prompt", "crecosbys_prompt", 800, 600);
  TCanvas* crecosbys_prompt_pt = new TCanvas("crecosbys_prompt_pt", "crecosbys_prompt_pt", 800, 600);
  TCanvas* crecosbys_prompt_y = new TCanvas("crecosbys_prompt_y", "crecosbys_prompt_y", 800, 600);
  // Reconstructed distributions (non-prompt)
  TCanvas* crecosbys_nonprompt = new TCanvas("crecosbys_nonprompt", "crecosbys_nonprompt", 800, 600);
  TCanvas* crecosbys_nonprompt_pt = new TCanvas("crecosbys_nonprompt_pt", "crecosbys_nonprompt_pt", 800, 600);
  TCanvas* crecosbys_nonprompt_y = new TCanvas("crecosbys_nonprompt_y", "crecosbys_nonprompt_y", 800, 600);

  if (NRecoStep == 4) {
    cgensbys->Divide(2, 2);
    cgensbys_pt->Divide(2, 2);
    cgensbys_y->Divide(2, 2);
    crecosbys->Divide(2, 2);
    crecosbys_pt->Divide(2, 2);
    crecosbys_y->Divide(2, 2);
    crecosbys_prompt->Divide(2, 2);
    crecosbys_prompt_pt->Divide(2, 2);
    crecosbys_prompt_y->Divide(2, 2);
    crecosbys_nonprompt->Divide(2, 2);
    crecosbys_nonprompt_pt->Divide(2, 2);
    crecosbys_nonprompt_y->Divide(2, 2);
  }

  TCanvas* canEffSbyS_pt = new TCanvas(Form("canEffSbyS_pt_%s", particle.Data()), "Eff", 800, 600);
  canEffSbyS_pt->Divide(2, 2);

  TCanvas* canEffSbyS_y = new TCanvas(Form("canEffSbyS_y_%s", particle.Data()), "Eff", 800, 600);
  canEffSbyS_y->Divide(2, 2);

  TLegend* legendSbyS = new TLegend(0.15, 0.7, 0.45, 0.9);
  legendSbyS->SetFillColorAlpha(0, kWhite);
  legendSbyS->SetLineWidth(0);

  if (effSbyS) {
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
      cgensbys->cd(1);
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

      cgensbys_pt->cd(1);
      hGenPtIncl->Draw("pe");

      cgensbys_y->cd(1);
      hGenYIncl->Draw("pe");

      // prompt generated
      TString nameHistgenPrompt = outputDir + "/hPtvsYGenPrompt"; // generator level pT of generated particles
      TH2F* hPtvsYGenPrompt = (TH2F*)file->Get(nameHistgenPrompt.Data());
      if (!hPtvsYGenPrompt) {
        Printf("Error: Failed to load %s from %s", nameHistgenPrompt.Data(), pathFile.Data());
        return 1;
      }
      cgensbys->cd(2);
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

      cgensbys_pt->cd(2);
      hGenPtPrompt->Draw("pe");

      cgensbys_y->cd(2);
      hGenYPrompt->Draw("pe");

      // non-prompt generated
      TString nameHistgenNonPrompt = outputDir + "/hPtvsYGenNonPrompt"; // generator level pT of generated particles
      TH2F* hPtvsYGenNonPrompt = (TH2F*)file->Get(nameHistgenNonPrompt.Data());
      if (!hPtvsYGenNonPrompt) {
        Printf("Error: Failed to load %s from %s", nameHistgenNonPrompt.Data(), pathFile.Data());
        return 1;
      }
      cgensbys->cd(3);
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

      cgensbys_pt->cd(3);
      hGenPtNonPrompt->Draw("pe");

      cgensbys_y->cd(3);
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
        crecosbys->cd(iRs + 1);
        hPtvsYRecIncl->Draw("colz");

        TH1F* hRecoPtIncl = (TH1F*)hPtvsYRecIncl->ProjectionX(Form("h%s_Pt", recostep[iRs].Data()), 1, hPtvsYRecIncl->GetXaxis()->GetLast(), "e");
        crecosbys_pt->cd(iRs + 1);
        hRecoPtIncl->Draw("pe");

        TH1F* hRecoYIncl = (TH1F*)hPtvsYRecIncl->ProjectionY(Form("h%s_Y", recostep[iRs].Data()), 1, hPtvsYRecIncl->GetYaxis()->GetLast(), "e");
        crecosbys_y->cd(iRs + 1);
        hRecoYIncl->Draw("pe");

        if (iNRebin > 1) {
          hRecoPtIncl->Rebin(iNRebin);
          hRecoYIncl->Rebin(iNRebin);
        } else if (NRebin > 1) {
          hRecoPtIncl = (TH1F*)hRecoPtIncl->Rebin(NRebin, "hRecoPtInclR", dRebin);
          hRecoYIncl = (TH1F*)hRecoYIncl->Rebin(NRebin, "hRecoYInclR", dRebin);
        }

        auto padEffSbyS_pt = canEffSbyS_pt->cd(1);

        TH1F* hEffPtIncl = nullptr;
        hEffPtIncl = (TH1F*)hRecoPtIncl->Clone(Form("hEffPtIncl%s", recostep[iRs].Data()));
        hEffPtIncl->Divide(hEffPtIncl, hGenPtIncl, 1., 1., "B");
        hEffPtIncl->SetTitle("inclusive ;#it{p}^{rec.}_{T} (GeV/#it{c}); efficiency");
        yMin = hEffPtIncl->GetMinimum(0) * 0.2;
        yMax = hEffPtIncl->GetMaximum() * 1.5;
        SetHistogramStyle(hEffPtIncl, colours[iRs], 20, 1.5, 2);
        legendSbyS->AddEntry(hEffPtIncl, Form("%s", recostep[iRs].Data()), "P");

        if (iRs == 0) {
          SetHistogram(hEffPtIncl, yMin, yMax, marginRLow, marginRHigh, logScaleH);
          SetPad(padEffSbyS_pt, logScaleH);
          hEffPtIncl->Draw("pe");
        } else
          hEffPtIncl->Draw("pesame");
        legendSbyS->Draw("same");

        auto padEffSbyS_y = canEffSbyS_y->cd(1);

        TH1F* hEffYIncl = nullptr;
        hEffYIncl = (TH1F*)hRecoYIncl->Clone(Form("hEffYIncl%s", recostep[iRs].Data()));
        hEffYIncl->Divide(hEffYIncl, hGenYIncl, 1., 1., "B");
        hEffYIncl->SetTitle("inclusive ;#it{y}; efficiency");
        yMin = hEffYIncl->GetMinimum(0) * 0.2;
        yMax = hEffYIncl->GetMaximum() * 1.5;
        SetHistogramStyle(hEffYIncl, colours[iRs], 20, 1.5, 2);
        if (iRs == 0) {
          SetHistogram(hEffYIncl, yMin, yMax, marginRLow, marginRHigh, logScaleH);
          SetPad(padEffSbyS_y, logScaleR);
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
        crecosbys_prompt->cd(iRs + 1);
        hPtvsYRecPrompt->Draw("colz");

        TH1F* hRecoPtPrompt = (TH1F*)hPtvsYRecPrompt->ProjectionX(Form("h%s_Pt_prompt", recostep[iRs].Data()), 1, hPtvsYRecPrompt->GetXaxis()->GetLast(), "e");
        crecosbys_prompt_pt->cd(iRs + 1);
        hRecoPtPrompt->Draw("pe");

        TH1F* hRecoYPrompt = (TH1F*)hPtvsYRecPrompt->ProjectionY(Form("h%s_Y_prompt", recostep[iRs].Data()), 1, hPtvsYRecPrompt->GetYaxis()->GetLast(), "e");
        crecosbys_prompt_y->cd(iRs + 1);
        hRecoYPrompt->Draw("pe");

        if (iNRebin > 1) {
          hRecoPtPrompt->Rebin(iNRebin);
          hRecoYPrompt->Rebin(iNRebin);
        } else if (NRebin > 1) {
          hRecoPtPrompt = (TH1F*)hRecoPtPrompt->Rebin(NRebin, "hRecoPtPromptR", dRebin);
          hRecoYPrompt = (TH1F*)hRecoYPrompt->Rebin(NRebin, "hRecoYPromptR", dRebin);
        }

        auto padEffPromptSbyS_pt = canEffSbyS_pt->cd(2);

        TH1F* hEffPtPrompt = nullptr;
        hEffPtPrompt = (TH1F*)hRecoPtPrompt->Clone(Form("hEffPtPrompt%s", recostep[iRs].Data()));
        hEffPtPrompt->Divide(hEffPtPrompt, hGenPtPrompt, 1., 1., "B");
        hEffPtPrompt->SetTitle("prompt ;#it{p}^{rec.}_{T} (GeV/#it{c}); efficiency");
        yMin = hEffPtPrompt->GetMinimum(0) * 0.2;
        yMax = hEffPtPrompt->GetMaximum() * 1.5;
        SetHistogramStyle(hEffPtPrompt, colours[iRs], 20, 1.5, 2);

        if (iRs == 0) {
          SetHistogram(hEffPtPrompt, yMin, yMax, marginRLow, marginRHigh, logScaleH);
          SetPad(padEffPromptSbyS_pt, logScaleH);
          hEffPtPrompt->Draw("pe");
        } else
          hEffPtPrompt->Draw("pesame");
        legendSbyS->Draw("same");

        auto padEffPromptSbyS_y = canEffSbyS_y->cd(2);

        TH1F* hEffYPrompt = nullptr;
        hEffYPrompt = (TH1F*)hRecoYPrompt->Clone(Form("hEffYPrompt%s", recostep[iRs].Data()));
        hEffYPrompt->Divide(hEffYPrompt, hGenYPrompt, 1., 1., "B");
        hEffYPrompt->SetTitle("prompt ;#it{y}; efficiency");
        yMin = hEffYPrompt->GetMinimum(0) * 0.2;
        yMax = hEffYPrompt->GetMaximum() * 1.5;
        SetHistogramStyle(hEffYPrompt, colours[iRs], 20, 1.5, 2);
        if (iRs == 0) {
          SetHistogram(hEffYPrompt, yMin, yMax, marginRLow, marginRHigh, logScaleH);
          SetPad(padEffPromptSbyS_y, logScaleR);
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
        crecosbys_nonprompt->cd(iRs + 1);
        hPtvsYRecNonPrompt->Draw("colz");

        TH1F* hRecoPtNonPrompt = (TH1F*)hPtvsYRecNonPrompt->ProjectionX(Form("h%s_Pt_nonprompt", recostep[iRs].Data()), 1, hPtvsYRecNonPrompt->GetXaxis()->GetLast(), "e");
        crecosbys_nonprompt_pt->cd(iRs + 1);
        hRecoPtNonPrompt->Draw("pe");

        TH1F* hRecoYNonPrompt = (TH1F*)hPtvsYRecNonPrompt->ProjectionY(Form("h%s_Y_nonprompt", recostep[iRs].Data()), 1, hPtvsYRecNonPrompt->GetYaxis()->GetLast(), "e");
        crecosbys_nonprompt_y->cd(iRs + 1);
        hRecoYNonPrompt->Draw("pe");

        if (iNRebin > 1) {
          hRecoPtNonPrompt->Rebin(iNRebin);
          hRecoYNonPrompt->Rebin(iNRebin);
        } else if (NRebin > 1) {
          hRecoPtNonPrompt = (TH1F*)hRecoPtNonPrompt->Rebin(NRebin, "hRecoPtNonPromptR", dRebin);
          hRecoYNonPrompt = (TH1F*)hRecoYNonPrompt->Rebin(NRebin, "hRecoYNonPromptR", dRebin);
        }

        auto padEffNonPromptSbyS_pt = canEffSbyS_pt->cd(3);

        TH1F* hEffPtNonPrompt = nullptr;
        hEffPtNonPrompt = (TH1F*)hRecoPtNonPrompt->Clone(Form("hEffPtNonPrompt%s", recostep[iRs].Data()));
        hEffPtNonPrompt->Divide(hEffPtNonPrompt, hGenPtNonPrompt, 1., 1., "B");
        hEffPtNonPrompt->SetTitle("non-prompt ;#it{p}^{rec.}_{T} (GeV/#it{c}); efficiency");
        yMin = hEffPtNonPrompt->GetMinimum(0) * 0.2;
        yMax = hEffPtNonPrompt->GetMaximum() * 1.5;
        SetHistogramStyle(hEffPtNonPrompt, colours[iRs], 20, 1.5, 2);

        if (iRs == 0) {
          SetHistogram(hEffPtNonPrompt, yMin, yMax, marginRLow, marginRHigh, logScaleH);
          SetPad(padEffNonPromptSbyS_pt, logScaleH);
          hEffPtNonPrompt->Draw("pe");
        } else
          hEffPtNonPrompt->Draw("pesame");
        legendSbyS->Draw("same");

        auto padEffNonPromptSbyS_y = canEffSbyS_y->cd(3);

        TH1F* hEffYNonPrompt = nullptr;
        hEffYNonPrompt = (TH1F*)hRecoYNonPrompt->Clone(Form("hEffYNonPrompt%s", recostep[iRs].Data()));
        hEffYNonPrompt->Divide(hEffYNonPrompt, hGenYNonPrompt, 1., 1., "B");
        hEffYNonPrompt->SetTitle("non-prompt ;#it{y}; efficiency");
        yMin = hEffYNonPrompt->GetMinimum(0) * 0.2;
        yMax = hEffYNonPrompt->GetMaximum() * 1.5;
        SetHistogramStyle(hEffYNonPrompt, colours[iRs], 20, 1.5, 2);
        if (iRs == 0) {
          SetHistogram(hEffYNonPrompt, yMin, yMax, marginRLow, marginRHigh, logScaleH);
          SetPad(padEffNonPromptSbyS_y, logScaleR);
          hEffYNonPrompt->Draw("pe");
        } else
          hEffYNonPrompt->Draw("pesame");
        legendSbyS->Draw("same");
      }
    }
    canEffSbyS_pt->SaveAs(Form("MC_%s_eff_stepbystep_pT.pdf", particle.Data()));
    canEffSbyS_y->SaveAs(Form("MC_%s_eff_stepbystep_y.pdf", particle.Data()));
  }

  delete arrayParticle;
  return 0;
}
