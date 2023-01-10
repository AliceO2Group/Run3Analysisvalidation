///////////////////////////////////////////////////////////////////////////
//  Macro to produce 2D histograms of correlations vs. delta phi and delta eta
//
//  This macro allows you to process the content of the CorrelationContainer
//  for both same-event and mixed-event correlations and obtain the 2D correlation plot.
//  1/N_trig * dN/ddeltaphi*ddeltaeta = dN_same/ddeltaphi*ddeltaeta / dN_mixed/ddeltaphi*ddeltaeta
//  Both numerator (same-event) and denominator (mixed-event) are properly normalised.
//
//  Input: file with histograms produced by o2-analysis-hf-task-flow
//
//  Usage: root -l extract2D.C
//
//  Parameters:
//  - fileName: input file
//  - folder: name of the folder created by the o2-analysis-hf-task-flow,
//            contains the results stored in CorrelationContainer
//  - outputFile: output file containing final 2D correlation histograms in delta phi, delta eta
//  - outputPlots: folder to store produced plots
//  - plotExamplePlots: flag to draw correlation plots in "surf" mode in selected (wide) ranges of multiplicity and pT
//  - saveSameEventDis: flag to also save 2D histograms of same-event correlations
//  - hfcase: flag to perform these calculations for HF-h correlations
//
//  Contributors:
//    Katarina Krizkova Gajdosova <katarina.gajdosova@cern.ch>
//    Gian Michele Innocenti <gian.michele.innocenti@cern.ch>
//    Jan Fiete Grosse-Oetringhaus <Jan.Fiete.Grosse-Oetringhaus@cern.ch>
///////////////////////////////////////////////////////////////////////////

Float_t gpTMin = 0.21;
Float_t gpTMax = 2.99;
Float_t gEtaMin = -0.8;
Float_t gEtaMax = 0.8;
Float_t gZVtxMin = -10;
Float_t gZVtxMax = 10;

void SetupRanges(CorrelationContainer* obj)
{
  obj->setEtaRange(0, 0);
  //obj->setEtaRange(gEtaMin, gEtaMax);
  obj->setPtRange(gpTMin, gpTMax);
  obj->setZVtxRange(gZVtxMin + 0.01, gZVtxMax - 0.01);
}

///////////////////////////////////////////////////////////////////////////
//  Function to average the 2D correlations over Vz bins normalised for
//  the value in (0,0) bin. It is also divided by the number of trigger particles.
//  Note: instead of just taking a value in one (0,0) bin, there is an option to
//        do an average of values in delta eta = 0 over all delta phi bins
//        to account for fluctuations
///////////////////////////////////////////////////////////////////////////
void GetSumOfRatios(CorrelationContainer* h, CorrelationContainer* hMixed, TH1** hist, CorrelationContainer::CFStep step, Float_t centralityBegin, Float_t centralityEnd, Float_t ptBegin, Float_t ptEnd, Bool_t normalizePerTrigger = kTRUE)
{
  Printf("GetSumOfRatios | step %d | %.1f-%.1f%% | %.1f - %.1f GeV/c | %.1f - %.1f GeV/c", step, centralityBegin, centralityEnd, gpTMin, gpTMax, ptBegin, ptEnd);

  h->setCentralityRange(0.01 + centralityBegin, -0.01 + centralityEnd);
  hMixed->setCentralityRange(0.01 + centralityBegin, -0.01 + centralityEnd);
  *hist = h->getSumOfRatios(hMixed, step, ptBegin, ptEnd, normalizePerTrigger);

  TString str;
  str.Form("%.1f < p_{T,trig} < %.1f", ptBegin - 0.01, ptEnd + 0.01);

  TString str2;
  str2.Form("%.2f < p_{T,assoc} < %.2f", gpTMin - 0.01, gpTMax + 0.01);

  TString newTitle;
  newTitle.Form("%s - %s - %.0f-%.0f", str.Data(), str2.Data(), centralityBegin, centralityEnd);
  if ((*hist))
    (*hist)->SetTitle(newTitle);
}

