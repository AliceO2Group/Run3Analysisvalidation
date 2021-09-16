#include <ConstField.h>
#include <Exception.h>
#include <FieldManager.h>
#include <KalmanFitterRefTrack.h>
#include <KalmanFitterInfo.h>
#include <KalmanFittedStateOnPlane.h>
#include <StateOnPlane.h>
#include <Track.h>
#include <TrackCand.h>
#include <TrackPoint.h>

#include <MeasurementProducer.h>
#include <MeasurementFactory.h>
#include <MeasuredStateOnPlane.h>

#include "mySpacepointDetectorHit.h"
#include "mySpacepointMeasurement.h"

#include <MaterialEffects.h>
#include <RKTrackRep.h>
#include <TGeoMaterialInterface.h>

#include <EventDisplay.h>

#include <HelixTrackModel.h>
#include <MeasurementCreator.h>

#include <TDatabasePDG.h>
#include <TEveManager.h>
#include <TGeoManager.h>
#include <TRandom.h>
#include "TVector3.h"
#include "TMatrixDSym.h"
#include <vector>

#include "TDatabasePDG.h"
#include <TMath.h>
#include "TFile.h"
#include "TTree.h"
#include "TParticle.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "THnSparse.h"
#include "TObjString.h"
#include "TDatime.h"

#include "MIDTrackletSelector.h"

enum part_t{kMIDElectron, kMIDMuon, kMIDPion, kMIDKaon, kMIDProton, kNPartTypes};
const int pdgCode[kNPartTypes] = {11, 13, 211, 321, 2212};
const char* partName[kNPartTypes] = {"electron", "muon", "pion", "kaon", "proton"};

const int nLayerITS = 12;
const int nMinMeasurementsITS = nLayerITS;

const double rLayerMID1 = 238.;   // in cm

enum {kGoodMatch, kFakeMatch};
const char *tagMatch[2] = {"GoodMatch", "FakeMatch"};

THnSparse *hDistanceFromGoodHitAtLayerMID1[kNPartTypes]={0};
TH3D *hChi2VsMomVsEtaMatchedTracks[kNPartTypes][2]={{0}};
TH2D *hMomVsEtaITSTracks[kNPartTypes]={0};

const int nMaxHelixSteps = 100;

void BookHistos();

void CircleFit(double x1, double y1, double x2, double y2, double x3, double y3, double &radius);
void EstimateInitialMomentum(genfit::mySpacepointDetectorHit* hitMin,
			     genfit::mySpacepointDetectorHit* hitMid,
			     genfit::mySpacepointDetectorHit* hitMax,
			     TVector3 vtx,
			     double fieldStrength,
			     double &charge,
			     TVector3 &mom);

//====================================================================================================================================================

