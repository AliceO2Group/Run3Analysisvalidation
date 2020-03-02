#include "AliAnalysisManager.h"
#include "AliESDEvent.h"
#include "AliESDtrack.h"
#include "AliGenEventHeader.h"
#include "AliHeader.h"
#include "AliInputEventHandler.h"
#include "AliPID.h"
#include "AliPIDResponse.h"
#include "AliTOFGeometry.h"
#include "AliTOFPIDResponse.h"
// #include "TAlienCollection.h"
#include "TFile.h"
#include "TGrid.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLeaf.h"
#include "TList.h"
#include "TMath.h"
#include "TRandom.h"
#include "TTree.h"

float beta(float l, float t, float t0) { return l / (t - t0) / 0.029979246; }

float betaerror(float l, float t, float t0, float sigmat = 80) {
  return beta(l, t, t0) / (t - t0) * sigmat;
}

float expbeta(float p, float m) {
  if (p > 0)
    return p / TMath::Sqrt(p * p + m * m);
  return 0;
}

float p(float eta, float signed1Pt) { return cosh(eta) / fabs(signed1Pt); }

const float kElectronMass = 5.10998909999999971e-04;
const float kPionMass = 1.39569997787475586e-01f;
const float kKaonMass = 4.93676990270614624e-01;
const float kProtonMass = 9.38271999359130859e-01f;

