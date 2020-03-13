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

#include "tofhelper.h"

Bool_t ComputePidSpectra(TString esdfile = "../inputESD/AliESDs_20200201_v0.root",
                         TString output = "PidSpectra.root",
                         bool applyeventcut = 0)
{
  TFile* esdFile = TFile::Open(esdfile.Data());
  if (!esdFile || !esdFile->IsOpen()) {
    printf("Error in opening ESD file");
    return kFALSE;
  }
  Printf("Computing Pid Spectra");
  AliESDEvent* esd = new AliESDEvent;
  TTree* tree = (TTree*)esdFile->Get("esdTree");
  if (!tree) {
    printf("Error: no ESD tree found");
    return kFALSE;
  }
  Printf("Reading TTree with %lli events", tree->GetEntries());
  esd->ReadFromTree(tree);

  TList* lh = new TList();
  lh->SetOwner();

#define DOTH1F(OBJ, ...)                   \
  TH1F* OBJ = new TH1F(#OBJ, __VA_ARGS__); \
  lh->Add(OBJ);
#define DOTH2F(OBJ, ...)                   \
  TH2F* OBJ = new TH2F(#OBJ, __VA_ARGS__); \
  lh->Add(OBJ);

  // Standard pT distributions
  DOTH1F(hp_NoCut, ";#it{p} (GeV/#it{c});Tracks", 100, 0, 20);
  DOTH1F(hp_TrkCut, ";#it{p} (GeV/#it{c});Tracks", 100, 0, 20);
  DOTH1F(hp_TOFCut, ";#it{p} (GeV/#it{c});Tracks", 100, 0, 20);
  // Distributions with PID
  DOTH1F(hp_El, ";#it{p} (GeV/#it{c});Tracks", 100, 0, 20);
  DOTH1F(hpt_El, ";#it{p}_{T} (GeV/#it{c});Tracks", 100, 0, 20);
  DOTH2F(hp_beta, ";#it{p} (GeV/#it{c});TOF #beta;Tracks", 100, 0, 20, 100, 0, 2);
  DOTH2F(hp_beta_El, ";#it{p} (GeV/#it{c});#beta - #beta_{el};Tracks", 100, 0, 20, 100, -0.01, 0.01);
  DOTH2F(hp_betasigma_El, ";#it{p} (GeV/#it{c});(#beta - #beta_{el})/#sigma;Tracks", 100, 0, 20, 100, -5, 5);
  //
  AliPIDResponse* pidr = new AliPIDResponse(kTRUE);
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
      Printf("!!!Tracks are not connected, %i tracks are affected !!!!", esd->GetNumberOfTracks());
    pidr->SetTOFResponse(esd, AliPIDResponse::kBest_T0);

    AliESDVertex* primvtx = (AliESDVertex*)esd->GetPrimaryVertex();
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
      Float_t mom = (pidr->GetTOFResponse().GetMinMom(i) + pidr->GetTOFResponse().GetMaxMom(i)) / 2.f;
      Printf("Mom [%.2f, %.2f] = %.2f", pidr->GetTOFResponse().GetMinMom(i), pidr->GetTOFResponse().GetMaxMom(i), mom);
      eventTime[i] = pidr->GetTOFResponse().GetStartTime(mom);
      Printf("T0=%f", eventTime[i]);
      eventTimeRes[i] = pidr->GetTOFResponse().GetStartTimeRes(mom);
      eventTimeWeight[i] = 1. / (eventTimeRes[i] * eventTimeRes[i]);
    }

    // Recalculate unique event time and its resolution
    float fEventTime = TMath::Mean(10, eventTime, eventTimeWeight); // Weighted mean of times per momentum interval

    float fEventTimeRes = TMath::Sqrt(9. / 10.) * TMath::Mean(10, eventTimeRes); // PH bad approximation

    for (Int_t itrk = 0; itrk < esd->GetNumberOfTracks(); itrk++) {
      AliESDtrack* trk = esd->GetTrack(itrk);
      // trk->SetESDEvent(esd);
      Int_t status = trk->GetStatus();
      float Mom = p(trk->Eta(), trk->GetSigned1Pt());
      hp_NoCut->Fill(Mom);
      bool sel = status & AliESDtrack::kITSrefit &&
                 (trk->HasPointOnITSLayer(0) || trk->HasPointOnITSLayer(1)) &&
                 trk->GetNcls(1) > 70;
      if (!sel)
        continue;
      hp_TrkCut->Fill(Mom);
      sel = sel && (status & AliESDtrack::kTOFout) && (status & AliESDtrack::kTIME);
      if (!sel)
        continue;
      hp_TOFCut->Fill(Mom);
      if (!trk->GetESDEvent())
        Printf("For track %i I cannot get ESD event from track!!!", itrk);
      // float StartTime = pidr->GetTOFResponse().GetStartTime(Mom);
      float StartTime = fEventTime;

      float Beta = beta(trk->GetIntegratedLength(), trk->GetTOFsignal(), StartTime);
      float betadiff = Beta - expbeta(Mom, kElectronMass);
      float betasigma = betaerror(trk->GetIntegratedLength(), trk->GetTOFsignal(), StartTime);
      if (abs(betadiff / betasigma) > 1)
        continue;

      hp_El->Fill(trk->P());
      hpt_El->Fill(trk->Pt());
      hp_beta->Fill(trk->Pt(), Beta);
      hp_beta_El->Fill(trk->Pt(), betadiff);
      hp_betasigma_El->Fill(trk->Pt(), betadiff / betasigma);
    }
    esd->ResetStdContent();
  }

  TFile* fout = new TFile(output.Data(), "recreate");
  fout->mkdir("filterEl-task");
  fout->cd("filterEl-task");
  lh->Write();
  fout->Close();
  return true;
}