void StudyMuonMatchingChi2(const char *inputFileName,
			   const char *outputFileName,
			   int pdg = -13,
			   bool displayTracks = kTRUE,
			   const char *geoFileName = "g4meGeometry.muon.root",
			   double fieldStrength = 0.5) {

  TDatime t;
  gRandom->SetSeed(t.GetDate()+t.GetYear()*t.GetHour()*t.GetMinute()*t.GetSecond());

  MIDTrackletSelector *trackletSel = new MIDTrackletSelector();
  if (!(trackletSel -> Setup("muonTrackletAcceptance.root"))) {
    printf("MID tracklet selector could not be initialized. Quitting.\n");
    return;
  }
  
  BookHistos();

  // init geometry and mag. field
  new TGeoManager("Geometry", "Geane geometry");
  TGeoManager::Import(geoFileName);
  genfit::FieldManager::getInstance()->init(new genfit::ConstField(0.,0., fieldStrength*10)); // in kGauss
  genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());

  // init event display
  genfit::EventDisplay* display = 0;
  if (displayTracks) display = genfit::EventDisplay::getInstance();

  // init fitter
  genfit::AbsKalmanFitter* fitter = new genfit::KalmanFitterRefTrack();
  fitter -> setMaxIterations(20);
  fitter -> setMinIterations(10);
  
  TFile *fileIn = new TFile(inputFileName);
  TTree *treeIn = (TTree*) fileIn->Get("TracksToBeFitted");
  TClonesArray *trackCandidatesHitPosITS=0, *trackCandidatesHitCovITS=0, *hitsPosITS=0, *hitsCovITS=0, *particlesITS=0;
  TClonesArray *trackCandidatesHitPosMID=0, *trackCandidatesHitCovMID=0, *hitsPosMID=0, *hitsCovMID=0;
  std::vector<int> *idTrackITS = 0;
  std::vector<int> *idTrackMID = 0;
  
  treeIn->SetBranchAddress("TrackCandidatesHitPosITS",&trackCandidatesHitPosITS);
  treeIn->SetBranchAddress("TrackCandidatesHitCovITS",&trackCandidatesHitCovITS);
  treeIn->SetBranchAddress("TrackCandidatesHitPosMID",&trackCandidatesHitPosMID);
  treeIn->SetBranchAddress("TrackCandidatesHitCovMID",&trackCandidatesHitCovMID);
  treeIn->SetBranchAddress("ParticlesITS",            &particlesITS);
  treeIn->SetBranchAddress("idTrackITS",              &idTrackITS);
  treeIn->SetBranchAddress("idTrackMID",              &idTrackMID);

  TClonesArray myDetectorHitArrayITS("genfit::mySpacepointDetectorHit");
  TClonesArray myDetectorHitArrayGlobal("genfit::mySpacepointDetectorHit");
  
  TParticle *part = 0;

  const double primVtxResolution =   3e-4;  //   3 um (for ~10 contributors)
  const double resolutionITS     =   5e-4;  //   5 um
  const double resolutionMID     = 100e-4;  // 100 um
  
  // init the factory

  int myDetId = 1;

  genfit::MeasurementFactory<genfit::AbsMeasurement> factoryITS;
  genfit::MeasurementProducer<genfit::mySpacepointDetectorHit, genfit::mySpacepointMeasurement> myProducerITS(&myDetectorHitArrayITS);
  factoryITS.addProducer(myDetId, &myProducerITS);

  genfit::MeasurementFactory<genfit::AbsMeasurement> factoryGlobal;
  genfit::MeasurementProducer<genfit::mySpacepointDetectorHit, genfit::mySpacepointMeasurement> myProducerGlobal(&myDetectorHitArrayGlobal);
  factoryGlobal.addProducer(myDetId, &myProducerGlobal);
  
  int nEvents = treeIn->GetEntries();

  // main loop

  for (int iEvent=0; iEvent<nEvents; iEvent++) {

    //    if (!(iEvent%100)) printf("\n----------- iEv = %5d of %5d ----------------\n",iEvent,nEvents);
    printf("\n----------- iEv = %5d of %5d ----------------\n",iEvent,nEvents);

    treeIn->GetEntry(iEvent);

    int nTracksITS    = trackCandidatesHitPosITS->GetEntries();
    int nTrackletsMID = trackCandidatesHitPosMID->GetEntries();

    vector<vector<vector<genfit::Track*>>> fitTracksGlobal(kNPartTypes,vector<vector<genfit::Track*>>(2));   // for drawing purposes only
    vector<vector<genfit::Track*>>         fitTracksITS(kNPartTypes);                                        // for drawing purposes only

    for (int iTrackITS=0; iTrackITS<nTracksITS; iTrackITS++) {

      bool fitITSConverged = kFALSE;
      double charge = 1.;   // abs value of muon charge
      TVector3 posAtLayerMID1, fittedMomAtVtx;
      
      myDetectorHitArrayITS.Clear();

      hitsPosITS = (TClonesArray*) trackCandidatesHitPosITS->At(iTrackITS);
      hitsCovITS = (TClonesArray*) trackCandidatesHitCovITS->At(iTrackITS);

      // TrackCand
      genfit::TrackCand myCandITS;

      int nMeasurementsITS = hitsPosITS->GetEntries();
      if (nMeasurementsITS < nMinMeasurementsITS) continue;

      //      printf("ITS track %3d has %2d nMeasurements\n",iTrackITS,nMeasurementsITS);

      for (int iHitITS=0; iHitITS<nMeasurementsITS; iHitITS++) {
	TVector3 *posHit    = (TVector3*)    hitsPosITS->At(iHitITS);
	TMatrixDSym *covHit = (TMatrixDSym*) hitsCovITS->At(iHitITS);
	new(myDetectorHitArrayITS[iHitITS]) genfit::mySpacepointDetectorHit(*posHit,*covHit);	
	myCandITS.addHit(myDetId, iHitITS);
      }
            
      TVector3 vtx(0,0,0);   // primary vertex

      part = (TParticle*) particlesITS->At(iTrackITS);
      vtx.SetXYZ(gRandom->Gaus(part->Vx(),primVtxResolution),
		 gRandom->Gaus(part->Vy(),primVtxResolution),
		 gRandom->Gaus(part->Vz(),primVtxResolution));

      TVector3 momIni;

      EstimateInitialMomentum((genfit::mySpacepointDetectorHit*)myDetectorHitArrayITS[0],
			      (genfit::mySpacepointDetectorHit*)myDetectorHitArrayITS[nMeasurementsITS/2],
			      (genfit::mySpacepointDetectorHit*)myDetectorHitArrayITS[nMeasurementsITS-1],
			      vtx,
			      fieldStrength,
			      charge,
			      momIni);

      if ((TDatabasePDG::Instance()->GetParticle(pdg)->Charge() * charge) < 0) pdg *= -1;

      // initial guess for cov
      TMatrixDSym covSeed(6);
      for (int i=0; i<3; i++) covSeed(i,i) = resolutionITS*resolutionITS;
      for (int i=3; i<6; i++) covSeed(i,i) = pow(resolutionITS / nMeasurementsITS / sqrt(3), 2);
      
      // set start values and pdg to cand
      myCandITS.setPosMomSeedAndPdgCode(vtx, momIni, pdg);
      myCandITS.setCovSeed(covSeed);

      // create track
      genfit::AbsTrackRep* repITS = new genfit::RKTrackRep(pdg);
      genfit::Track fitTrackITS(myCandITS, factoryITS, repITS);

      // do the fit: ITS track -------------------

      try {
	fitter->processTrack(&fitTrackITS);
      }
      catch(genfit::Exception& e) {
	std::cerr << e.what();
	std::cerr << "Exception, next track" << std::endl;
	continue;
      }

      fitTrackITS.checkConsistency();

      if (fitTrackITS.getFitStatus(repITS)->isFitConverged()) fitITSConverged = kTRUE;

      if (fitITSConverged) {

	genfit::MeasuredStateOnPlane fittedStateITS(fitTrackITS.getFittedState(0,repITS));

	// estimating kinematics at primary vertex
	fittedStateITS.extrapolateToPoint(vtx);
	fittedMomAtVtx = fittedStateITS.getMom();
      
	// estimating position at first MID layer
	// (I use a simple helix model which doesn't take into account propagation in materials. A proper way to do it would be fittedStateITS.extrapolateToCylinder(rLayerMID1)
	// but unfortunately the method crashes when a track is absorbed in the materials and doesn't manage to arrive the requested MID layer)
	genfit::HelixTrackModel helix(vtx, fittedMomAtVtx, charge);
	double length = 0;
	double deltaR = rLayerMID1;
	int nSteps = 0;
	while (deltaR > 1 && nSteps < nMaxHelixSteps) {    // 1 cm tolerance for the radial distance between the MID layer and the effective extrapolation radius of the helix
	  length += deltaR;
	  posAtLayerMID1 = helix.getPos(length);
	  deltaR = rLayerMID1 - posAtLayerMID1.Perp();
	  nSteps++;
	}

      }
      
      // do the fit: Global track -------------------

      bool isGoodMatch = kFALSE;
      double bestChi2OverNDF_Global = 99999999.;
      genfit::Track *bestGlobalTrack=0;
      TVector3 goodHitAtLayerMID1;
      bool goodTrackletExists = kFALSE;

      int nSelTracklets = 0;

      for (int iTrackletMID=0; iTrackletMID<nTrackletsMID; iTrackletMID++) {

	if (!fitITSConverged) continue;
	
	myDetectorHitArrayGlobal.Clear();

	hitsPosMID = (TClonesArray*) trackCandidatesHitPosMID->At(iTrackletMID);
	hitsCovMID = (TClonesArray*) trackCandidatesHitCovMID->At(iTrackletMID);

	int nMeasurementsMID = hitsPosMID->GetEntries();
	if (nMeasurementsMID != 2) continue;

	//	if (!(trackletSel->IsMIDTrackletSelected(*((TVector3*)hitsPosMID->At(0)),*((TVector3*)hitsPosMID->At(1)),fittedMomAtVtx,posAtLayerMID1,charge))) continue;
	if (!(trackletSel->IsMIDTrackletSelectedWithSearchSpot(*((TVector3*)hitsPosMID->At(0)),*((TVector3*)hitsPosMID->At(1)),posAtLayerMID1,kFALSE))) continue;

	nSelTracklets++;
	
	// TrackCand
	genfit::TrackCand myCandGlobal;

	if (idTrackITS->at(iTrackITS) == idTrackMID->at(iTrackletMID)) {
	  // WARNING: if more than a tracklet has the track ID of the ITS track (for instance tracks doing spirals), the last registered one is
	  // registered in goodHitAtLayerMID1. However, the tracklet selector should remove the "backward tracklets" thanks to the comparison
	  // at the first MID layer between the tracklet position and the extrapolation of the ITS track
	  goodHitAtLayerMID1.SetXYZ(((TVector3*)hitsPosMID->At(0))->X(),((TVector3*)hitsPosMID->At(0))->Y(),((TVector3*)hitsPosMID->At(0))->Z());
	  goodTrackletExists = kTRUE;
	}

	int nHitsGlobal = 0;
	
	for (int iHitITS=0; iHitITS<nMeasurementsITS; iHitITS++) {
	  TVector3 *posHit    = (TVector3*)    hitsPosITS->At(iHitITS);
	  TMatrixDSym *covHit = (TMatrixDSym*) hitsCovITS->At(iHitITS);
	  new(myDetectorHitArrayGlobal[nHitsGlobal]) genfit::mySpacepointDetectorHit(*posHit,*covHit);
	  myCandGlobal.addHit(myDetId, nHitsGlobal);
	  nHitsGlobal++;
	}
	for (int iHitMID=0; iHitMID<nMeasurementsMID; iHitMID++) {
	  TVector3 *posHit    = (TVector3*)    hitsPosMID->At(iHitMID);
	  TMatrixDSym *covHit = (TMatrixDSym*) hitsCovMID->At(iHitMID);
	  new(myDetectorHitArrayGlobal[nHitsGlobal]) genfit::mySpacepointDetectorHit(*posHit,*covHit);
	  myCandGlobal.addHit(myDetId, nHitsGlobal);
	  nHitsGlobal++;
	}

	myCandGlobal.setPosMomSeedAndPdgCode(vtx, fittedMomAtVtx, pdg);
	myCandGlobal.setCovSeed(covSeed);
	
	genfit::AbsTrackRep* repGlobal = new genfit::RKTrackRep(pdg);
	genfit::Track fitTrackGlobal(myCandGlobal, factoryGlobal, repGlobal);

	try {
	  fitter->processTrack(&fitTrackGlobal);
	}
	catch(genfit::Exception& e) {
	  std::cerr << e.what();
	  std::cerr << "Exception, next track" << std::endl;
	  continue;
	}

	fitTrackGlobal.checkConsistency();
		
	double chi2OverNDF_Global = fitTrackGlobal.getFitStatus(repGlobal)->getChi2()/fitTrackGlobal.getFitStatus(repGlobal)->getNdf();

	// the best matching tracklet is defined as the one minimizing the global track chi2
	if (chi2OverNDF_Global < bestChi2OverNDF_Global) {
	  bestChi2OverNDF_Global = chi2OverNDF_Global;
	  isGoodMatch = (idTrackITS->at(iTrackITS) == idTrackMID->at(iTrackletMID));
	  if (bestGlobalTrack) delete bestGlobalTrack;
	  bestGlobalTrack = new genfit::Track(fitTrackGlobal);
	}

      }

      //      printf("%3d selected tracklets out of %3d\n",nSelTracklets,nTrackletsMID);

      int pdgCodePart = TMath::Abs(part->GetPdgCode());
      double momPart  = part->P();
      double etaPart  = part->Eta();
	   
      for (int iPartType=0; iPartType<kNPartTypes; iPartType++) {
	if (pdgCodePart == pdgCode[iPartType]) {

	  // filling histos with the ITS track information

	  hMomVsEtaITSTracks[iPartType]->Fill(etaPart,momPart);

	  fitTracksITS[iPartType].push_back(new genfit::Track(fitTrackITS));

	  if (goodTrackletExists) {
	    double deltaPhi = posAtLayerMID1.DeltaPhi(goodHitAtLayerMID1);
	    double deltaEta = posAtLayerMID1.Eta() - goodHitAtLayerMID1.Eta();
	    double var[4] = {deltaEta,deltaPhi,etaPart,momPart};
	    hDistanceFromGoodHitAtLayerMID1[iPartType] -> Fill(var);
	  }
	  
	  // filling histos with the best ITS-MID match information
	  
	  if (bestGlobalTrack) {
	    
	    if (isGoodMatch) {
	      hChi2VsMomVsEtaMatchedTracks[iPartType][kGoodMatch] -> Fill(bestChi2OverNDF_Global,etaPart,momPart);
	      if (iEvent<100) fitTracksGlobal[iPartType][kGoodMatch].push_back(new genfit::Track(*bestGlobalTrack));
	    }
	    else {
	      hChi2VsMomVsEtaMatchedTracks[iPartType][kFakeMatch] -> Fill(bestChi2OverNDF_Global,etaPart,momPart);
	      if (iEvent<100) fitTracksGlobal[iPartType][kFakeMatch].push_back(new genfit::Track(*bestGlobalTrack));
	    }

	  }

	  break;

	}
      }
      
    }
    
    if (iEvent < 100 && display) {
      // add tracks to event display
      display->addEvent(fitTracksGlobal[kMIDMuon][kGoodMatch]);
    }


  } // end loop over events

  delete fitter;
  
  TFile *fileOut = new TFile(outputFileName,"recreate");
  for (int iPart=0; iPart<kNPartTypes; iPart++) {
    hMomVsEtaITSTracks[iPart] -> Write();
    hDistanceFromGoodHitAtLayerMID1[iPart] -> Write();
    for (int iMatch=0; iMatch<2; iMatch++) {
      hChi2VsMomVsEtaMatchedTracks[iPart][iMatch] -> Write();
    }
  }

  fileOut -> Close(); 

  // open event display
  if (display) display->open();

}

