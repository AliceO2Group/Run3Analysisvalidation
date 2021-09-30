TChain* CreateLocalChain(const char* txtfile);

Long64_t RunHFTaskLocal(TString txtfile = "./list_ali.txt",
                        TString jsonfilename = "dpl-config_std.json",
                        Bool_t isMC = kFALSE,
                        Bool_t useO2Vertexer = kFALSE)
{
  // Load common libraries
  gSystem->Load("libCore.so");
  gSystem->Load("libTree.so");
  gSystem->Load("libGeom.so");
  gSystem->Load("libVMC.so");
  gSystem->Load("libPhysics.so");
  gSystem->Load("libSTEERBase");
  gSystem->Load("libESD");
  gSystem->Load("libAOD");
  gSystem->Load("libANALYSIS");
  gSystem->Load("libANALYSISalice");
  gSystem->SetIncludePath("-I. -I$ROOTSYS/include -I$ALICE_ROOT -I$ALICE_ROOT/include -I$ALICE_ROOT/ITS -I$ALICE_ROOT/TPC -I$ALICE_ROOT/CONTAINERS -I$ALICE_ROOT/STEER/STEER -I$ALICE_ROOT/STEER/STEERBase -I$ALICE_ROOT/STEER/ESD -I$ALICE_ROOT/STEER/CDB -I$ALICE_ROOT/TRD -I$ALICE_ROOT/macros -I$ALICE_ROOT/ANALYSIS -I$ALICE_PHYSICS -I$ALICE_PHYSICS/include -g");

  AliAnalysisManager* mgr = new AliAnalysisManager("testAnalysis");

  TChain* chainESD = CreateLocalChain(txtfile.Data());
  if (!chainESD) {
    Error("CreateLocalChain", "Failed to create chain from file %s", txtfile.Data());
    return -1;
  }

  // Create and configure the alien handler plugin
  AliESDInputHandler* esdH = new AliESDInputHandler();
  //  esdH->SetNeedField(kTRUE);
  mgr->SetInputEventHandler(esdH);

  AliMCEventHandler* handler = new AliMCEventHandler;
  handler->SetReadTR(kFALSE);
  mgr->SetMCtruthEventHandler(handler);

  // Apply the event selection
  AliPhysicsSelectionTask* physSelTask = reinterpret_cast<AliPhysicsSelectionTask*>(gInterpreter->ProcessLine(Form(".x %s(%d)", gSystem->ExpandPathName("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C"), isMC)));

  AliAnalysisTaskHFSimpleVertices* tasktr3 = reinterpret_cast<AliAnalysisTaskHFSimpleVertices*>(gInterpreter->ProcessLine(Form(".x %s(\"\",\"%s\",%d)", gSystem->ExpandPathName("$ALICE_PHYSICS/PWGHF/vertexingHF/macros/AddTaskHFSimpleVertices.C"), jsonfilename.Data(), isMC)));
  if (useO2Vertexer) {
    tasktr3->SetUseO2Vertexer();
  }

  mgr->InitAnalysis();
  mgr->PrintStatus();
  return mgr->StartAnalysis("local", chainESD);
};

TChain* CreateLocalChain(const char* txtfile)
{
  // Open the file
  ifstream in;
  in.open(txtfile);
  Int_t count = 0;
  // Read the input list of files and add them to the chain
  TString line;
  TChain* chain = new TChain("esdTree");
  while (in.good()) {
    in >> line;
    if (line.IsNull() || line.BeginsWith("#"))
      continue;
    TString esdFile(line);
    TFile* file = TFile::Open(esdFile);
    if (file && !file->IsZombie()) {
      chain->Add(esdFile);
      file->Close();
    } else {
      Error("CreateLocalChain", "Skipping un-openable file: %s", esdFile.Data());
    }
  }
  in.close();
  if (!chain->GetListOfFiles()->GetEntries()) {
    Error("CreateLocalChain", "No file from %s could be opened", txtfile);
    delete chain;
    return nullptr;
  }
  return chain;
}
