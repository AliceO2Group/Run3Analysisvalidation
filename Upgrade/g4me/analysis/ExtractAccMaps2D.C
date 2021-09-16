#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"

enum part_t{kMIDElectron, kMIDMuon, kMIDPion, kMIDKaon, kMIDProton, kNPartTypes};
const char* partName[kNPartTypes] = {"electron", "muon", "pion", "kaon", "proton"};

enum {kGoodMatch, kFakeMatch, kAllMatch};
const char *tagMatch[3] = {"GoodMatch", "FakeMatch", "allMatch"};

TH2D *accEffMuonPID_2D[kNPartTypes]={0}, *gen[kNPartTypes]={0}, *recWithMuonPID[kNPartTypes][3]={{0}}, *fakeMatchRate[kNPartTypes]={0};

TH3D *hChi2VsMomVsEtaMatchedTracks[kNPartTypes][2]={{0}};
TH2D *hMomVsEtaITSTracks[kNPartTypes]={0};

TFile *fileIn = 0;

//====================================================================================================================================================

void ExtractAccMaps2D(const char* inputFileName = "histosTracking.root", double chi2Max=1.5) {

  fileIn = new TFile(inputFileName);

  for (int iPart=0; iPart<kNPartTypes; iPart++) {
    
    gen[iPart] = (TH2D*) fileIn->Get(Form("hMomVsEtaITSTracks_%s",partName[iPart]));

    for (int iMatch=0; iMatch<2; iMatch++) {
      hChi2VsMomVsEtaMatchedTracks[iPart][iMatch] = (TH3D*) fileIn->Get(Form("hChi2VsMomVsEtaMatchedTracks_%s_%s",partName[iPart],tagMatch[iMatch]));
      hChi2VsMomVsEtaMatchedTracks[iPart][iMatch] -> GetXaxis() -> SetRangeUser(0, chi2Max);
      recWithMuonPID[iPart][iMatch] = (TH2D*) hChi2VsMomVsEtaMatchedTracks[iPart][iMatch]->Project3D("zy");
      recWithMuonPID[iPart][iMatch] -> SetName(Form("recWithMuonPID_%s_%s",partName[iPart],tagMatch[iMatch]));
    }
    
    recWithMuonPID[iPart][kAllMatch] = (TH2D*) recWithMuonPID[iPart][kGoodMatch]->Clone(Form("recWithMuonPID_%s_%s",partName[iPart],tagMatch[kAllMatch]));
    recWithMuonPID[iPart][kAllMatch] -> Add(recWithMuonPID[iPart][kFakeMatch]);
    fakeMatchRate[iPart] = (TH2D*) recWithMuonPID[iPart][kFakeMatch]->Clone(Form("fakeMatchRate_%s",partName[iPart]));
    fakeMatchRate[iPart] -> Divide(recWithMuonPID[iPart][kAllMatch]);
    fakeMatchRate[iPart] -> SetTitle(Form("Fake match probability for for %ss",partName[iPart]));

    accEffMuonPID_2D[iPart] = (TH2D*) recWithMuonPID[iPart][kAllMatch]->Clone(Form("accEffMuonPID_2D_%s",partName[iPart]));
    accEffMuonPID_2D[iPart] -> Divide(gen[iPart]);
    accEffMuonPID_2D[iPart] -> SetTitle(Form("Acc #times Eff #times #muPID for %ss",partName[iPart]));
    
  }

  TFile *fileOut = new TFile(Form("accEffMuonPID.MaxChi2_%3.1f.root",chi2Max),"recreate");
  
  for (int iPart=0; iPart<kNPartTypes; iPart++) {
    
    gen[iPart]              -> Write();
    fakeMatchRate[iPart]    -> Write();
    accEffMuonPID_2D[iPart] -> Write();

    for (int iMatch=0; iMatch<3; iMatch++) recWithMuonPID[iPart][iMatch] -> Write();
    
  }

  fileOut -> Close();

}