//====================================================================================================================================================

void BookHistos() {

  // non-uniform p binning
  
  const int nMomBins = 40;
  const double momBinCenter[nMomBins] = {
    1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 
    2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9,
    3.0, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9,
    4.0, 4.5, 5.0, 6.0, 7.0, 8.0, 10., 12., 15., 20.
  };
  
  double momBinLimits[nMomBins+1] = {0};
  momBinLimits[0] = momBinCenter[0] - 0.5*(momBinCenter[1]-momBinCenter[0]);
  for (int iMomBin=0; iMomBin<nMomBins-1; iMomBin++) momBinLimits[iMomBin+1] = 0.5 * (momBinCenter[iMomBin]+momBinCenter[iMomBin+1]);
  momBinLimits[nMomBins] = momBinCenter[nMomBins-1] + 0.5*(momBinCenter[nMomBins-1]-momBinCenter[nMomBins-2]);
  
  // uniform eta binning

  const int nEtaBins = 33;
  const double etaMin = -1.65;
  const double etaMax =  1.65;
  
  for (int iPart=0; iPart<kNPartTypes; iPart++) {

    hMomVsEtaITSTracks[iPart] = new TH2D(Form("hMomVsEtaITSTracks_%s",partName[iPart]),Form("hMomVsEtaITSTracks_%s",partName[iPart]),
					 nEtaBins,etaMin,etaMax,nMomBins,momBinLimits[0],momBinLimits[nMomBins]);
    hMomVsEtaITSTracks[iPart] -> GetYaxis() -> Set(nMomBins,momBinLimits);

    hMomVsEtaITSTracks[iPart] -> Sumw2();
    hMomVsEtaITSTracks[iPart] -> SetXTitle("#eta");	    
    hMomVsEtaITSTracks[iPart] -> SetYTitle("p (GeV/c)");
    
    for (int iMatch=0; iMatch<2; iMatch++) {
      hChi2VsMomVsEtaMatchedTracks[iPart][iMatch] = new TH3D(Form("hChi2VsMomVsEtaMatchedTracks_%s_%s",partName[iPart],tagMatch[iMatch]),
							     Form("hChi2VsMomVsEtaMatchedTracks_%s_%s",partName[iPart],tagMatch[iMatch]),
							     200,0,20,nEtaBins,etaMin,etaMax,nMomBins,momBinLimits[0],momBinLimits[nMomBins]);
      hChi2VsMomVsEtaMatchedTracks[iPart][iMatch] -> GetZaxis() -> Set(nMomBins,momBinLimits);
      
      hChi2VsMomVsEtaMatchedTracks[iPart][iMatch] -> Sumw2();
      hChi2VsMomVsEtaMatchedTracks[iPart][iMatch] -> SetXTitle(Form("#chi^{2}/ndf (%s, %s)",partName[iPart],tagMatch[iMatch]));
      hChi2VsMomVsEtaMatchedTracks[iPart][iMatch] -> SetYTitle("#eta");	    
      hChi2VsMomVsEtaMatchedTracks[iPart][iMatch] -> SetZTitle("p (GeV/c)");
      
    }

    int nBins[4] = {300,300,nEtaBins,nMomBins};
    double xMin[4] = {-0.3, -0.3, etaMin, momBinLimits[0]};
    double xMax[4] = { 0.3,  0.3, etaMax, momBinLimits[nMomBins]};
    
    hDistanceFromGoodHitAtLayerMID1[iPart] = new THnSparseD(Form("hDistanceFromGoodHitAtLayerMID1_%s",partName[iPart]),
							    Form("hDistanceFromGoodHitAtLayerMID1_%s",partName[iPart]),
							    4, nBins,xMin,xMax);
    hDistanceFromGoodHitAtLayerMID1[iPart] -> GetAxis(3) -> Set(nMomBins,momBinLimits);
    
    hDistanceFromGoodHitAtLayerMID1[iPart] -> GetAxis(0) -> SetTitle("#Delta#eta");
    hDistanceFromGoodHitAtLayerMID1[iPart] -> GetAxis(1) -> SetTitle("#Delta#phi");
    hDistanceFromGoodHitAtLayerMID1[iPart] -> GetAxis(2) -> SetTitle("#eta");
    hDistanceFromGoodHitAtLayerMID1[iPart] -> GetAxis(3) -> SetTitle("#p (GeV/c)");
    
  }

}

