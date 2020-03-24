#if !defined(__CINT__) || defined(__MAKECINT__)
#include <TMath.h>
#include <TFile.h>
#include "AliESDtrack.h"
#include "AliESDtrackCuts.h"
#include "AliAODVertex.h"
#include "AliESDVertex.h"
#include "AliESDEvent.h"
#include "AliAODRecoDecayHF2Prong.h"
#include "AliAODRecoDecayHF3Prong.h"
#include "AliVertexerTracks.h"
#endif

Bool_t GetTrackMomentumAtSecVert(AliESDtrack* tr, AliAODVertex* secVert, Double_t momentum[3], float fBzkG){
  /// fast calculation (no covariance matrix treatment) of track momentum at secondary vertex

  Double_t alpha=tr->GetAlpha();
  Double_t sn=TMath::Sin(alpha), cs=TMath::Cos(alpha);
  Double_t x=tr->GetX(), y=tr->GetParameter()[0], snp=tr->GetParameter()[2];
  Double_t xv= secVert->GetX()*cs + secVert->GetY()*sn;
  Double_t yv=-secVert->GetX()*sn + secVert->GetY()*cs;
  x-=xv; y-=yv;
  Double_t crv=tr->GetC(fBzkG);
  if (TMath::Abs(fBzkG) < 0.000001) crv=0.;
  double csp = TMath::Sqrt((1.-snp)*(1.+snp));
  
  Double_t tgfv=-(crv*x - snp)/(crv*y + csp);
  cs = 1./TMath::Sqrt(1+tgfv*tgfv);
  sn = cs<1. ? tgfv*cs : 0.;

  x = xv*cs + yv*sn;
  Double_t alpNew = alpha+TMath::ASin(sn);
  Double_t ca=TMath::Cos(alpNew-alpha), sa=TMath::Sin(alpNew-alpha);
  Double_t p2=tr->GetSnp();
  Double_t xNew=tr->GetX()*ca + tr->GetY()*sa;
  Double_t p2New=p2*ca - TMath::Sqrt((1.- p2)*(1.+p2))*sa;
  momentum[0]=tr->GetSigned1Pt();
  momentum[1]=p2New*(x-xNew)*tr->GetC(fBzkG);
  momentum[2]=tr->GetTgl();
  Bool_t retCode=tr->Local2GlobalMomentum(momentum,alpNew);
  return retCode;
}

Bool_t SingleTrkCuts(AliESDtrack *trk, AliESDtrackCuts *esdTrackCuts, AliESDVertex* fV1, Double_t fBzkG){
  if (!trk->PropagateToDCA(fV1,fBzkG,kVeryBig)) return kFALSE;
  trk->RelateToVertex(fV1,fBzkG,kVeryBig);
  return esdTrackCuts->AcceptTrack(trk);
}

Bool_t SingleTrkCutsSimple(AliESDtrack *trk, AliESDtrackCuts *esdTrackCuts, AliESDVertex* fV1, Double_t fBzkG){
  Int_t status=trk->GetStatus();
  bool sel_track = status & AliESDtrack::kITSrefit && (trk->HasPointOnITSLayer(0) || trk->HasPointOnITSLayer(1));
  sel_track = sel_track && trk->GetNcls(1)>70;
  return sel_track;
}

AliESDVertex* ReconstructSecondaryVertex(AliVertexerTracks* vt, TObjArray *trkArray, AliESDVertex* primvtx){


  vt->SetVtxStart(primvtx);
  
  AliESDVertex* trkv=(AliESDVertex*)vt->VertexForSelectedESDTracks(trkArray);
  if(trkv->GetNContributors()!=trkArray->GetEntriesFast()) return 0x0;
  Double_t vertRadius2=trkv->GetX()*trkv->GetX()+trkv->GetY()*trkv->GetY();
  //FIXME if(vertRadius2>8.) return 0x0;
  return trkv;
}

