Float_t gpTMin = 0.21;
Float_t gpTMax = 2.99;
Float_t gEtaMin = -2.41;
Float_t gEtaMax = 2.39;
Float_t gZVtxMin = -7.01;
Float_t gZVtxMax = 6.99;

void SetupRanges(CorrelationContainer* obj)
{
  obj->setEtaRange(gEtaMin, gEtaMax);
  obj->setPtRange(gpTMin, gpTMax);
  obj->setZVtxRange(gZVtxMin, gZVtxMax);
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

  h->setCentralityRange(centralityBegin, centralityEnd);
  hMixed->setCentralityRange(centralityBegin, centralityEnd);
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
void GetSameEventCorrelation(CorrelationContainer* h, TH2** hist, CorrelationContainer::CFStep step, Float_t centralityBegin, Float_t centralityEnd, Float_t ptBegin, Float_t ptEnd)
{
  Printf("GetSameEventCorrelation | step %d | %.1f < Nch < %.1f | %.1f - %.1f GeV/c | %.1f - %.1f GeV/c \n", step, centralityBegin, centralityEnd, ptBegin, ptEnd, gpTMin, gpTMax);

  h->setCentralityRange(centralityBegin, centralityEnd);

  *hist = h->getPerTriggerYield(step, ptBegin, ptEnd);

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
//  This is a function to draw a projection of specific axis of the CorrelationContainer
//  It is mostly meant as a quick QA of the filled distributions
///////////////////////////////////////////////////////////////////////////
TH1* GetProjectionOfAxis(CorrelationContainer* h, CorrelationContainer::CFStep step, int naxis = 6)
{
  THnBase *sparse = h->getPairHist()->getTHn(step);
  printf("n bins: %.0d \n", sparse->GetAxis(naxis)->GetNbins());

  TH1 *hprojection = (TH1*)sparse->Projection(naxis);
  return hprojection;
}

///////////////////////////////////////////////////////////////////////////
// This macro allows you to process the content of the CorrelationContainer
// for both same event and mixed events and obtain the 2D correlation plot
// after mixed event subtraction with proper normalization.
///////////////////////////////////////////////////////////////////////////

void extract2D(const char* fileName = "AnalysisResults_O2.root", const char* outputFile = "dphi_corr.root", const char* folder = "task-hf-correlations", bool saveSameEventDis = false)
{
  gStyle->SetOptStat(1111111);

  CorrelationContainer::CFStep step = CorrelationContainer::kCFStepReconstructed;
  Bool_t normalizePerTrigger = kTRUE; // don't do this if histograms are to be merged with other periods later -> Use MergeDPhiFiles below

  auto file = TFile::Open(outputFile, "RECREATE");
  file->Close();

  // the interval below defines the pttrig, ptass and multiplicity (or
  // centrality in which the analysis will be performed. One "ridge" plot
  // with the correct normalization will be obtained for each of this
  // combination (with the condition that the selection on the associated pt
  // has to be looser than the one on the trigger particle.

  Int_t maxLeadingPt = 4;
  Float_t leadingPtArr[] = {0.5, 1.0, 2.0, 3.0};

  Int_t maxAssocPt = 4;
  Float_t assocPtArr[] = {0.5, 1.0, 2.0, 3.0};

  Int_t maxCentrality = 19;
  Float_t centralityArr[] = {0.0, 2.750, 5.250, 7.750, 12.750, 17.750, 22.750, 27.750, 32.750, 37.750, 42.750, 47.750, 52.750, 57.750, 62.750, 67.750, 72.750, 77.750, 82.750, 87.750, 92.750, 97.750, 250.1};

  // h is the container of the same event
  // hMixed contain the data corresponding to the mixed event sample.
  auto inputFile = TFile::Open(fileName);
  auto h = (CorrelationContainer*)inputFile->Get(Form("%s/sameEventTPCTPCChHadrons", folder));
  auto hMixed = (CorrelationContainer*)inputFile->Get(Form("%s/mixedEventTPCTPCChHadrons", folder));

  // the number of events for each multiplicity/centrality class is calculated and printed.
  auto eventHist = h->getEventCount();
  Printf("Events with centrality: %d", (int)eventHist->Integral(eventHist->GetXaxis()->FindBin(6.), eventHist->GetXaxis()->FindBin(6.), eventHist->GetYaxis()->FindBin(0.), eventHist->GetYaxis()->FindBin(99.9)));
  Printf("Events: %d", (int)eventHist->ProjectionX()->Integral(eventHist->GetXaxis()->FindBin(6.), eventHist->GetXaxis()->FindBin(6.)));
  //   eventHist->ProjectionX()->Draw();
  //   return;

  h->setPtRange(gpTMin, gpTMax);
  h->setCentralityRange(0, 49.9);
  h->setZVtxRange(-7.01, 6.99);

  TH2* sameTwoD = (TH2*)h->getPerTriggerYield(step, 0.2, 2.99);
  auto c1 = new TCanvas;
  sameTwoD->Draw("SURF1");
  c1->SaveAs("sameCorr.png");

  //  Note: this is not normalised for the bin (0,0)
  TH2* mixedTwoD = (TH2*)hMixed->getPerTriggerYield(step, 0.2, 2.99);
  auto c2 = new TCanvas;
  mixedTwoD->Draw("SURF1");
  c2->SaveAs("mixedCorr.png");

  auto ratio = (TH2*)sameTwoD->Clone("ratio");
  ratio->Divide(mixedTwoD);

  auto c3 = new TCanvas;
  ratio->GetYaxis()->SetRangeUser(-1.59, 1.59);
  ratio->Draw("SURF1");
  c3->SaveAs("pertriggeryield.png");

  //  TODO: when doing HF correlations, make sure to project/integrate the correct inv. mass range/axis

  for (Int_t i = 0; i < maxLeadingPt; i++) {
    for (Int_t j = 0; j < maxAssocPt; j++) {
      if (assocPtArr[j] >= leadingPtArr[i + 1])
        continue;

      // Below each analysis is made for each choice of pttrig, ptassoc and multiplicity.
      // The same event correlations is divided by mixed event after normalization 
      // for each pTtrig, pTassoc and mult bin (will be integrated over all Vz)

      gpTMin = assocPtArr[j] + 0.01;
      gpTMax = assocPtArr[j + 1] - 0.01;
      gZVtxMin = -7.01;
      gZVtxMax = 6.99;

      SetupRanges(h);
      SetupRanges(hMixed);

      for (Int_t mult = 0; mult < maxCentrality; mult++) {

        TH1* hist1 = 0;
        GetSumOfRatios(h, hMixed, &hist1, step, centralityArr[mult], centralityArr[mult + 1] - 1, leadingPtArr[i] + 0.01, leadingPtArr[i + 1] - 0.01, normalizePerTrigger);

        file = TFile::Open(outputFile, "UPDATE");

        if (hist1) {
          hist1->SetName(Form("dphi_%d_%d_%d", i, j, mult));
          hist1->Write();
        }

        file->Close();

        delete hist1;
      }
      
      //  Below, get same-event 2D distributions to compare with Jasper
      //  for each pT trig and pT passoc and each multiplicity bin
      //  in a particular 0 < Vz < 1 bin
      if (!saveSameEventDis)
        continue;

      gZVtxMin = -0.01;
      gZVtxMax = 0.99;

      SetupRanges(h);

      for (Int_t mult = 0; mult < maxCentrality; mult++) {

        TH2* histSame2D = 0;
        GetSameEventCorrelation(h, &histSame2D, step, centralityArr[mult], centralityArr[mult + 1] - 0.01, leadingPtArr[i] + 0.01, leadingPtArr[i + 1] - 0.01);

        file = TFile::Open(outputFile, "UPDATE");

        if(histSame2D) {
          histSame2D->SetName(Form("same_%d_%d_%d", i, j, mult));
          histSame2D->Write();
        }

        file->Close();

        delete histSame2D;

      }
    }

    // triggers contain the information about the number of trigger particles
    // in each interval of multiplicity (used for the result normalization)

    if (saveSameEventDis) {
      gZVtxMin = 0.01;
      gZVtxMax = 0.99;
    } else {
      gZVtxMin = -7.01;
      gZVtxMax = 6.99;
    }

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