///////////////////////////////////////////////////////////////////////////
//  Function to plot same-event 2D correlations in a particular Vz bin
//  It is also divided by the number of trigger particles and dphi bin width.
///////////////////////////////////////////////////////////////////////////
void GetSameEventCorrelation(CorrelationContainer* h, TH2** hist, CorrelationContainer::CFStep step, Float_t centralityBegin, Float_t centralityEnd, Float_t ptBegin, Float_t ptEnd, Bool_t normalizePerTrigger = kTRUE)
{
  Printf("GetSameEventCorrelation | step %d | %.1f < Nch < %.1f | %.1f - %.1f GeV/c | %.1f - %.1f GeV/c \n", step, centralityBegin, centralityEnd, ptBegin, ptEnd, gpTMin, gpTMax);

  h->setCentralityRange(0.01 + centralityBegin, -0.01 + centralityEnd);

  *hist = h->getPerTriggerYield(step, ptBegin, ptEnd, normalizePerTrigger);

  TString str;
  str.Form("%.1f < p_{T,trig} < %.1f", ptBegin - 0.01, ptEnd + 0.01);

  TString str2;
  str2.Form("%.1f < p_{T,assoc} < %.1f", gpTMin - 0.01, gpTMax + 0.01);

  TString newTitle;
  newTitle.Form("%s - %s , mult %.0f-%.0f, vz %.0f - %.0f", str.Data(), str2.Data(), centralityBegin, centralityEnd, gZVtxMin, gZVtxMax);
  if ((*hist))
    (*hist)->SetTitle(newTitle);
}

///////////////////////////////////////////////////////////////////////////
//  Function to draw a projection of specific axis of the CorrelationContainer
//  It is mostly meant as a quick QA of the filled distributions
///////////////////////////////////////////////////////////////////////////
TH1* GetProjectionOfAxis(CorrelationContainer* h, CorrelationContainer::CFStep step, bool pairhist = false, int naxis = 6)
{
  THnBase* sparse;
  if (pairhist)
    sparse = h->getPairHist()->getTHn(step);
  else
    sparse = h->getTriggerHist()->getTHn(step);

  TH1* hprojection = (TH1*)sparse->Projection(naxis);
  return hprojection;
}