AliAODVertex* ConvertToAODVertex(AliESDVertex* trkv){
  Double_t pos_[3],cov_[6],chi2perNDF_;
  trkv->GetXYZ(pos_); // position
  trkv->GetCovMatrix(cov_); //covariance matrix
  chi2perNDF_ = trkv->GetChi2toNDF();
  double dispersion_ = trkv->GetDispersion();
  //  printf(" pos_ %f %f %f \n", pos_[0], pos_[1], pos_[2]);
  AliAODVertex *vertexAOD = new AliAODVertex(pos_,cov_,chi2perNDF_,0x0,-1,AliAODVertex::kUndef,2);
  return vertexAOD;
}

AliAODRecoDecayHF2Prong* Make2Prong(TObjArray *twoTrackArray, AliAODVertex *secVert, Double_t fBzkG){
  
  AliESDtrack *track_0= (AliESDtrack*)twoTrackArray->UncheckedAt(0);
  AliESDtrack *track_1= (AliESDtrack*)twoTrackArray->UncheckedAt(1);
  
  Double_t px[2],py[2],pz[2],d0[2],d0err[2];
  Double_t momentum[3];
  GetTrackMomentumAtSecVert(track_0,secVert,momentum,fBzkG);
  px[0] = momentum[0]; py[0] = momentum[1]; pz[0] = momentum[2];
  GetTrackMomentumAtSecVert(track_1,secVert,momentum,fBzkG);
  px[1] = momentum[0]; py[1] = momentum[1]; pz[1] = momentum[2];
  
  Float_t d0z0f[2],covd0z0f[3];
  track_0->GetImpactParameters(d0z0f,covd0z0f);
  d0[0]=d0z0f[0];
  d0err[0] = TMath::Sqrt(covd0z0f[0]);
  track_1->GetImpactParameters(d0z0f,covd0z0f);
  d0[1]=d0z0f[0];
  d0err[1] = TMath::Sqrt(covd0z0f[0]);
  
  Double_t xdummy,ydummy;
  float dcap1n1 = track_0->GetDCA(track_1,fBzkG,xdummy,ydummy);

  AliAODRecoDecayHF2Prong* the2Prong = new AliAODRecoDecayHF2Prong(0x0,px,py,pz,d0,d0err,dcap1n1);
  // AliAODVertex* ownsecv=secVert->CloneWithoutRefs();
  // the2Prong->SetOwnSecondaryVtx(ownsecv);
  return the2Prong;
}

AliAODRecoDecayHF3Prong* Make3Prong(TObjArray *threeTrackArray, AliAODVertex *secVert, Double_t fBzkG){

  AliESDtrack *track_0= (AliESDtrack*)threeTrackArray->UncheckedAt(0);
  AliESDtrack *track_1= (AliESDtrack*)threeTrackArray->UncheckedAt(1);
  AliESDtrack *track_2= (AliESDtrack*)threeTrackArray->UncheckedAt(2);
 
  Double_t px[3],py[3],pz[3],d0[3],d0err[3];
  Double_t momentum[3];
  GetTrackMomentumAtSecVert(track_0,secVert,momentum,fBzkG);
  px[0] = momentum[0]; py[0] = momentum[1]; pz[0] = momentum[2];
  GetTrackMomentumAtSecVert(track_1,secVert,momentum,fBzkG);
  px[1] = momentum[0]; py[1] = momentum[1]; pz[1] = momentum[2];
  GetTrackMomentumAtSecVert(track_2,secVert,momentum,fBzkG);
  px[2] = momentum[0]; py[2] = momentum[1]; pz[2] = momentum[2];
  Float_t d0z0f[2],covd0z0f[3];
  track_0->GetImpactParameters(d0z0f,covd0z0f);
  d0[0]=d0z0f[0];
  d0err[0] = TMath::Sqrt(covd0z0f[0]);
  track_1->GetImpactParameters(d0z0f,covd0z0f);
  d0[1]=d0z0f[0];
  d0err[1] = TMath::Sqrt(covd0z0f[0]);
  track_2->GetImpactParameters(d0z0f,covd0z0f);
  d0[2]=d0z0f[0];
  d0err[2] = TMath::Sqrt(covd0z0f[0]);
  
  Double_t xdummy,ydummy;
  float dcap1n1 = track_0->GetDCA(track_1,fBzkG,xdummy,ydummy);
  float dcap2n1 = track_2->GetDCA(track_1,fBzkG,xdummy,ydummy);
  float dcap1p2 = track_0->GetDCA(track_2,fBzkG,xdummy,ydummy);
  Double_t dca[3]={dcap1n1,dcap2n1,dcap1p2};
  Double_t dispersion=0;
  Double_t dist12=0.;
  Double_t dist23=0.;
  Short_t charge=(Short_t)(track_0->Charge()+track_1->Charge()+track_2->Charge());

  // construct the candidate passing a NULL pointer for the secondary vertex to avoid creation of TRef
  AliAODRecoDecayHF3Prong *the3Prong = new AliAODRecoDecayHF3Prong(0x0,px,py,pz,d0,d0err,dca,dispersion,dist12,dist23,charge);
  // add a pointer to the secondary vertex via SetOwnSecondaryVtx (no TRef created)
  // AliAODVertex* ownsecv=secVert->CloneWithoutRefs();
  // the3Prong->SetOwnSecondaryVtx(ownsecv);
  return the3Prong;
}