//====================================================================================================================================================

void CircleFit(double x1, double y1, double x2, double y2, double x3, double y3, double &radius) {

  auto d1x = y2-y1;
  auto d1y = x1-x2;
  auto d2x = y3-y1;
  auto d2y = x1-x3;

  auto k = d1y*d2x - d1x*d2y;

  if (TMath::Abs(k)<0.000001) {
    radius = 999999999;
    return;
  }
  
  auto s1x = (x1+x2)/2;
  auto s1y = (y1+y2)/2;
  auto s2x = (x1+x3)/2;
  auto s2y = (y1+y3)/2;
  auto l   = d1x * (s2y-s1y) - d1y * (s2x - s1x);
  auto m   = l/k;

  auto centerX = s2x + m*d2x;
  auto centerY = s2y + m*d2y;

  auto dx = centerX - x1;
  auto dy = centerY - y1;
  radius = TMath::Sqrt(dx*dx + dy*dy);

}

//====================================================================================================================================================

void EstimateInitialMomentum(genfit::mySpacepointDetectorHit* hitMin,
			     genfit::mySpacepointDetectorHit* hitMid,
			     genfit::mySpacepointDetectorHit* hitMax,
			     TVector3 vtx,
			     double fieldStrength,
			     double &charge,
			     TVector3 &mom) {

  double radius = 0;
  CircleFit(hitMin->getPos().X(), hitMin->getPos().Y(),
	    hitMid->getPos().X(), hitMid->getPos().Y(),
	    hitMax->getPos().X(), hitMax->getPos().Y(),
	    radius);
  
  double pt = TMath::Abs(radius*0.01 * fieldStrength * charge / 3.3);     // momentum component transverse to the mag. field
  
  TVector3 v1(hitMax->getPos().X() - hitMin->getPos().X(),
	      hitMax->getPos().Y() - hitMin->getPos().Y(),
	      hitMax->getPos().Z() - hitMin->getPos().Z());

  TVector3 v2(hitMin->getPos().X() - vtx.X(),
	      hitMin->getPos().Y() - vtx.Y(),
	      hitMin->getPos().Z() - vtx.Z());

  double eta = v1.Eta();
  double phi = v2.Phi();

  mom.SetPtEtaPhi(pt,eta,phi);
  
  TVector3 v3(hitMax->getPos().X() - hitMid->getPos().X(),
	      hitMax->getPos().Y() - hitMid->getPos().Y(),
	      0);
  
  TVector3 v4(hitMid->getPos().X() - hitMin->getPos().X(),
	      hitMid->getPos().Y() - hitMin->getPos().Y(),
	      0);

  TVector3 v5 = v3.Cross(v4);
  
  if (v5.Z() < 0) charge *= -1;
  
}

//====================================================================================================================================================
