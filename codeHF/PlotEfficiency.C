// Plotting of reconstruction efficiency

#include "utils_plot.h"

Int_t PlotEfficiency(TString pathFile = "AnalysisResults.root", TString particles = "d0")
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
  int colours[] = {1, 2, 4};
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

  // loop over particles
  for (int iP = 0; iP < arrayParticle->GetEntriesFast(); iP++) {
    particle = ((TObjString*)(arrayParticle->At(iP)))->GetString();
    if (!particle.Length()) {
      Printf("Error: Empty particle string");
      return 1;
    }
    Printf("\nPlotting efficiency for: %s", particle.Data());

    TString outputDir = Form("hf-task-%s", particle.Data()); // analysis output directory with histograms
    
    TString nameHistRec;
    TString nameHistgen;

    // inclusive candidates
    if (particles==" lc ") {
      nameHistRec = "hf-task-lc/MC/reconstructed/signal/hPtRecSig"; // reconstruction level pT of matched candidates
      nameHistgen = "hf-task-lc/MC/generated/signal/hPtGen"; // generator level pT of generated particles
    } else {
      nameHistRec = outputDir + "/hPtRecSig"; // reconstruction level pT of matched candidates
      //nameHistRec = outputDir + "/hPtGenSig"; // generator level pT of matched candidates (no pT smearing)
      nameHistgen = outputDir + "/hPtGen"; // generator level pT of generated particles
    }
    

    TH1F* hPtRecIncl = (TH1F*)file->Get(nameHistRec.Data());
    if (!hPtRecIncl) {
      hPtRecIncl = (TH1F*)file->Get("hf-task-lc/MC/reconstructed/signal/hPtRecSig");
      if (!hPtRecIncl) {
        Printf("Error: Failed to load %s from %s", nameHistRec.Data(), pathFile.Data());
        return 1;
      }
    }
    TH1F* hPtGenIncl = (TH1F*)file->Get(nameHistgen.Data());
    if (!hPtGenIncl) {
      hPtGenIncl = (TH1F*)file->Get("hf-task-lc/MC/generated/signal/hPtGen");
      if (!hPtGenIncl) {
        Printf("Error: Failed to load %s from %s", nameHistgen.Data(), pathFile.Data());
        return 1;
      }
    }

    // prompt candidates
    bool okPrompt = true;
    if (particles == "lc")
    {
      nameHistRec = "MC/reconstructed/prompt/hPtRecSigPrompt";
      nameHistgen = "MC/generated/prompt/hPtGenPrompt";
    } else {
      nameHistRec = outputDir + "/hPtRecSigPrompt";
      nameHistgen = outputDir + "/hPtGenPrompt";
    }
    
    
    TH1F* hPtRecPrompt = (TH1F*)file->Get(nameHistRec.Data());
    if (!hPtRecPrompt) {
      hPtRecPrompt = (TH1F*)file->Get("MC/reconstructed/prompt/hPtRecSigPrompt");
      if (!hPtRecPrompt) {
        Printf("Warning: Failed to load %s from %s", nameHistRec.Data(), pathFile.Data());
        okPrompt = false;
      }
    }
    TH1F* hPtGenPrompt = (TH1F*)file->Get(nameHistgen.Data());
    if (!hPtGenPrompt) {
      hPtGenPrompt = (TH1F*)file->Get("MC/generated/prompt/hPtGenPrompt");
      if (!hPtGenPrompt) {
        Printf("Warning: Failed to load %s from %s", nameHistgen.Data(), pathFile.Data());
        okPrompt = false;
      }
    }

    if (particles == "lc")
    {
      nameHistRec = "MC/reconstructed/prompt/hPtRecSigNonPrompt";
      nameHistgen = "MC/generated/prompt/hPtGenNonPrompt";
    } else {
      nameHistRec = outputDir + "/hPtRecSigNonPrompt";
      nameHistgen = outputDir + "/hPtGenNonPrompt";
    }

    // non-prompt candidates
    bool okNonPrompt = true;
    TH1F* hPtRecNonPrompt = (TH1F*)file->Get(nameHistRec.Data());
    if (!hPtRecNonPrompt) {
      hPtRecNonPrompt = (TH1F*)file->Get("MC/reconstructed/prompt/hPtRecSigNonPrompt");
      if (!hPtRecNonPrompt) {
        Printf("Warning: Failed to load %s from %s", nameHistRec.Data(), pathFile.Data());
        okNonPrompt = false;
      }
    }
    TH1F* hPtGenNonPrompt = (TH1F*)file->Get(nameHistgen.Data());
    if (!hPtGenNonPrompt) {
      hPtGenNonPrompt = (TH1F*)file->Get("MC/generated/prompt/hPtGenNonPrompt");
      if (!hPtGenNonPrompt) {
        Printf("Warning: Failed to load %s from %s", nameHistgen.Data(), pathFile.Data());
        okNonPrompt = false;
      }
    }

    TCanvas* canPt = new TCanvas(Form("canPt_%s", particle.Data()), "Pt", 1200, 1000);
    TLegend* legendPt = new TLegend(0.72, 0.72, 0.92, 0.92);
    TCanvas* canEff = new TCanvas(Form("canEff_%s", particle.Data()), "Eff", 1200, 1000);
    TLegend* legendEff = new TLegend(0.1, 0.72, 0.3, 0.92);

    nGen = hPtGenIncl->GetEntries();
    nRec = hPtRecIncl->GetEntries();

    // pT spectra
    auto padPt = canPt->cd();
    // inclusive
    if (iNRebin > 1) {
      hPtGenIncl->Rebin(iNRebin);
      hPtRecIncl->Rebin(iNRebin);
    } else if (NRebin > 1) {
      hPtGenIncl = (TH1F*)hPtGenIncl->Rebin(NRebin, "hPtGenInclR", dRebin);
      hPtRecIncl = (TH1F*)hPtRecIncl->Rebin(NRebin, "hPtRecInclR", dRebin);
    }
    yMin = std::min(hPtRecIncl->GetMinimum(0), hPtGenIncl->GetMinimum(0));
    yMax = std::max(hPtRecIncl->GetMaximum(), hPtGenIncl->GetMaximum());
    SetHistogramStyle(hPtGenIncl, colours[0], markers[0], 1.5, 2);
    SetHistogramStyle(hPtRecIncl, colours[0], markers[0], 1.5, 2);
    hPtGenIncl->SetTitle(Form("Entries: Rec: %d, Gen: %d;#it{p}_{T} (GeV/#it{c});entries", nRec, nGen));
    hPtGenIncl->GetYaxis()->SetMaxDigits(3);
    // prompt
    if (okPrompt) {
      if (iNRebin > 1) {
        hPtGenPrompt->Rebin(iNRebin);
        hPtRecPrompt->Rebin(iNRebin);
      } else if (NRebin > 1) {
        hPtGenPrompt = (TH1F*)hPtGenPrompt->Rebin(NRebin, "hPtGenPromptR", dRebin);
        hPtRecPrompt = (TH1F*)hPtRecPrompt->Rebin(NRebin, "hPtRecPromptR", dRebin);
      }
      yMin = std::min({yMin, hPtRecPrompt->GetMinimum(0), hPtGenPrompt->GetMinimum(0)});
      yMax = std::max({yMax, hPtRecPrompt->GetMaximum(), hPtGenPrompt->GetMaximum()});
      SetHistogramStyle(hPtGenPrompt, colours[1], markers[1], 1.5, 2);
      SetHistogramStyle(hPtRecPrompt, colours[1], markers[1], 1.5, 2);
    }
    // non-prompt
    if (okNonPrompt) {
      if (iNRebin > 1) {
        hPtGenNonPrompt->Rebin(iNRebin);
        hPtRecNonPrompt->Rebin(iNRebin);
      } else if (NRebin > 1) {
        hPtGenNonPrompt = (TH1F*)hPtGenNonPrompt->Rebin(NRebin, "hPtGenNonPromptR", dRebin);
        hPtRecNonPrompt = (TH1F*)hPtRecNonPrompt->Rebin(NRebin, "hPtRecNonPromptR", dRebin);
      }
      yMin = std::min({yMin, hPtRecNonPrompt->GetMinimum(0), hPtGenNonPrompt->GetMinimum(0)});
      yMax = std::max({yMax, hPtRecNonPrompt->GetMaximum(), hPtGenNonPrompt->GetMaximum()});
      SetHistogramStyle(hPtGenNonPrompt, colours[2], markers[2], 1.5, 2);
      SetHistogramStyle(hPtRecNonPrompt, colours[2], markers[2], 1.5, 2);
    }
    SetHistogram(hPtGenIncl, yMin, yMax, marginLow, marginHigh, logScaleH);
    SetPad(padPt, logScaleH);
    hPtGenIncl->Draw();
    hPtRecIncl->Draw("same EP");
    legendPt->AddEntry(hPtRecIncl, "inclusive rec", "P");
    legendPt->AddEntry(hPtGenIncl, "inclusive gen", "L");
    if (okPrompt) {
      hPtGenPrompt->Draw("same");
      hPtRecPrompt->Draw("same EP");
      legendPt->AddEntry(hPtRecPrompt, "prompt rec", "P");
      legendPt->AddEntry(hPtGenPrompt, "prompt gen", "L");
    }
    if (okNonPrompt) {
      hPtGenNonPrompt->Draw("same");
      hPtRecNonPrompt->Draw("same EP");
      legendPt->AddEntry(hPtRecNonPrompt, "non-prompt rec", "P");
      legendPt->AddEntry(hPtGenNonPrompt, "non-prompt gen", "L");
    }
    legendPt->Draw();

    // efficiency
    auto padEff = canEff->cd();
    TH1F* hEffIncl = nullptr;
    TH1F* hEffPrompt = nullptr;
    TH1F* hEffNonPrompt = nullptr;
    // inclusive
    hEffIncl = (TH1F*)hPtRecIncl->Clone("hEffIncl");
    hEffIncl->Divide(hEffIncl, hPtGenIncl, 1., 1., "B");
    yMin = hEffIncl->GetMinimum(0);
    yMax = hEffIncl->GetMaximum();
    SetHistogramStyle(hEffIncl, colours[0], markers[0], 1.5, 2);
    hEffIncl->SetTitle(Form("Entries ratio: %g;#it{p}_{T} (GeV/#it{c});reconstruction efficiency", (double)nRec / (double)nGen));
    // prompt
    if (okPrompt) {
      hEffPrompt = (TH1F*)hPtRecPrompt->Clone("hEffPrompt");
      hEffPrompt->Divide(hPtRecPrompt, hPtGenPrompt, 1., 1., "B");
      yMin = std::min(yMin, hEffPrompt->GetMinimum(0));
      yMax = std::max(yMax, hEffPrompt->GetMaximum());
      SetHistogramStyle(hEffPrompt, colours[1], markers[1], 1.5, 2);
    }
    // non-prompt
    if (okNonPrompt) {
      hEffNonPrompt = (TH1F*)hPtRecNonPrompt->Clone("hEffNonPrompt");
      hEffNonPrompt->Divide(hPtRecNonPrompt, hPtGenNonPrompt, 1., 1., "B");
      yMin = std::min(yMin, hEffNonPrompt->GetMinimum(0));
      yMax = std::max(yMax, hEffNonPrompt->GetMaximum());
      SetHistogramStyle(hEffNonPrompt, colours[2], markers[2], 1.5, 2);
    }
    SetHistogram(hEffIncl, yMin, yMax, marginRLow, marginRHigh, logScaleR);
    SetPad(padEff, logScaleR);
    hEffIncl->Draw("EP");
    legendEff->AddEntry(hEffIncl, "inclusive", "P");
    if (okPrompt) {
      hEffPrompt->Draw("same EP");
      legendEff->AddEntry(hEffPrompt, "prompt", "P");
    }
    if (okNonPrompt) {
      hEffNonPrompt->Draw("same EP");
      legendEff->AddEntry(hEffNonPrompt, "non-prompt", "P");
    }
    legendEff->Draw();

    canPt->SaveAs(Form("MC_%s_pT.pdf", particle.Data()));
    canEff->SaveAs(Form("MC_%s_eff.pdf", particle.Data()));
  }
  delete arrayParticle;
  return 0;
}
