#include "MIDTrackletSelector.h"
#include "THnSparse.h"
#include "TH3.h"
#include "TH2.h"
#include "TFile.h"
#include "TVector3.h"
#include "TMath.h"

MIDTrackletSelector::MIDTrackletSelector() {

  mInputFile       = NULL;
  mTrackletAcc3D   = NULL;
  mTrackletAcc2D   = NULL;

  for (int iCharge=0; iCharge<kNChargeOptions; iCharge++) mTrackletAcc4D[iCharge] = NULL;

  mIsSelectorSetup = kFALSE;

}

//==========================================================================================================

bool MIDTrackletSelector::Setup(const Char_t *nameInputFile = "muonTrackletAcceptance.root") {
  
  mInputFile = new TFile(nameInputFile);
  if (!mInputFile) {
    printf("File %s not found\n",nameInputFile);
    return kFALSE;
  }
  if (!(mInputFile->IsOpen())) {
    printf("File %s not open\n",nameInputFile);
    return kFALSE;
  }
  
  mTrackletAcc4D[kMuonMinus] = (THnSparse*) mInputFile->Get("trackletAcceptanceMuMinus");
  if (!mTrackletAcc4D[kMuonMinus]) {
    printf("Object <trackletAcceptanceMuMinus> not found in file %s, quitting\n",mInputFile->GetName());
    return kFALSE;
  }

  mTrackletAcc4D[kMuonPlus] = (THnSparse*) mInputFile->Get("trackletAcceptanceMuPlus");
  if (!mTrackletAcc4D[kMuonPlus]) {
    printf("Object <trackletAcceptanceMuPlus> not found in file %s, quitting\n",mInputFile->GetName());
    return kFALSE;
  }

  mTrackletAcc4D[kAllMuons] = (THnSparse*) mTrackletAcc4D[kMuonMinus]->Clone("trackletAcceptanceAllMuons");
  mTrackletAcc4D[kAllMuons] -> Add(mTrackletAcc4D[kMuonPlus]);
  
  mTrackletAcc3D = (TH3C*) mTrackletAcc4D[kAllMuons]->Projection(0,1,2);
  mTrackletAcc2D = (TH2C*) mTrackletAcc4D[kAllMuons]->Projection(1,0);

  mEtaMax = mTrackletAcc4D[kAllMuons]->GetAxis(2)->GetXmax();
  mMomMax = mTrackletAcc4D[kAllMuons]->GetAxis(3)->GetBinCenter(mTrackletAcc4D[kAllMuons]->GetAxis(3)->GetNbins());
  mMomMin = mTrackletAcc4D[kAllMuons]->GetAxis(3)->GetBinCenter(1);

  mIsSelectorSetup = kTRUE;
  
  printf("Setup of MIDTrackletSelector successfully completed\n");
  return kTRUE;
  
}

//====================================================================================================================================================

bool MIDTrackletSelector::IsMIDTrackletSelected(TVector3 posHitLayer1, TVector3 posHitLayer2, bool evalEta=kFALSE) {

  if (!mIsSelectorSetup) {
    printf("ERROR: MIDTrackletSelector not initialized\n");
    return kFALSE;
  }

  if (posHitLayer1.Perp() > posHitLayer2.Perp()) {
    TVector3 tmp = posHitLayer1;
    posHitLayer1 = posHitLayer2;
    posHitLayer2 = tmp;
  }

  
  double deltaPhi = posHitLayer2.DeltaPhi(posHitLayer1);
  double deltaEta = posHitLayer2.Eta() - posHitLayer1.Eta();
  
  if (evalEta) {
    double eta = posHitLayer1.Eta();
    if (abs(eta) > mEtaMax) return kFALSE;
    return mTrackletAcc3D->GetBinContent(mTrackletAcc3D->FindBin(deltaEta,deltaPhi,eta));
  }

  else return mTrackletAcc2D->GetBinContent(mTrackletAcc2D->FindBin(deltaEta,deltaPhi));

  return kFALSE;

}

