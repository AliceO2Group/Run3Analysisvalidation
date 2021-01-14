Bool_t ComputeVerticesRun1()
{

  TString esdfile = "../inputESD/AliESDs_20200201_v0.root";
  TFile* esdFile = TFile::Open(esdfile.Data());
  if (!esdFile || !esdFile->IsOpen()) {
    printf("Error in opening ESD file");
    return kFALSE;
  }

  AliESDEvent* esd = new AliESDEvent;
  TTree* tree = (TTree*)esdFile->Get("esdTree");
  if (!tree) {
    printf("Error: no ESD tree found");
    return kFALSE;
  }
  esd->ReadFromTree(tree);

  TH1F* hpt_nocuts = new TH1F("hpt_nocuts", " ; pt tracks (#GeV) ; Entries", 100, 0, 10.);
  TH1F* htgl_nocuts = new TH1F("htgl_nocuts", "tgl tracks (#GeV)", 100, 0., 10.);
  TH1F* hpt_cuts = new TH1F("hpt_cuts", " ; pt tracks (#GeV) ; Entries", 100, 0, 10.);
  TH1F* htgl_cuts = new TH1F("htgl_cuts", "tgl tracks (#GeV)", 100, 0., 10.);
  TObjArray* trkArray = new TObjArray(20);
  for (Int_t iEvent = 0; iEvent < tree->GetEntries(); iEvent++) {
    tree->GetEvent(iEvent);
    if (!esd) {
      printf("Error: no ESD object found for event %d", iEvent);
      return kFALSE;
    }
    cout << "-------- Event " << iEvent << endl;
    printf(" Tracks # = %d\n", esd->GetNumberOfTracks());
    Double_t fBzkG = (Double_t)esd->GetMagneticField();
    Int_t it = 0;
    for (Int_t iTrack = 0; iTrack < esd->GetNumberOfTracks(); iTrack++) {
      AliESDtrack* track = esd->GetTrack(iTrack);
      Int_t status = track->GetStatus();
      hpt_nocuts->Fill(track->Pt());
      htgl_nocuts->Fill(track->GetTgl());
      if (status & AliESDtrack::kITSrefit && (track->HasPointOnITSLayer(0) || track->HasPointOnITSLayer(1)) && track->GetNcls(1) > 70) {
        trkArray->AddAt(track, it++);
        hpt_cuts->Fill(track->Pt());
        htgl_cuts->Fill(track->GetTgl());
      }
    }
    printf(" Tracks ITSrefit = %d\n", it);
    printf(" Entries in array = %d\n", trkArray->GetEntries());
    trkArray->Clear();
  }
  delete trkArray;

  TFile* fout = new TFile("QA-ITS1.root", "recreate");
  hpt_nocuts->Write();
  htgl_nocuts->Write();
  hpt_cuts->Write();
  htgl_cuts->Write();
  fout->Close();
  return true;
}