Bool_t
ComputePidSpectra(TString esdfile = "../inputESD/AliESDs_20200201_v0.root",
                  TString output = "PidSpectra.root", bool applyeventcut = 0) {
  TFile *esdFile = TFile::Open(esdfile.Data());
  if (!esdFile || !esdFile->IsOpen()) {
    printf("Error in opening ESD file");
    return kFALSE;
  }

  AliESDEvent *esd = new AliESDEvent;
  TTree *tree = (TTree *)esdFile->Get("esdTree");
  if (!tree) {
    printf("Error: no ESD tree found");
    return kFALSE;
  }
  esd->ReadFromTree(tree);

  TList *lh = new TList();
  lh->SetOwner();

  const TString pt = "#it{p}_{T} (GeV/#it{c})";
  const TString mom = "#it{p} (GeV/#it{c})";
  TH1F *hp = new TH1F("hp", "pt;" + mom + ";Tracks", 100, 0, 20);
  lh->Add(hp);
  TH1F *hptel = new TH1F("hptel", "pt;" + pt + ";Tracks", 100, 0, 20);
  lh->Add(hptel);
  TH2F *hbeta = new TH2F("hbeta", "beta;" + pt + ";TOF #beta;Tracks", 100, 0,
                         20, 100, 0, 2);
  lh->Add(hbeta);
  TH2F *hbetael =
      new TH2F("hbetael", "beta diff;" + pt + ";#beta - #beta_{el};Tracks", 100,
               0, 20, 100, -0.01, 0.01);
  lh->Add(hbetael);
  TH2F *hbetaelsigma =
      new TH2F("hbetaelsigma",
               "beta N#sigma;" + pt + ";(#beta - #beta_{el})/#sigma;Tracks",
               100, 0, 20, 100, -5, 5);
  lh->Add(hbetaelsigma);

  // AliAnalysisManager::GetAnalysisManager()->Print();
  // (AliPIDResponse
  //      *)((AliInputEventHandler *)(AliAnalysisManager::GetAnalysisManager()
  //                                      ->GetInputEventHandler()))
  //     ->GetPIDResponse();

  AliPIDResponse *pidr = new AliPIDResponse(kFALSE);
  for (Int_t iEvent = 0; iEvent < tree->GetEntries(); iEvent++) {
    tree->GetEvent(iEvent);
    if (!esd) {
      printf("Error: no ESD object found for event %d", iEvent);
      return kFALSE;
    }
    esd->ConnectTracks(); // Deve essere sempre chiamato dopo aver letto
                          // l'evento (non troverebbe l'ESDevent). Scrivo in
                          // tutte le tracce l origine dell evento così poi da
                          // arrivare ovunque(tipo al cluster e al tempo
                          // quindi).
    Printf("Event %i has %i tracks", iEvent, esd->GetNumberOfTracks());
    if (!esd->AreTracksConnected() && esd->GetNumberOfTracks() > 0)
      Printf("!!!Tracks are not connected, %i tracks are affected !!!!",
             esd->GetNumberOfTracks());
    // pidr->SetTOFResponse(esd, AliPIDResponse::kBest_T0);
    // AliTOFPIDResponse &TOFResponse = pidr->GetTOFResponse();

    AliESDVertex *primvtx = (AliESDVertex *)esd->GetPrimaryVertex();
    if (applyeventcut == 1) {
      Printf("Applying event selection");
      // if (!primvtx)
        // return kFALSE;
      // TString title = primvtx->GetTitle();
      if (primvtx->IsFromVertexer3D() || primvtx->IsFromVertexerZ())
        continue;
      if (primvtx->GetNContributors() < 2)
        continue;
    }

    Float_t eventTime[10];
    Float_t eventTimeRes[10];
    Double_t eventTimeWeight[10];

    for (Int_t i = 0; i < pidr->GetTOFResponse().GetNmomBins(); i++) {
      Float_t mom = (pidr->GetTOFResponse().GetMinMom(i) +
                     pidr->GetTOFResponse().GetMaxMom(i)) /
                    2.f;
      eventTime[i] = pidr->GetTOFResponse().GetStartTime(mom);
      eventTimeRes[i] = pidr->GetTOFResponse().GetStartTimeRes(mom);
      eventTimeWeight[i] = 1. / (eventTimeRes[i] * eventTimeRes[i]);
    }

    // Recalculate unique event time and its resolution
    float fEventTime = TMath::Mean(
        10, eventTime,
        eventTimeWeight); // Weighted mean of times per momentum interval
    float fEventTimeRes = TMath::Sqrt(9. / 10.) *
                          TMath::Mean(10, eventTimeRes); // PH bad approximation

    for (Int_t itrk = 0; itrk < esd->GetNumberOfTracks(); itrk++) {
      AliESDtrack *trk = esd->GetTrack(itrk);
      // trk->SetESDEvent(esd);
      Int_t status = trk->GetStatus();
      float Mom = p(trk->Eta(), trk->GetSigned1Pt());
      hp->Fill(Mom);
      bool sel = status & AliESDtrack::kITSrefit &&
                 (trk->HasPointOnITSLayer(0) || trk->HasPointOnITSLayer(1)) &&
                 trk->GetNcls(1) > 70;
      if (!sel)
        continue;
      sel = sel && (status & AliESDtrack::kTOFout) &&
            (status & AliESDtrack::kTIME);
      if (!sel)
        continue;
      if (!trk->GetESDEvent())
        Printf("For track %i I cannot get ESD event from track!!!", itrk);
      // float StartTime = pidr->GetTOFResponse().GetStartTime(Mom);
      float StartTime = fEventTimeRes;
      if (trk->GetTOFsignal() <= 0)
        continue;

      float Beta =
          beta(trk->GetIntegratedLength(), trk->GetTOFsignal(), StartTime);
      float betadiff = Beta - expbeta(Mom, kElectronMass);
      float betasigma =
          betaerror(trk->GetIntegratedLength(), trk->GetTOFsignal(), StartTime);
      if (abs(betadiff / betasigma) > 1)
        continue;

      hptel->Fill(trk->Pt());
      hbeta->Fill(trk->Pt(), Beta);
      hbetael->Fill(trk->Pt(), betadiff);
      hbetaelsigma->Fill(trk->Pt(), betadiff / betasigma);
    }
    esd->ResetStdContent();
  }

  TFile *fout = new TFile(output.Data(), "recreate");
  fout->mkdir("filterEl-task");
  fout->cd("filterEl-task");
  lh->Write();
  fout->Close();
  return true;
}