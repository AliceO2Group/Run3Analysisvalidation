#include "io.C"
#include "style.C"
#include <iostream>
#include "TTree.h"
#include "TFile.h"
#include <vector>
#include "TVector3.h"
#include "TMatrixDSym.h"
#include "TClonesArray.h"
#include "TMath.h"
#include "TDatabasePDG.h"
#include "TParticle.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TRandom.h"
#include "TDatime.h"

#include "MIDTrackletSelector.h"

#ifdef __MAKECINT__
#pragma link C++ class vector<TClonesArray>+;
#endif

// IDs of the two MIS layers, taken from the PVIDMapFile.dat produced by g4me
const int idLayerMID1 = 300;
const int idLayerMID2 = 301;

const double rMaxITS = 110;  // (in cm). Above this radius, hits are not considered as belonging to the ITS

// This macro reads an output file from a g4me simulation and writes a TTree containing, event per event, a list of ITS tracks (i.e. TClonesArray
// of ITS hits from a same track) and MID tracklets (i.e. any combination of hits from the 1st and 2nd MID layers, passing the selections
// implemented in the macro IsMIDTrackletSelected.C) as an input for the fit routine based on GenFit

TTree *treeOut = 0;

IO_t io;

Bool_t IsTrackCharged(Int_t iTrack);
Bool_t IsTrackInteresting(Int_t iTrack);

//====================================================================================================================================================

