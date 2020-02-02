Bool_t LoopTracks(){

  TString esdfile = "../inputESD/AliESDs_20200201_v0.root";
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

  TObjArray *trkArray    = new TObjArray(20);
  for (Int_t iEvent = 0; iEvent < tree->GetEntries(); iEvent++) {
    tree->GetEvent(iEvent);
    if (!esd) {
      printf("Error: no ESD object found for event %d", iEvent);
      return kFALSE;
    }
    cout<<"-------- Event "<<iEvent<<endl;
    printf(" Tracks # = %d\n",esd->GetNumberOfTracks());
    Int_t it=0;
    for (Int_t iTrack = 0; iTrack < esd->GetNumberOfTracks(); iTrack++) {
      AliESDtrack* track = esd->GetTrack(iTrack);
      hpt_nocuts->Fill(track->Pt());
    
    printf(" Tracks ITSrefit = %d\n",it);
    printf(" Entries in array = %d\n",trkArray->GetEntries());
    }
  } 
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

  TFile* fout=new TFile("../output/LFlooptracks.root","recreate");
  hpt_nocuts->Write();
  fout->Close();
  return true; 
}



