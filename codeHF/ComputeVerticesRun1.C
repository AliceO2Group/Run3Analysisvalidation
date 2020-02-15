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
  // FIXME if (!trk->PropagateToDCA(fV1,fBzkG,kVeryBig)) return kFALSE;
  // FIXME trk->RelateToVertex(fV1,fBzkG,kVeryBig);
  return esdTrackCuts->AcceptTrack(trk);
}

Bool_t ComputeVerticesRun1(TString esdfile = "../inputESD/AliESDs_20200201_v0.root",
		TString output = "Vertices2prong-ITS1.root", bool applyprimaryvtxcut = 0, 
		bool applytrackcut = 1, bool applysecvertexcut = 0){

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
  TH1F* hvx=new TH1F("hvx"," ; X vertex (cm) ; Entries",100,-0.1, 0.1);
  TH1F* hvy=new TH1F("hvy"," ; Y vertex (cm) ; Entries",100,-0.1, 0.1);
  TH1F* hvz=new TH1F("hvz"," ; Z vertex (cm) ; Entries",100,-0.1, 0.1);
  TH1F* hitsmap_cuts=new TH1F("hitsmap_cuts", "hitsmap_cuts", 100, 0., 100.);
  
  TH1F* hvertexx=new TH1F("hvertexx", "hvertexx", 100, -10., 10.);
  TH1F* hvertexy=new TH1F("hvertexy", "hvertexy", 100, -10., 10.);
  TH1F* hvertexz=new TH1F("hvertexz", "hvertexz", 100, -10., 10.);
  
  TH1F* hdecayxyz=new TH1F("hdecayxyz", "hdecayxyz", 100, 0., 1.0);
  TH1F* hdecayxy=new TH1F("hdecayxy", "hdecayxy", 100, 0., 1.0);
  TH1F* hmass_nocuts=new TH1F("hmass_nocuts", "; Inv Mass (GeV/c^{2})", 500, 0, 5.0);
  
  AliESDtrackCuts *esdTrackCuts = new AliESDtrackCuts("AliESDtrackCuts","default");
  esdTrackCuts->SetMinNClustersTPC(70);
  esdTrackCuts->SetRequireITSRefit(kTRUE);
  esdTrackCuts->SetClusterRequirementITS(AliESDtrackCuts::kSPD,
					 AliESDtrackCuts::kAny);
  //FIXME esdTrackCuts->SetAcceptKinkDaughters(kFALSE);
  //FIXME esdTrackCuts->SetMaxDCAToVertexZ(3.2);
  //FIXME esdTrackCuts->SetMaxDCAToVertexXY(2.4);
  //FIXME esdTrackCuts->SetDCAToVertex2D(kTRUE);

  for (Int_t iEvent = 0; iEvent < tree->GetEntries(); iEvent++) {
    tree->GetEvent(iEvent);
    if (!esd) {
      printf("Error: no ESD object found for event %d", iEvent);
      return kFALSE;
    }
    printf("\n------------ Event: %d  Tracks %d ------------------\n",iEvent,esd->GetNumberOfTracks());

    AliESDVertex *primvtx = (AliESDVertex*)esd->GetPrimaryVertex();
    if(applyprimaryvtxcut == 1){
      if(!primvtx) return kFALSE;
      TString title=primvtx->GetTitle();
      if(primvtx->IsFromVertexer3D() || primvtx->IsFromVertexerZ()) continue;
      if(primvtx->GetNContributors()<2) continue;
    }
    
    hvertexx->Fill(primvtx->GetX());
    hvertexy->Fill(primvtx->GetY());
    hvertexz->Fill(primvtx->GetZ());
    
    Double_t posp_[3],covp_[6],chi2perNDFp_;
    primvtx->GetXYZ(posp_); // position
    primvtx->GetCovMatrix(covp_); //covariance matrix
    float chi2perNDF_ = primvtx->GetChi2toNDF();
    double dispersionp_ = primvtx->GetDispersion();
    AliAODVertex *vertexAODp = 0;
    vertexAODp = new AliAODVertex(posp_,covp_,chi2perNDFp_);	
     
    
    
    Double_t fBzkG = (Double_t)esd->GetMagneticField();
    AliVertexerTracks* vt=new AliVertexerTracks(fBzkG);
    vt->SetVtxStart(primvtx);
    
    Double_t mom0[3], mom1[3];
    float dcap1n1;
    Double_t xdummy,ydummy;
    for (Int_t iTrack_0 = 0; iTrack_0 < esd->GetNumberOfTracks(); iTrack_0++) {
      AliESDtrack* track_0 = esd->GetTrack(iTrack_0);
      track_0->GetPxPyPz(mom0);
      hpt_nocuts->Fill(track_0->Pt());
      htgl_nocuts->Fill(track_0->GetTgl()); 
      //FIXME if (applytrackcut==1 && !SingleTrkCuts(track_0,esdTrackCuts,primvtx,fBzkG)) continue;
      Int_t status_0=track_0->GetStatus();
      bool sel_track0 = status_0 & AliESDtrack::kITSrefit && (track_0->HasPointOnITSLayer(0) || track_0->HasPointOnITSLayer(1)) && track_0->GetNcls(1)>70;
      if (applytrackcut==1 &&!sel_track0) continue;
      hpt_cuts->Fill(track_0->Pt());
      htgl_cuts->Fill(track_0->GetTgl()); 
      hitsmap_cuts->Fill(track_0->GetITSClusterMap());

      for (Int_t iTrack_1 = iTrack_0 + 1; iTrack_1 < esd->GetNumberOfTracks(); iTrack_1++) {
        AliESDtrack* track_1 = esd->GetTrack(iTrack_1);
        track_1->GetPxPyPz(mom1);
	if(track_1->Charge() * track_0->Charge() >0) continue;
	//FIXME if (applytrackcut==1 &&!SingleTrkCuts(track_1,esdTrackCuts,primvtx,fBzkG)) continue;
        Int_t status_1=track_1->GetStatus();
        bool sel_track1 = status_1 & AliESDtrack::kITSrefit && (track_1->HasPointOnITSLayer(0) || track_1->HasPointOnITSLayer(1)) && track_1->GetNcls(1)>70;
        if (applytrackcut==1 &&!sel_track1) continue;

        TObjArray *twoTrackArray = new TObjArray(2);
        twoTrackArray->AddAt(track_0, 0);
        twoTrackArray->AddAt(track_1, 1);
	AliESDVertex* trkv=(AliESDVertex*)vt->VertexForSelectedESDTracks(twoTrackArray);
	if(trkv->GetNContributors()!=twoTrackArray->GetEntriesFast()) continue;
	Double_t vertRadius2=trkv->GetX()*trkv->GetX()+trkv->GetY()*trkv->GetY();
	if (applysecvertexcut){
	   if(vertRadius2>8.) continue; //FIXME
        }
	printf(" px track_0 %.4f, track_1 %.4f \n", TMath::Max(track_0->Px(),track_1->Px()),TMath::Min(track_0->Px(),track_1->Px())); 
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
		
	Double_t pos_[3],cov_[6],chi2perNDF_;
        trkv->GetXYZ(pos_); // position
        trkv->GetCovMatrix(cov_); //covariance matrix
        chi2perNDF_ = trkv->GetChi2toNDF();
        double dispersion_ = trkv->GetDispersion();
	printf(" pos_ %f %f %f \n", pos_[0], pos_[1], pos_[2]);
        AliAODVertex *vertexAOD = 0;
	vertexAOD = new AliAODVertex(pos_,cov_,chi2perNDF_,0x0,-1,AliAODVertex::kUndef,2);	
        float dcap1n1 = track_0->GetDCA(track_1,fBzkG,xdummy,ydummy);
        Double_t px[2],py[2],pz[2],d0[2],d0err[2];
	Double_t momentum[3];
        GetTrackMomentumAtSecVert(track_0,vertexAOD,momentum,fBzkG);
        px[0] = momentum[0]; py[0] = momentum[1]; pz[0] = momentum[2];
        GetTrackMomentumAtSecVert(track_1,vertexAOD,momentum,fBzkG);
        px[1] = momentum[0]; py[1] = momentum[1]; pz[1] = momentum[2];

        Float_t d0z0f[2],covd0z0f[3];
        track_0->GetImpactParameters(d0z0f,covd0z0f);
        d0[0]=d0z0f[0];
        d0err[0] = TMath::Sqrt(covd0z0f[0]);
        track_1->GetImpactParameters(d0z0f,covd0z0f);
        d0[1]=d0z0f[0];
        d0err[1] = TMath::Sqrt(covd0z0f[0]);
        AliAODRecoDecayHF2Prong *the2Prong;
        the2Prong = new AliAODRecoDecayHF2Prong(vertexAOD,px,py,pz,d0,d0err,dcap1n1);
        the2Prong->SetOwnPrimaryVtx(vertexAODp);
	Double_t m0=the2Prong->InvMassD0();
	Double_t m0b=the2Prong->InvMassD0bar();
        hmass_nocuts->Fill(m0);
        hmass_nocuts->Fill(m0b);
	delete twoTrackArray;
      }
    }
    delete vt;
  }
 
  TFile* fout=new TFile(output.Data(),"recreate");
  hvx->Write();
  hvy->Write();
  hvz->Write();
  hpt_nocuts->Write();
  htgl_nocuts->Write();
  hpt_cuts->Write();
  htgl_cuts->Write();
  hitsmap_cuts->Write();

  hvertexx->Write();
  hvertexy->Write();
  hvertexz->Write();
  hdecayxyz->Write();
  hdecayxy->Write();
  hmass_nocuts->Write();

  fout->Close();
  return true; 
}