Bool_t ComputeVerticesRun1_Opt(TString esdfile = "AliESDs.root", TString output = "Vertices2prong-ITS1.root"){

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
  
  TH1F* hpt_nocuts=new TH1F("hpt_nocuts"," ; pt tracks (#GeV) ; Entries",100, 0, 10.);
  TH1F* htgl_nocuts=new TH1F("htgl_nocuts", "tgl tracks (#GeV)", 100, 0., 10.);
  TH1F* hpt_cuts=new TH1F("hpt_cuts"," ; pt tracks (#GeV) ; Entries",100, 0, 10.);
  TH1F* htgl_cuts=new TH1F("htgl_cuts", "tgl tracks (#GeV)", 100, 0., 10.);
  TH1F* hvx=new TH1F("hvx"," Secondary vertex ; X vertex (cm) ; Entries",100,-0.1, 0.1);
  TH1F* hvy=new TH1F("hvy"," Secondary vertex ; Y vertex (cm) ; Entries",100,-0.1, 0.1);
  TH1F* hvz=new TH1F("hvz"," Secondary vertex ; Z vertex (cm) ; Entries",100,-0.1, 0.1);
  TH1F* hitsmap=new TH1F("hitsmap", "hitsmap_cuts", 100, 0., 100.);
  
  TH1F* hvertexx=new TH1F("hvertexx", " Primary vertex ; X vertex (cm) ; Entries", 100, -1.0, 1.0);
  TH1F* hvertexy=new TH1F("hvertexy", " Primary vertex ; Y vertex (cm) ; Entries", 100, -1.0, 1.0);
  TH1F* hvertexz=new TH1F("hvertexz", " Primary vertex ; Z vertex (cm) ; Entries", 100, -1.0, 1.0);
  
  TH1F* hdecayxyz=new TH1F("hdecayxyz", "hdecayxyz", 100, 0., 1.0);
  TH1F* hdecayxy=new TH1F("hdecayxy", "hdecayxy", 100, 0., 1.0);
  TH1F* hmass0=new TH1F("hmass", "; Inv Mass (GeV/c^{2})", 500, 0, 5.0);
  TH1F* hmassP=new TH1F("hmassP", "; Inv Mass (GeV/c^{2})", 500, 0, 5.0);
  
  AliESDtrackCuts *esdTrackCuts = new AliESDtrackCuts("AliESDtrackCuts","default");
  esdTrackCuts->SetPtRange(0.5,1.e10);
  esdTrackCuts->SetEtaRange(-0.8,+0.8);
  esdTrackCuts->SetMinNClustersTPC(50);
  esdTrackCuts->SetRequireITSRefit(kTRUE);
  esdTrackCuts->SetClusterRequirementITS(AliESDtrackCuts::kSPD,
					 AliESDtrackCuts::kAny);
  esdTrackCuts->SetAcceptKinkDaughters(kFALSE);
  esdTrackCuts->SetMaxDCAToVertexZ(3.2);
  esdTrackCuts->SetMaxDCAToVertexXY(2.4);
  esdTrackCuts->SetDCAToVertex2D(kTRUE);

  for (Int_t iEvent = 0; iEvent < tree->GetEntries(); iEvent++) {
    tree->GetEvent(iEvent);
    if (!esd) {
      printf("Error: no ESD object found for event %d", iEvent);
      return kFALSE;
    }
    printf("\n------------ Run %d Event: %d  Tracks %d ------------------\n",esd->GetRunNumber(),iEvent,esd->GetNumberOfTracks());
    TString trClass=esd->GetFiredTriggerClasses();
    printf("      Fired Trigger Classes %s\n",trClass.Data());
    //FIXME if(!trClass.Contains("CV0L7-B")) continue;

    Bool_t do3Prongs=kFALSE;
    Int_t maxTracksToProcess=9999999; /// temporary to limit the time duration of tests
    Int_t totTracks=TMath::Min(maxTracksToProcess,esd->GetNumberOfTracks());

    AliESDVertex *primvtx = (AliESDVertex*)esd->GetPrimaryVertex();
    if(!primvtx) return kFALSE;
    TString title=primvtx->GetTitle();
    if(primvtx->IsFromVertexer3D() || primvtx->IsFromVertexerZ()) continue;
    if(primvtx->GetNContributors()<2) continue;
    hvertexx->Fill(primvtx->GetX());
    hvertexy->Fill(primvtx->GetY());
    hvertexz->Fill(primvtx->GetZ());
    //   AliAODVertex *vertexAODp = ConvertToAODVertex(primvtx);

    Double_t fBzkG = (Double_t)esd->GetMagneticField();

    // Apply single track cuts and flag them
    UChar_t* status = new UChar_t[totTracks];    
    for (Int_t iTrack = 0; iTrack < totTracks; iTrack++) {
      status[iTrack]=0;
      AliESDtrack* track = esd->GetTrack(iTrack);
      if (SingleTrkCutsSimple(track,esdTrackCuts,primvtx,fBzkG)) status[iTrack]=1; //FIXME
    }
     
    TObjArray *twoTrackArray = new TObjArray(2);
    TObjArray *threeTrackArray = new TObjArray(3);

    
    AliVertexerTracks* vt=new AliVertexerTracks(fBzkG);
    
    Double_t mom0[3], mom1[3], mom2[3];
    for (Int_t iTrack_0 = 0; iTrack_0 < totTracks; iTrack_0++) {
      AliESDtrack* track_0 = esd->GetTrack(iTrack_0);
      track_0->GetPxPyPz(mom0);
      hpt_nocuts->Fill(track_0->Pt());
      htgl_nocuts->Fill(track_0->GetTgl()); 
      if (status[iTrack_0]==0) continue;
      hpt_cuts->Fill(track_0->Pt());
      htgl_cuts->Fill(track_0->GetTgl()); 
      hitsmap->Fill(track_0->GetITSClusterMap());

      for (Int_t iTrack_1 = iTrack_0 + 1; iTrack_1 < totTracks; iTrack_1++) {
        AliESDtrack* track_1 = esd->GetTrack(iTrack_1);
        track_1->GetPxPyPz(mom1);
	if(track_1->Charge() * track_0->Charge() >0) continue;
	if (status[iTrack_1]==0) continue;

        twoTrackArray->AddAt(track_0, 0);
        twoTrackArray->AddAt(track_1, 1);
	AliESDVertex* trkv=ReconstructSecondaryVertex(vt,twoTrackArray,primvtx);
	if(trkv==0x0){
	  twoTrackArray->Clear();
	  continue;
	}

	//	printf(" px track_0 %.4f, track_1 %.4f \n", TMath::Max(track_0->Px(),track_1->Px()),TMath::Min(track_0->Px(),track_1->Px())); 
	hvx->Fill(trkv->GetX());
	hvy->Fill(trkv->GetY());
	hvz->Fill(trkv->GetZ());
	double deltax = trkv->GetX() - primvtx->GetX();
	double deltay = trkv->GetY() - primvtx->GetY();
	double deltaz = trkv->GetZ() - primvtx->GetZ();
	double decaylength = TMath::Sqrt(deltax*deltax+deltay*deltay+deltaz*deltaz);
	double decaylengthxy = TMath::Sqrt(deltax*deltax+deltay*deltay);

	hdecayxyz->Fill(decaylength);
	hdecayxy->Fill(decaylengthxy);
		
	AliAODVertex* vertexAOD=ConvertToAODVertex(trkv);
	delete trkv;
        AliAODRecoDecayHF2Prong *the2Prong = Make2Prong(twoTrackArray,vertexAOD,fBzkG);
	//        the2Prong->SetOwnPrimaryVtx(vertexAODp);
	Double_t m0=the2Prong->InvMassD0();
	Double_t m0b=the2Prong->InvMassD0bar();
        hmass0->Fill(m0);
        hmass0->Fill(m0b);
	delete the2Prong;
	delete vertexAOD;
	//	printf(" masses = %f %f\n",TMath::Max(m0,m0b),TMath::Min(m0,m0b));
	if(do3Prongs){
	  for (Int_t iTrack_2 = iTrack_0 + 1; iTrack_2 < totTracks; iTrack_2++) {
	    if(iTrack_2==iTrack_0 || iTrack_2==iTrack_1) continue;
	    AliESDtrack* track_2 = esd->GetTrack(iTrack_2);
	    if(!track_2) continue;
	    
	    track_2->GetPxPyPz(mom2);
	    // the charge sign of the triplet should be -+- or +-+
	    // exclude cases in which the 3rd and 2nd track have the same sign
	    if(track_2->Charge() * track_1->Charge() >0) continue; 
	    if (status[iTrack_2]==0) continue;
	    //	  printf("  Tracks: %d(%d) %d(%d) %d(%d)\n",iTrack_0,track_0->Charge(),iTrack_1,track_1->Charge(),iTrack_2,track_2->Charge());
	    threeTrackArray->AddAt(track_0, 0);
	    threeTrackArray->AddAt(track_1, 1);
	    threeTrackArray->AddAt(track_2, 2);
	    AliESDVertex* trkv3=ReconstructSecondaryVertex(vt,threeTrackArray,primvtx);
	    if(trkv3==0x0){
	      threeTrackArray->Clear();
	      continue;
	    }
	    AliAODVertex* vertexAOD3=ConvertToAODVertex(trkv3);
	    AliAODRecoDecayHF3Prong *the3Prong = Make3Prong(threeTrackArray,vertexAOD3,fBzkG);
	    //	  the3Prong->SetOwnPrimaryVtx(vertexAODp);
	    Double_t mp=the3Prong->InvMassDplus();
	    hmassP->Fill(mp);
	    delete trkv3;
	    delete the3Prong;
	    delete vertexAOD3;
	    threeTrackArray->Clear();
	  }
	}
	twoTrackArray->Clear();
      }
      //      delete vertexAODp;
    }
    delete [] status;
    delete vt;
    delete twoTrackArray;
    delete threeTrackArray;
  }
  delete esdTrackCuts;
  
  TFile* fout=new TFile(output.Data(),"recreate");
  hvx->Write();
  hvy->Write();
  hvz->Write();
  hpt_nocuts->Write();
  htgl_nocuts->Write();
  hpt_cuts->Write();
  htgl_cuts->Write();
  hitsmap->Write();

  hvertexx->Write();
  hvertexy->Write();
  hvertexz->Write();
  hdecayxyz->Write();
  hdecayxy->Write();
  hmass0->Write();
  hmassP->Write();

  fout->Close();
  return true; 
}