//====================================================================================================================================================

bool MIDTrackletSelector::IsMIDTrackletSelectedWithSearchSpot(TVector3 posHitLayer1, TVector3 posHitLayer2, TVector3 posITStrackLayer1, bool evalEta=kFALSE) {

  if (!mIsSelectorSetup) {
    printf("ERROR: MIDTrackletSelector not initialized\n");
    return kFALSE;
  }

  if (posHitLayer1.Perp() > posHitLayer2.Perp()) {
    TVector3 tmp = posHitLayer1;
    posHitLayer1 = posHitLayer2;
    posHitLayer2 = tmp;
  }

  double deltaPhiITS = posITStrackLayer1.DeltaPhi(posHitLayer1);
  double deltaEtaITS = posITStrackLayer1.Eta() - posHitLayer1.Eta();
  
  if (TMath::Sqrt(deltaPhiITS*deltaPhiITS + deltaEtaITS*deltaEtaITS) > 0.2) return kFALSE;

  return IsMIDTrackletSelected(posHitLayer1, posHitLayer2, evalEta);
  
}

//====================================================================================================================================================

bool MIDTrackletSelector::IsMIDTrackletSelected(TVector3 posHitLayer1, TVector3 posHitLayer2, TVector3 trackITS, int charge=0) {

  if (!mIsSelectorSetup) {
    printf("ERROR: MIDTrackletSelector not initialized\n");
    return kFALSE;
  }

  if (posHitLayer1.Perp() > posHitLayer2.Perp()) {
    TVector3 tmp = posHitLayer1;
    posHitLayer1 = posHitLayer2;
    posHitLayer2 = tmp;
  }
  
  double deltaPhi = posHitLayer2.DeltaPhi(posHitLayer1);
  double deltaEta = posHitLayer2.Eta() - posHitLayer1.Eta();
  double eta      = trackITS.Eta();
  double mom      = trackITS.Mag();
  
  if (abs(eta) > mEtaMax) return kFALSE;

  if (mom > mMomMax) mom = mMomMax;
  if (mom < mMomMin) mom = mMomMin;
  
  double coord[4] = {deltaEta,deltaPhi,eta,mom};
  
  if      (charge > 0) return mTrackletAcc4D[kMuonPlus] ->GetBinContent(mTrackletAcc4D[kMuonPlus] ->GetBin(coord));
  else if (charge < 0) return mTrackletAcc4D[kMuonMinus]->GetBinContent(mTrackletAcc4D[kMuonMinus]->GetBin(coord));
  else                 return mTrackletAcc4D[kAllMuons] ->GetBinContent(mTrackletAcc4D[kAllMuons] ->GetBin(coord));
  
  return kFALSE;
  
}

//====================================================================================================================================================

bool MIDTrackletSelector::IsMIDTrackletSelectedWithSearchSpot(TVector3 posHitLayer1, TVector3 posHitLayer2, TVector3 trackITS, TVector3 posITStrackLayer1, int charge=0) {

  if (!mIsSelectorSetup) {
    printf("ERROR: MIDTrackletSelector not initialized\n");
    return kFALSE;
  }

  if (posHitLayer1.Perp() > posHitLayer2.Perp()) {
    TVector3 tmp = posHitLayer1;
    posHitLayer1 = posHitLayer2;
    posHitLayer2 = tmp;
  }

  double deltaPhiITS = posITStrackLayer1.DeltaPhi(posHitLayer1);
  double deltaEtaITS = posITStrackLayer1.Eta() - posHitLayer1.Eta();
  
  if (TMath::Sqrt(deltaPhiITS*deltaPhiITS + deltaEtaITS*deltaEtaITS) > 0.2) return kFALSE;

  return IsMIDTrackletSelected(posHitLayer1, posHitLayer2, trackITS, charge);
    
}

//====================================================================================================================================================
