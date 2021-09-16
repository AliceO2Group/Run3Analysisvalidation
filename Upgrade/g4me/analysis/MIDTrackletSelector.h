#ifndef MIDTrackletSelector_h
#define MIDTrackletSelector_h

#include "THnSparse.h"
#include "TH3.h"
#include "TH2.h"
#include "TFile.h"
#include "TVector3.h"
#include "TMath.h"

using namespace std;

class MIDTrackletSelector {
  
public:
  MIDTrackletSelector();
  ~MIDTrackletSelector() = default;

  enum { kMuonMinus, kMuonPlus, kAllMuons, kNChargeOptions };
  
  bool Setup(const Char_t *nameInputFile);
  bool IsSelectorSetup() { return mIsSelectorSetup; }
  bool IsMIDTrackletSelected(TVector3 posHitLayer1, TVector3 posHitLayer2, bool evalEta);
  bool IsMIDTrackletSelected(TVector3 posHitLayer1, TVector3 posHitLayer2, TVector3 trackITS, int charge);
  bool IsMIDTrackletSelectedWithSearchSpot(TVector3 posHitLayer1, TVector3 posHitLayer2, TVector3 posITStrackLayer1, bool evalEta);
  bool IsMIDTrackletSelectedWithSearchSpot(TVector3 posHitLayer1, TVector3 posHitLayer2, TVector3 trackITS, TVector3 posITStrackLayer1, int charge);

  TH2C* GetAcc2D()                { return mTrackletAcc2D; }
  TH3C* GetAcc3D()                { return mTrackletAcc3D; }
  THnSparse* GetAcc4D(int charge) { return mTrackletAcc4D[charge]; }
  
protected:
  
  TFile *mInputFile;
  TH3C *mTrackletAcc3D;
  TH2C *mTrackletAcc2D;
  THnSparse *mTrackletAcc4D[kNChargeOptions];
  bool mIsSelectorSetup;
  double mEtaMax;
  double mMomMax;
  double mMomMin;

};

#endif

