Bool_t ComputeVerticesRun1(TString esdfile = "../inputESD/AliESDs_20200201_v0.root", TString output = "Vertices2prong-ITS1.root"){

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
  TH1F* hvx=new TH1F("hvx"," ; X vertex (#mum) ; Entries",100,-0.1, 0.1);
  TH1F* hvy=new TH1F("hvy"," ; Y vertex (#mum) ; Entries",100,-0.1, 0.1);
  TH1F* hvz=new TH1F("hvz"," ; Z vertex (#mum) ; Entries",100,-0.1, 0.1);
  TH1F* hitsmap=new TH1F("hitsmap", "hitsmap_cuts", 100, 0., 100.);
  TObjArray *trkArray    = new TObjArray(20);
  for (Int_t iEvent = 0; iEvent < tree->GetEntries(); iEvent++) {
    tree->GetEvent(iEvent);
    if (!esd) {
      printf("Error: no ESD object found for event %d", iEvent);
      return kFALSE;
    }
    cout<<"-------- Event "<<iEvent<<endl;
    printf(" Tracks # = %d\n",esd->GetNumberOfTracks());
    Double_t fBzkG = (Double_t)esd->GetMagneticField();
    AliVertexerTracks* vt=new AliVertexerTracks(fBzkG);
    Int_t it=0;
    for (Int_t iTrack = 0; iTrack < esd->GetNumberOfTracks(); iTrack++) {
      AliESDtrack* track = esd->GetTrack(iTrack);
      Int_t status=track->GetStatus();
      hpt_nocuts->Fill(track->Pt());
      htgl_nocuts->Fill(track->GetTgl()); 
      hitsmap->Fill(track->GetITSClusterMap());
      if(status & AliESDtrack::kITSrefit && (track->HasPointOnITSLayer(0) || track->HasPointOnITSLayer(1)) && track->GetNcls(1)>70){
	trkArray->AddAt(track,it++);
        hpt_cuts->Fill(track->Pt());
        htgl_cuts->Fill(track->GetTgl()); 
      }
    }
    printf(" Tracks ITSrefit = %d\n",it);
    printf(" Entries in array = %d\n",trkArray->GetEntries());
    if(it==2){
      AliESDVertex* trkv=(AliESDVertex*)vt->VertexForSelectedESDTracks(trkArray);
      if(trkv->GetNContributors()==2){
	hvx->Fill(trkv->GetX());
	hvy->Fill(trkv->GetY());
	hvz->Fill(trkv->GetZ());
      }
    }
    delete vt;
    trkArray->Clear();
  }
  delete trkArray;
 
  /* 
  TCanvas* cv=new TCanvas("cv","Vertex",1600,700);
  cv->Divide(3,1);
  cv->cd(1);
  hvx->Draw();
  cv->cd(2);
  hvy->Draw();
  cv->cd(3);
  hvz->Draw();
  */

  TFile* fout=new TFile(output.Data(),"recreate");
  hvx->Write();
  hvy->Write();
  hvz->Write();
  hpt_nocuts->Write();
  htgl_nocuts->Write();
  hpt_cuts->Write();
  htgl_cuts->Write();
  hitsmap->Write();
  fout->Close();
  return true; 
}



