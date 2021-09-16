#if !defined(__CINT__) || defined(__MAKECINT__)
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TDatabasePDG.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TEfficiency.h"
#endif

using namespace std;

enum proc_t{kJpsiToEE, kJpsiToMuMu, kXToPiPiEE, kXToPiPiMuMu, kXicc, kBplus, kNChannels};

const char *hfTaskLabel[kNChannels] = {"jpsi",     "jpsiToMuMu",  "x",           "xToPiPiMuMu", "xicc",     "bplus"};
const char *histNameSig[kNChannels] = {"hmassSig", "hMassRecSig", "hMassRecSig", "hMassRecSig", "hmassSig", "hMassRecSig"};
const char *histNameBkg[kNChannels] = {"hmass",    "hMassRecBkg", "hMass",       "hMassRecBkg", "hmass",    "hMass"};

const char *label[kNChannels] = {
  "J/#psi #rightarrow ee",
  "J/#psi #rightarrow #mu#mu",
  "X #rightarrow J/#psi(ee) #pi#pi",
  "X #rightarrow J/#psi(#mu#mu) #pi#pi",
  "#Xi_{cc}^{++} #rightarrow #Xi_{c}^{+}#pi^{+}",
  "B^{+} #rightarrow D^{0}#pi^{+}"
};

const Double_t massMin[kNChannels]  = {2.60, 2.60, 3.60, 3.60, 3.30, 4.60};
const Double_t massMax[kNChannels]  = {3.55, 3.55, 4.10, 4.10, 3.90, 6.00};

const Double_t massMean[kNChannels] = {
  TDatabasePDG::Instance()->GetParticle(443)->Mass(),
  TDatabasePDG::Instance()->GetParticle(443)->Mass(),
  3.872,
  3.872,
  TDatabasePDG::Instance()->GetParticle(4412)->Mass(),
  TDatabasePDG::Instance()->GetParticle(521)->Mass()
};

const Double_t nsigma = 3;

Int_t nPtBins = 0;
const Int_t nMaxPtBins = 1000;
Double_t ptBinLimits[nMaxPtBins] = {0};

Double_t sideband[2] = {0};    // upper and lower limit of the signal window

// Control parameters for the description of the bkg with a polynomial function.
// The description starts with a pol2 model, and if the bkg fit has a larger chi2/ndf than the chi2OverNDF_limit parameter,
// the degree of the polynomial function describing it is increased by 1, with an upper limit given by maxPolDegree
const Double_t chi2OverNDF_limit = 3.; 
const Int_t maxPolDegree = 4;

TCanvas *cnvSig=0, *cnvBkg=0, *cnvBkgperEvents=0, *cnvEfficiency=0;

TH1D *hBkgPerEvent=0, *hEfficiency=0, *hEfficiencyNoPID=0, *hMassSig[nMaxPtBins]={0}, *hMassBkg[nMaxPtBins]={0};
TH2D *hMassVsPtSig=0, *hMassVsPtBkg=0;

TF1 *fitBkg[nMaxPtBins]={0}, *fitBkgSideBands[nMaxPtBins]={0}, *fitSig[nMaxPtBins]={0};

Double_t fitPol(Double_t* x_var, Double_t* par);
Double_t fitPolSideBands(Double_t* x_var, Double_t* par);

void info(proc_t channel);
void mystyle();

void BookCanvas();
void BookHistos();

//====================================================================================================================================================