void PrepareTracksForMatchingAndFit(const char *inputFileName, const char *outputFileName, const double hitMinP = 0.050) {
  
  TDatime t;
  gRandom->SetSeed(t.GetDate()+t.GetYear()*t.GetHour()*t.GetMinute()*t.GetSecond());
  
  MIDTrackletSelector *trackletSel = new MIDTrackletSelector();
  if (!(trackletSel -> Setup("muonTrackletAcceptance.root"))) {
    printf("MID tracklet selector could not be initialized. Quitting.\n");
    return;
  }
  
  style();

  const double resolutionITS =   5.e-4;  //   5 um
  const double resolutionMID = 100.e-4;  // 100 um

  io.open(inputFileName);
  auto nEvents = io.nevents();

  int nPreparedTracksITS=0, nHits_MIDLayer1=0, nHits_MIDLayer2=0, nPreparedTrackletsMID=0;

  TFile *fileOut = new TFile(outputFileName,"recreate");
  treeOut = new TTree("TracksToBeFitted","Tracks to be fitted");

  TClonesArray trackCandidatesHitPosITS("TClonesArray");     // array of hit position arrays (for the ITS tracks)  
  TClonesArray trackCandidatesHitCovITS("TClonesArray");     // array of hit covariance arrays (for the ITS tracks)
  TClonesArray trackCandidatesHitPosMID("TClonesArray");     // array of hit position arrays (for the MID tracklets)  
  TClonesArray trackCandidatesHitCovMID("TClonesArray");     // array of hit covariance arrays (for the MID tracklets)
  TClonesArray particlesITS("TParticle");                    // array of particles corresponding to the ITS tracks
  std::vector<int> idTrackITS;
  std::vector<int> idTrackMID;
  
  treeOut->Branch("TrackCandidatesHitPosITS",&trackCandidatesHitPosITS,256000,-1);
  treeOut->Branch("TrackCandidatesHitCovITS",&trackCandidatesHitCovITS,256000,-1);
  treeOut->Branch("TrackCandidatesHitPosMID",&trackCandidatesHitPosMID,256000,-1);
  treeOut->Branch("TrackCandidatesHitCovMID",&trackCandidatesHitCovMID,256000,-1);
  treeOut->Branch("ParticlesITS",            &particlesITS,            256000,-1);
  treeOut->Branch("idTrackITS",              &idTrackITS);
  treeOut->Branch("idTrackMID",              &idTrackMID);

  TVector3 pos, mom;
  TMatrixDSym covITS(3);
  for (int i=0; i<3; i++) covITS(i,i) = resolutionITS*resolutionITS;
  TMatrixDSym covMID(3);
  for (int i=0; i<3; i++) covMID(i,i) = resolutionMID*resolutionMID;

  // loop over events
  
  for (int iEv=0; iEv<nEvents; iEv++) {

    io.event(iEv);

    trackCandidatesHitPosITS.Clear();
    trackCandidatesHitCovITS.Clear();
    trackCandidatesHitPosMID.Clear();
    trackCandidatesHitCovMID.Clear();
    particlesITS.Clear();
    idTrackITS.clear();
    idTrackMID.clear();

    std::vector<TClonesArray> allTracksHitPosITS(io.tracks.n,TClonesArray("TVector3"));
    std::vector<TClonesArray> allTracksHitCovITS(io.tracks.n,TClonesArray("TMatrixDSym"));

    std::vector<int> arrayHitID_MIDLayer1(io.hits.n,-1);
    std::vector<int> arrayHitID_MIDLayer2(io.hits.n,-1);
    nHits_MIDLayer1 = 0;
    nHits_MIDLayer2 = 0;
    
    for (int iHit=0; iHit<io.hits.n; iHit++) {

      // filling arrays of hit IDs from MID layers (coming from any charged tracks)

      auto trackID = io.hits.trkid[iHit];

      if (!(IsTrackCharged(trackID))) continue;
      
      mom.SetXYZ(io.hits.px[iHit],io.hits.py[iHit],io.hits.pz[iHit]);
      if (mom.Mag() < hitMinP) continue;

      if (io.hits.lyrid[iHit] == idLayerMID1) arrayHitID_MIDLayer1[nHits_MIDLayer1++] = iHit;
      if (io.hits.lyrid[iHit] == idLayerMID2) arrayHitID_MIDLayer2[nHits_MIDLayer2++] = iHit;

      // filling arrays of hits from ITS tracks (only for interesting tracks: charged and primary).
      // Hits from ITS are by definition all the hits having radius < rMaxITS
      
      if (!(IsTrackInteresting(trackID))) continue;

      pos.SetXYZ(gRandom->Gaus(io.hits.x[iHit],resolutionITS),gRandom->Gaus(io.hits.y[iHit],resolutionITS),gRandom->Gaus(io.hits.z[iHit],resolutionITS));
      if (pos.Perp() < rMaxITS) {
	new ((allTracksHitPosITS.at(trackID))[(allTracksHitPosITS.at(trackID)).GetEntries()]) TVector3(pos);
	new ((allTracksHitCovITS.at(trackID))[(allTracksHitCovITS.at(trackID)).GetEntries()]) TMatrixDSym(covITS);
      }

    }

    // filling the final arrays with the hit information from good ITS tracks
    
    nPreparedTracksITS = 0;

    for (int iTrack=0; iTrack<io.tracks.n; iTrack++) {
      if (IsTrackInteresting(iTrack)) {
	new (trackCandidatesHitPosITS[nPreparedTracksITS]) TClonesArray(allTracksHitPosITS.at(iTrack));
	new (trackCandidatesHitCovITS[nPreparedTracksITS]) TClonesArray(allTracksHitCovITS.at(iTrack));
	idTrackITS.emplace_back(iTrack);
	TParticle part;
	part.SetPdgCode(io.tracks.pdg[iTrack]);
	part.SetProductionVertex(io.tracks.vx[iTrack],io.tracks.vy[iTrack],io.tracks.vz[iTrack],io.tracks.vt[iTrack]);
	part.SetMomentum(io.tracks.px[iTrack],io.tracks.py[iTrack],io.tracks.pz[iTrack],io.tracks.e[iTrack]);
	new (particlesITS[nPreparedTracksITS]) TParticle(part);
	nPreparedTracksITS++;
      }
    }

    // filling the final arrays with the hit information from selected MID tracklets

    nPreparedTrackletsMID = 0;
    TVector3 posHitMID1, posHitMID2;
    int idHitLayer1, idHitLayer2, trackIdHitLayer1, trackIdHitLayer2, trackletID;

    for (int iHitLayer1=0; iHitLayer1<nHits_MIDLayer1; iHitLayer1++) {
      
      idHitLayer1 = arrayHitID_MIDLayer1[iHitLayer1];
      posHitMID1.SetXYZ(gRandom->Gaus(io.hits.x[idHitLayer1],resolutionMID),gRandom->Gaus(io.hits.y[idHitLayer1],resolutionMID),gRandom->Gaus(io.hits.z[idHitLayer1],resolutionMID));
      trackIdHitLayer1 = io.hits.trkid[idHitLayer1];

      for (int iHitLayer2=0; iHitLayer2<nHits_MIDLayer2; iHitLayer2++) {

	idHitLayer2 = arrayHitID_MIDLayer2[iHitLayer2];
	posHitMID2.SetXYZ(gRandom->Gaus(io.hits.x[idHitLayer2],resolutionMID),gRandom->Gaus(io.hits.y[idHitLayer2],resolutionMID),gRandom->Gaus(io.hits.z[idHitLayer2],resolutionMID));
	trackIdHitLayer2 = io.hits.trkid[idHitLayer2];

	if (trackletSel->IsMIDTrackletSelected(posHitMID1,posHitMID2,kFALSE)) {

	  if (trackIdHitLayer1==trackIdHitLayer2) trackletID = trackIdHitLayer1;
	  else                                    trackletID = -1;
	  
	  TClonesArray trackletMIDpos("TVector3");
	  TClonesArray trackletMIDcov("TMatrixDSym");
	  
	  new (trackletMIDpos[trackletMIDpos.GetEntries()]) TVector3(posHitMID1);
	  new (trackletMIDpos[trackletMIDpos.GetEntries()]) TVector3(posHitMID2);
	  new (trackletMIDcov[trackletMIDcov.GetEntries()]) TMatrixDSym(covMID);
	  new (trackletMIDcov[trackletMIDcov.GetEntries()]) TMatrixDSym(covMID);
	  
	  new (trackCandidatesHitPosMID[nPreparedTrackletsMID]) TClonesArray(trackletMIDpos);
	  new (trackCandidatesHitCovMID[nPreparedTrackletsMID]) TClonesArray(trackletMIDcov);

	  idTrackMID.emplace_back(trackletID);
	  
	  nPreparedTrackletsMID++;
	  
	}

      }

    }

    printf("Ev %4d : %4d ITS tracks and %4d MID tracklets prepared for fitting\n",iEv,nPreparedTracksITS,nPreparedTrackletsMID);
    
    treeOut->Fill();

  }

  treeOut->Write();

}

//====================================================================================================================================================

Bool_t IsTrackInteresting(Int_t iTrack) {

  if (!(IsTrackCharged(iTrack)))            return kFALSE;
  if (!(io.tracks.parent[iTrack] == -1))    return kFALSE;

  return kTRUE;

}

//====================================================================================================================================================

Bool_t IsTrackCharged(Int_t iTrack) {

  if (iTrack<0 || iTrack>=io.tracks.n) {
    printf("ERROR: track index %d out of range (io.tracks.n = %d)\n",iTrack,io.tracks.n);
    return kFALSE;
  }
  if (!(TDatabasePDG::Instance()->GetParticle(io.tracks.pdg[iTrack])))                          return kFALSE;
  if (TMath::Abs(TDatabasePDG::Instance()->GetParticle(io.tracks.pdg[iTrack])->Charge()) < 0.1) return kFALSE;

  return kTRUE;

}

//====================================================================================================================================================
