#include <TH1F.h>
#include <TFile.h>
#include <TROOT.h>
#include "TCanvas.h"
#include "TLegend.h"
#include <iostream>
#include "AliESDEventMFT.h"
#include "AliESDtrack.h"
#include <TTree.h>
#include "TStyle.h"

Bool_t ComputeVerticesRun1(){

  TString esdfile = "../inputESD/AliESDs_20200201_v0.root";
  TFile* esdFile = TFile::Open(esdfile.Data());

  if (!esdFile || !esdFile->IsOpen()) {
    printf("Error in opening ESD file");
    return kFALSE;
  }
  
  AliESDEvent * esd = new AliESDEvent;

  TTree* tree = (TTree*) esdFile->Get("esdTree");
  if (!tree) {
    printf("Error: no ESD tree found");
    return kFALSE;
  }

  esd->ReadFromTree(tree);
  
  // file 1
  TH1F* hpt_nocuts =new TH1F("hpt_nocuts"," ; pt tracks (#GeV) ; Entries",100, 0, 10.);
  TH1F* hrun_number = new TH1F("hrun_number", "run number", 1000, 0., 1000000.);
  TH1F* hfCYY = new TH1F("hfCYY", "cYY", 1000, 0., 150.);
  TH1F* hfCZY = new TH1F("hfCZY", "cZY", 1000, -40., 10.);
  TH1F* hfCZZ = new TH1F("hfCZZ", "cZZ", 1000, 0., 150.);
  TH1F* hfCSnpY = new TH1F("hfCSnpY", "cSnpY", 1000, -2.5, 1.);
  TH1F* hfCSnpZ = new TH1F("hfCSnpZ", "cSnpZ", 1000, -2.5, 1.);
  TH1F* hfCSnpSnp = new TH1F("hfCSnpSnp", "cSnpSnp", 1000, 0., 0.1);
  TH1F* hfCTglY = new TH1F("hfCTglY", "cTglY", 1000, -0.1, 0.3);

  // file 2
  TH1F* hfCTglZ = new TH1F("hfCTglZ", "cTglZ", 1000, -3., 3.);
  TH1F* hfCTglSnp = new TH1F("hfCTglSnp", "cTglSnp", 1000, -0.01, 0.01);
  TH1F* hfCTglTgl = new TH1F("hfCTglTgl", "cTglTgl", 1000, 0., 0.2);
  TH1F* hfC1PtY = new TH1F("hfC1PtY", "c1PtY", 1000, 0., 30.);
  TH1F* hfC1PtZ = new TH1F("hfC1PtZ", "c1PtZ", 1000, -9., 3.);
  TH1F* hfC1PtSnp = new TH1F("hfC1PtSnp", "c1PtSnp", 1000, -0.8, 1.);
  TH1F* hfC1PtTgl = new TH1F("hfC1PtTgl", "c1PtTgl", 1000, -0.3, 0.1);
  TH1F* hfC1Pt21Pt2 = new TH1F("hfC1Pt21Pt2", "c1Pt21Pt2", 1000, -0.3, 0.1);

  TObjArray *trkArray    = new TObjArray(20);

  // loop over events
  for (Int_t iEvent = 0; iEvent < tree->GetEntries(); iEvent++) {
    tree->GetEvent(iEvent);

    if (!esd) {
      printf("Error: no ESD object found for event %d", iEvent);
      return kFALSE;
    }

    std::cout<<"-------- Event "<<iEvent<<std::endl;
    printf(" Tracks # = %d\n",esd->GetNumberOfTracks());
    Double_t fBzkG = (Double_t)esd->GetMagneticField();

    Int_t it=0;

    // loop over tracks
    for (Int_t iTrack = 0; iTrack < esd->GetNumberOfTracks(); iTrack++) {
      AliESDtrack* track = esd->GetTrack(iTrack);
      Int_t status=track->GetStatus();

      float_t p[2];
      float_t cov[3];
      track->GetImpactParameters(p, cov);

      hpt_nocuts->Fill(track->Pt());
      hfCYY->Fill(cov[0]);
      hfCZY->Fill(cov[1]);
      hfCZZ->Fill(cov[2]);
      hfCSnpY->Fill(track->GetSigmaSnpY());
      hfCSnpZ->Fill(track->GetSigmaSnpZ());
      hfCSnpSnp->Fill(track->GetSigmaSnp2());
      hfCTglY->Fill(track->GetSigmaTglY());
      hfCTglZ->Fill(track->GetSigmaTglZ());
      hfCTglSnp->Fill(track->GetSigmaTglSnp());
      hfCTglTgl->Fill(track->GetSigmaTgl2());
      hfC1PtY->Fill(track->GetSigma1PtY());
      hfC1PtZ->Fill(track->GetSigma1PtZ());
      hfC1PtSnp->Fill(track->GetSigma1PtSnp());
      hfC1PtTgl->Fill(track->GetSigma1PtTgl());
      hfC1Pt21Pt2->Fill(track->GetSigma1Pt2());
      

      if (status & AliESDtrack::kITSrefit && (track->HasPointOnITSLayer(0) || track->HasPointOnITSLayer(1)) && track->GetNcls(1)>70) {
        trkArray->AddAt(track,it++);
      }
    } // tracks

    hrun_number->Fill(esd->GetRunNumber());

    printf(" Tracks ITSrefit = %d\n",it);
    printf(" Entries in array = %d\n",trkArray->GetEntries());
    trkArray->Clear();
  } // events

  delete trkArray;
 
  TFile* fout=new TFile("QA-ITS1.root","recreate");

  hpt_nocuts->Write();
  hrun_number->Write();
  hfCYY->Write();
  hfCZY->Write();
  hfCZZ->Write();
  hfCSnpY->Write();
  hfCSnpZ->Write();
  hfCSnpSnp->Write();
  hfCTglY->Write();
  hfCTglZ->Write();
  hfCTglSnp->Write();
  hfCTglTgl->Write();
  hfC1PtY->Write();
  hfC1PtZ->Write();
  hfC1PtSnp->Write();
  hfC1PtTgl->Write();
  hfC1Pt21Pt2->Write();

  fout->Close();
  return true; 
}