void GetBkgPerEventAndEff(const char* signalfilename,
			  const char* bkgfilename,
			  const proc_t channel) {
  
  mystyle();

  Double_t bkg, errBkg;
  
  //-----------------------------------------------------------------------------------------------------------------------------

  // Conneting directories from input files
  
  TFile *input_sig = new TFile(signalfilename, "read");
  TFile *input_bkg = new TFile(bkgfilename,    "read");

  auto dir_sig   = (TDirectory*) input_sig->GetDirectory(Form("hf-task-%s-mc",hfTaskLabel[channel]));
  auto dir_bkg   = (TDirectory*) input_bkg->GetDirectory(Form("hf-task-%s",hfTaskLabel[channel]));

  hMassVsPtSig = (TH2D*) dir_sig->Get(histNameSig[channel]);
  hMassVsPtSig -> SetName("hMassVsPtSig");

  hMassVsPtBkg = (TH2D*) dir_bkg->Get(histNameBkg[channel]);
  hMassVsPtBkg -> SetName("hMassVsPtBkg");

  Double_t nEventsBkg = -1;
  TH1F *hCount = (TH1F*) input_bkg->Get("qa-global-observables/eventCount");
  if (!hCount) {
    nEventsBkg = 20e6;
    printf("\n********* WARNING: cannot retrieve bkg number of events, using nEventsBkg = %d *********\n\n",Int_t(nEventsBkg));
  }
  else {
    nEventsBkg = hCount->GetBinContent(1);
    printf("nEventsBkg = %d, read from qa-global-observables/eventCount\n",Int_t(nEventsBkg));
  }
  
  // check of consistency for hMassVsPtSig vs hMassVsPtBkg (same pt binning)
  TH1D *hTmpSig=hMassVsPtSig->ProjectionY();
  TH1D *hTmpBkg=hMassVsPtBkg->ProjectionY();
  if (!(hTmpSig->Add(hTmpBkg))) {
    printf("ERROR: sig and bkg histograms have different pt binning, quitting.\n");
    return;
  }

  nPtBins = TMath::Min(hMassVsPtBkg->GetNbinsY(), nMaxPtBins);
  BookCanvas();

  for (int i = 0; i<nPtBins; i++) {
    ptBinLimits[i]   = hMassVsPtSig->GetYaxis()->GetBinLowEdge(i+1);
    ptBinLimits[i+1] = hMassVsPtSig->GetYaxis()->GetBinLowEdge(i+1) + hMassVsPtSig->GetYaxis()->GetBinWidth(i+1);
  }

  BookHistos();

  auto hPtGenSig  = (TH1F*) dir_sig->Get("hPtGen");
  auto hPtRecSig  = (TH1F*) dir_sig->Get("hPtRecSig");
  
  auto gp = (TH1D*) hPtGenSig->Rebin(nPtBins,"gp", ptBinLimits);
  auto rp = (TH1D*) hPtRecSig->Rebin(nPtBins,"eff",ptBinLimits);
  
  gp -> Sumw2();
  rp -> Sumw2();
  rp -> Divide(gp);
  
  hEfficiency = (TH1D*) rp -> Clone();
  hEfficiency -> SetTitle(";p_{T} (GeV/c); Reconstruction Efficiency");
  hEfficiency -> SetLineColor(kRed);
  hEfficiency -> SetLineWidth(2);
  hEfficiency -> GetYaxis() -> CenterTitle();
  
  cnvEfficiency -> cd();
  hEfficiency->Draw("e");
  
  info(channel);

  //-----------------------------------------------------------------------------------------------------------------------------

  for (int i = 0; i < nPtBins; i++) {

    Int_t ptBin = i+1;

    // Projecting sig and bkg histos form TH2D objects
    
    hMassSig[i] = hMassVsPtSig->ProjectionX(Form("hMassSig_PtBin_%d", ptBin), ptBin, ptBin, "e");
    hMassBkg[i] = hMassVsPtBkg->ProjectionX(Form("hMassBkg_PtBin_%d", ptBin), ptBin, ptBin, "e");

    if (hMassSig[i]->GetMaximum() < 20) hMassSig[i]->Rebin(2);
    
    hMassSig[i]->GetXaxis()->SetRangeUser(massMin[channel], massMax[channel]);
    hMassBkg[i]->GetXaxis()->SetRangeUser(massMin[channel], massMax[channel]);

    hMassSig[i]->SetTitle(Form("%2.1f < p_{T} < %2.1f",ptBinLimits[i],ptBinLimits[i+1]));
    hMassBkg[i]->SetTitle(Form("%2.1f < p_{T} < %2.1f",ptBinLimits[i],ptBinLimits[i+1]));
   
   // Setting the fit functions for bkg and sig
    
    fitBkg[i]          = new TF1(Form("fitBkg_%d",i),          fitPol,          massMin[channel],massMax[channel],maxPolDegree+1);
    fitBkgSideBands[i] = new TF1(Form("fitBkgSideBands_%d",i), fitPolSideBands, massMin[channel],massMax[channel],maxPolDegree+1);
    fitSig[i]          = new TF1(Form("fitSig_%d",i),"gaus",massMean[channel]-5*hMassSig[i]->GetRMS(),massMean[channel]+5*hMassSig[i]->GetRMS());
		     
    fitBkg[i] -> SetNpx(10000);
    fitSig[i] -> SetNpx(10000);

    // Gaussian fit on the signal

    cnvSig -> cd(i+1);

    hMassSig[i] -> Fit(fitSig[i],"Q","",massMean[channel]-5*hMassSig[i]->GetRMS(),massMean[channel]+5*hMassSig[i]->GetRMS());
    Double_t sigmaSig = fitSig[i]->GetParameter(2);

    sideband[0] = massMean[channel] - nsigma*sigmaSig;
    sideband[1] = massMean[channel] + nsigma*sigmaSig;

    // Fit of the bakground 

    cnvBkg -> cd(i+1);

    // we start with a 2nd order polynomial
    Int_t nPolDegree = 2;
    for (Int_t j=nPolDegree+1; j<=maxPolDegree; j++) fitBkgSideBands[i] -> FixParameter(j,0);

    hMassBkg[i] -> Fit(fitBkgSideBands[i],"Q","",massMin[channel],massMax[channel]);

    while (fitBkgSideBands[i]->GetChisquare()/fitBkgSideBands[i]->GetNDF() > chi2OverNDF_limit && nPolDegree < maxPolDegree) {
      nPolDegree++;
      fitBkgSideBands[i] -> ReleaseParameter(nPolDegree);
      hMassBkg[i] -> Fit(fitBkgSideBands[i],"Q","",massMin[channel],massMax[channel]);
    }

    for (Int_t j=0; j<=maxPolDegree; j++) fitBkg[i] -> SetParameter(j, fitBkgSideBands[i] -> GetParameter(j));
      
    bkg = fitBkg[i] -> Integral(sideband[0],sideband[1])/hMassBkg[i]->GetBinWidth(1);
    bkg /= nEventsBkg;   // bkg is the expected background in the +/- 3 sigma window per MB event

    // Evaluating significance and filling histos
       
    hBkgPerEvent -> SetBinContent(i+1, bkg);
    hBkgPerEvent -> SetBinError(i+1, 0.);

  }

  TFile *fileOutEff = new TFile(Form("efficiency_%s.root",hfTaskLabel[channel]),"recreate");
  hEfficiency->Write();
  fileOutEff->Close();
 
  TFile *fileOutBkgPerEvents = new TFile(Form("bkgPerEvents_%s.root",hfTaskLabel[channel]),"recreate");
  cnvBkgperEvents -> cd();
  cnvBkgperEvents -> SetLogy();
  hBkgPerEvent -> Draw("e ][");
  info(channel);
  hBkgPerEvent->Write();
  fileOutBkgPerEvents->Close();

}