///////////////////////////////////////////////////////////////////////////
//  Main function
///////////////////////////////////////////////////////////////////////////
void extract2D(
  const char* fileName = "AnalysisResults.root",
  const char* folder = "hf-task-flow",
  const char* outputFile = "dphi_corr.root",
  const char* outputPlots = "./plots",
  bool plotExamplePlots = false,
  bool saveSameEventDis = false,
  bool hfcase = false)
{
  gStyle->SetOptStat(1111111);

  CorrelationContainer::CFStep step = CorrelationContainer::kCFStepReconstructed;
  Bool_t normalizePerTrigger = kTRUE; // don't do this if histograms are to be merged with other periods later -> Use MergeDPhiFiles below

  auto file = TFile::Open(outputFile, "RECREATE");
  file->Close();

  // the interval below defines the pttrig, ptass and multiplicity (or
  // centrality in which the analysis will be performed. One "ridge" plot
  // with the correct normalization will be obtained for each of these
  // combinations (with the condition that the selection on the associated pt
  // has to be looser than the one on the trigger particle.

  Int_t maxLeadingPtReferenceFlow = 1;
  Float_t leadingPtReferenceFlowArr[] = {0.2, 3.0}; // reference flow is done in one wide pT bin

  Int_t maxLeadingPt = 6;
  Float_t leadingPtArr[] = {0.2, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0};

  Int_t maxAssocPt = 1;
  Float_t assocPtArr[] = {0.2, 3.0};

  Int_t maxCentrality = 9;
  Float_t centralityArr[] = {0, 10, 20, 30, 40, 50, 60, 80, 100, 200};

  // h is the container of the same event
  // hMixed is the container of the mixed event
  auto inputFile = TFile::Open(fileName);
  const char* directoryh = "";
  const char* directoryhMixed = "";
  if (hfcase) {
    directoryh = Form("%s/sameEventHFHadrons", folder);
    directoryhMixed = Form("%s/mixedEventHFHadrons", folder);
  } else {
    directoryh = Form("%s/sameEventTPCTPCChHadrons", folder);
    directoryhMixed = Form("%s/mixedEventTPCTPCChHadrons", folder);
  }
  auto h = (CorrelationContainer*)inputFile->Get(Form("%s", directoryh));
  auto hMixed = (CorrelationContainer*)inputFile->Get(Form("%s", directoryhMixed));

  //  auto proj = GetProjectionOfAxis(h, step, false, 0);
  //  proj->Draw();
  //  printf("entires proj: %g \n", proj->GetEntries());
  //  return;

  // the number of events for each multiplicity/centrality class is calculated and printed.
  auto eventHist = h->getEventCount();
  Printf("Events with centrality: %d", (int)eventHist->Integral(eventHist->GetXaxis()->FindBin(6.), eventHist->GetXaxis()->FindBin(6.), eventHist->GetYaxis()->FindBin(0.), eventHist->GetYaxis()->FindBin(99.9)));
  Printf("Events: %d", (int)eventHist->ProjectionX()->Integral(eventHist->GetXaxis()->FindBin(6.), eventHist->GetXaxis()->FindBin(6.)));
  //   eventHist->ProjectionX()->Draw();
  //   return;

  if (plotExamplePlots) {
    //  low multiplicity illustrative histograms
    h->setPtRange(gpTMin, gpTMax);
    h->setCentralityRange(10, 20);
    h->setZVtxRange(-10, 10);
    hMixed->setPtRange(gpTMin, gpTMax);
    hMixed->setCentralityRange(10, 20);
    hMixed->setZVtxRange(-10, 10);

    TH2* sameTwoD = (TH2*)h->getPerTriggerYield(step, 0.2, 2.99, true);
    auto c1 = new TCanvas;
    sameTwoD->Draw("SURF1");
    sameTwoD->SetStats(0);
    sameTwoD->SetTitle("");
    sameTwoD->GetXaxis()->SetTitleOffset(1.5);
    c1->SaveAs(Form("%s/general/sameCorr.png", outputPlots));

    //  Note: this is not normalised for the bin (0,0),
    TH2* mixedTwoD = (TH2*)hMixed->getPerTriggerYield(step, 0.2, 2.99, false);
    auto c2 = new TCanvas;
    mixedTwoD->Draw("SURF1");
    mixedTwoD->SetStats(0);
    mixedTwoD->SetTitle("");
    mixedTwoD->GetXaxis()->SetTitleOffset(1.5);
    c2->SaveAs(Form("%s/general/mixedCorr.png", outputPlots));

    auto ratio = (TH2*)sameTwoD->Clone("ratio");
    ratio->Divide(mixedTwoD);

    auto c3 = new TCanvas;
    ratio->GetYaxis()->SetRangeUser(-1.59, 1.59);
    ratio->GetXaxis()->SetTitleOffset(1.5);
    ratio->SetStats(0);
    ratio->SetTitle("");
    ratio->Draw("SURF1");
    c3->SaveAs(Form("%s/general/pertriggeryield.png", outputPlots));

    auto c4 = new TCanvas;
    ratio->SetMaximum(0.46e-09);
    ratio->Draw("SURF1");
    c4->SaveAs(Form("%s/general/pertriggeryield_zoom.png", outputPlots));

    //  high multiplicity illustrative histograms
    h->setPtRange(gpTMin, gpTMax);
    h->setCentralityRange(40, 100);
    h->setZVtxRange(-10, 10);
    hMixed->setPtRange(gpTMin, gpTMax);
    hMixed->setCentralityRange(40, 100);
    hMixed->setZVtxRange(-10, 10);

    TH2* sameTwoDHM = (TH2*)h->getPerTriggerYield(step, 0.2, 2.99, true);
    auto c1HM = new TCanvas;
    sameTwoDHM->Draw("SURF1");
    sameTwoDHM->SetStats(0);
    sameTwoDHM->SetTitle("");
    sameTwoDHM->GetXaxis()->SetTitleOffset(1.5);
    c1HM->SaveAs(Form("%s/general/sameCorr_HM.png", outputPlots));

    //  Note: this is not normalised for the bin (0,0)
    TH2* mixedTwoDHM = (TH2*)hMixed->getPerTriggerYield(step, 0.2, 2.99, false);
    auto c2HM = new TCanvas;
    mixedTwoDHM->Draw("SURF1");
    mixedTwoDHM->SetStats(0);
    mixedTwoDHM->SetTitle("");
    mixedTwoDHM->GetXaxis()->SetTitleOffset(1.5);
    c2HM->SaveAs(Form("%s/general/mixedCorr_HM.png", outputPlots));

    auto ratioHM = (TH2*)sameTwoDHM->Clone("ratioHM");
    ratioHM->Divide(mixedTwoDHM);

    auto c3HM = new TCanvas;
    ratioHM->GetYaxis()->SetRangeUser(-1.59, 1.59);
    ratioHM->GetXaxis()->SetTitleOffset(1.5);
    ratioHM->SetStats(0);
    ratioHM->SetTitle("");
    ratioHM->Draw("SURF1");
    c3HM->SaveAs(Form("%s/general/pertriggeryield_HM.png", outputPlots));

    auto c4HM = new TCanvas;
    ratioHM->SetMaximum(0.1e-06);
    ratioHM->Draw("SURF1");
    c4HM->SaveAs(Form("%s/general/pertriggeryield_HM_zoom.png", outputPlots));
  }
  //  TODO: when doing HF correlations, make sure to project/integrate the correct inv. mass range/axis

  //  do correlations for reference flow
  //  Correlation is made for each choice of multiplicity.
  //  The same event correlations is divided by mixed event after normalization
  //  for each mult bin (will be integrated over all Vz)
  gpTMin = assocPtArr[0] + 0.01;
  gpTMax = assocPtArr[1] - 0.01;
  gZVtxMin = -10;
  gZVtxMax = 10;

  SetupRanges(h);
  SetupRanges(hMixed);

  for (int mult = 0; mult < maxCentrality; mult++) {
    TH1* hist1 = 0;
    GetSumOfRatios(h, hMixed, &hist1, step, centralityArr[mult], centralityArr[mult + 1], leadingPtReferenceFlowArr[0] + 0.01, leadingPtReferenceFlowArr[1] - 0.01, normalizePerTrigger);

    file = TFile::Open(outputFile, "UPDATE");

    if (hist1) {
      hist1->SetName(Form("dphi_ref_%d", mult));
      hist1->Write();
    }

    file->Close();

    delete hist1;
  }

  //  Get same-event 2D distributions (may be useful for some checks)
  if (saveSameEventDis) {
    SetupRanges(h);

    for (Int_t mult = 0; mult < maxCentrality; mult++) {

      TH2* histSame2D = 0;
      GetSameEventCorrelation(h, &histSame2D, step, centralityArr[mult], centralityArr[mult + 1], leadingPtReferenceFlowArr[0] + 0.01, leadingPtReferenceFlowArr[1] - 0.01, normalizePerTrigger);

      file = TFile::Open(outputFile, "UPDATE");

      if (histSame2D) {
        histSame2D->SetName(Form("same_ref_%d", mult));
        histSame2D->Write();
      }

      file->Close();

      delete histSame2D;
    }
  }

  //  do correlations for pT-differential flow
  //  Correlation is made for each choice of pttrig, ptassoc and multiplicity.
  //  The same event correlations is divided by mixed event after normalization
  //  for each pTtrig, pTassoc and mult bin (will be integrated over all Vz)
  for (Int_t i = 0; i < maxLeadingPt; i++) {
    for (Int_t j = 0; j < maxAssocPt; j++) {
      if (assocPtArr[j] >= leadingPtArr[i + 1]) {
        continue;
      }

      gpTMin = assocPtArr[j] + 0.01;
      gpTMax = assocPtArr[j + 1] - 0.01;
      gZVtxMin = -10;
      gZVtxMax = 10;

      SetupRanges(h);
      SetupRanges(hMixed);

      for (Int_t mult = 0; mult < maxCentrality; mult++) {

        TH1* hist1 = 0;
        GetSumOfRatios(h, hMixed, &hist1, step, centralityArr[mult], centralityArr[mult + 1], leadingPtArr[i] + 0.01, leadingPtArr[i + 1] - 0.01, normalizePerTrigger);

        file = TFile::Open(outputFile, "UPDATE");

        if (hist1) {
          hist1->SetName(Form("dphi_%d_%d_%d", i, j, mult));
          hist1->Write();
        }

        file->Close();

        delete hist1;
      }

      //  Below, get same-event 2D distributions (may be useful for some checks)
      if (!saveSameEventDis)
        continue;

      SetupRanges(h);

      for (Int_t mult = 0; mult < maxCentrality; mult++) {

        TH2* histSame2D = 0;
        GetSameEventCorrelation(h, &histSame2D, step, centralityArr[mult], centralityArr[mult + 1], leadingPtArr[i] + 0.01, leadingPtArr[i + 1] - 0.01, normalizePerTrigger);

        file = TFile::Open(outputFile, "UPDATE");

        if (histSame2D) {
          histSame2D->SetName(Form("same_%d_%d_%d", i, j, mult));
          histSame2D->Write();
        }

        file->Close();

        delete histSame2D;
      }
    }

    // triggers contain the information about the number of trigger particles
    // in each interval of multiplicity (used for the result normalization)

    gZVtxMin = -10;
    gZVtxMax = 10;

    SetupRanges(h);

    TH1* triggers = h->getTriggersAsFunctionOfMultiplicity(step, leadingPtArr[i] + 0.01, leadingPtArr[i + 1] - 0.01);
    triggers->SetName(Form("triggers_%d", i));
    TString str;
    str.Form("%.1f < p_{T,trig} < %.1f", leadingPtArr[i], leadingPtArr[i + 1]);
    triggers->SetTitle(str);

    file = TFile::Open(outputFile, "UPDATE");
    triggers->Write();
    file->Close();
  }

  delete h;
  delete hMixed;
}