//====================================================================================================================================================

Double_t fitPol(Double_t* x_var, Double_t* par) {

  Double_t result = par[0];
  for (Int_t i=1; i<=maxPolDegree; i++) result += par[i] * TMath::Power(x_var[0], i);

  return result;
  
}

//====================================================================================================================================================

Double_t fitPolSideBands(Double_t* x_var, Double_t* par) {

  if (sideband[0] < x_var[0] && x_var[0] < sideband[1]) {
    TF1::RejectPoint();
    return 0;
  }
  
  return fitPol(x_var, par);
    
}

//====================================================================================================================================================

void info(proc_t channel) {

  TLatex* t = new TLatex(8, 8, "ALICE3 O2 Performance");
  t->SetNDC();
  t->SetTextSize(0.5);
  t->SetTextAlign(10);
  t->SetTextColor(1);
  t->SetTextSize(0.03);
  t->DrawLatex(0.5, 0.85, "ALICE3 O2 Performance");

  t->SetTextSize(0.025);
  t->SetTextAlign(12);
  t->DrawLatex(0.5, 0.8, "PYTHIA 8 pp  #sqrt{s} = 14TeV ");
  //  t->DrawLatex(0.5, 0.75, "Inclusive J/#psi #rightarrow ee ");
  t->DrawLatex(0.5, 0.75, label[channel]);
  t->DrawLatex(0.5, 0.7, "kinematic cuts");
}

//====================================================================================================================================================

void BookCanvas() {

  cnvSig = new TCanvas("cnvSig","Signal fit",2000,800);
  int nColums = 5;
  int nRows = (nPtBins-1)/nColums + 1;
  cnvSig -> Divide(nColums,nRows);

  cnvBkg = new TCanvas("cnvBkg","Bkg fit",2000,800);
  cnvBkg -> Divide(nColums,nRows);

  cnvBkgperEvents = new TCanvas("BkgperEvents","Bkg/nEvents");  
  cnvEfficiency   = new TCanvas("cnvEfficiency","Efficiency",800,600);
  
}

//====================================================================================================================================================

void BookHistos() {

  hBkgPerEvent  = new TH1D("hBkgPerEvent",  ";p_{T}(J/#psi)(GeV/c);Bkg/nEvents",                nPtBins, ptBinLimits);
  hEfficiency    = new TH1D("hEfficiency",  ";p_{T}(J/#psi)(GeV/c); Reconstruction Efficiency", nPtBins, ptBinLimits);
  
  hBkgPerEvent -> SetLineColor(kRed);
  hBkgPerEvent -> SetLineWidth(3);

  hEfficiency   -> SetLineColor(kRed);
  hEfficiency   -> SetLineWidth(3);
  
  hBkgPerEvent -> GetYaxis() -> CenterTitle();

}

//====================================================================================================================================================

void mystyle() {

  gROOT->ForceStyle();
  gStyle->SetOptStat(0);
  gStyle->SetFrameLineWidth(2);
  gStyle->SetTitleSize(0.045, "x");
  gStyle->SetTitleSize(0.045, "y");
  gStyle->SetMarkerSize(1);
  gStyle->SetLineWidth(2);
  gStyle->SetLabelOffset(0.015, "x");
  gStyle->SetLabelOffset(0.01, "y");
  gStyle->SetTitleOffset(1, "x");
  gStyle->SetTitleOffset(0.8, "y");
  gStyle->SetTextSize(0.03);
  gStyle->SetTextAlign(5);
  gStyle->SetTextColor(1);

}

//====================================================================================================================================================
