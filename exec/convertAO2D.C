R__ADD_INCLUDE_PATH($ALICE_ROOT)
R__ADD_INCLUDE_PATH($ALICE_PHYSICS)
#include <ANALYSIS/macros/train/AddESDHandler.C>
#include <ANALYSIS/macros/train/AddAODHandler.C>
#include <ANALYSIS/macros/train/AddMCHandler.C>
#include <OADB/COMMON/MULTIPLICITY/macros/AddTaskMultSelection.C>
#include <OADB/macros/AddTaskPhysicsSelection.C>
#include <ANALYSIS/macros/AddTaskPIDResponse.C>
#include <RUN3/AddTaskAO2Dconverter.C>

TChain* CreateLocalChain(const char* txtfile, const char* type, int nfiles);

Long64_t convertAO2D(TString listoffiles, bool isMC = 1, bool useAliEvCuts = false, bool isESD = 1, int nmaxevents = -1)
{
  const char* anatype = isESD ? "ESD" : "AOD";
  if (isMC) {
    std::cout << "I AM DOING MC" << std::endl;
  }

  //  TGrid::Connect("alien:");

  // Create the chain based on xml collection or txt file
  // The entries in the txt file can be local paths or alien paths
  TChain* chain = CreateLocalChain(listoffiles.Data(), anatype, -1);
  if (!chain)
    return -1;
  chain->SetNotify(0x0);
  ULong64_t nentries = chain->GetEntries();
  if (nmaxevents != -1)
    nentries = nmaxevents;
  cout << nentries << " entries in the chain." << endl;
  cout << nentries << " converted" << endl;
  AliAnalysisManager* mgr = new AliAnalysisManager("AOD converter");
  if (isESD) {
    AddESDHandler();
  } else {
    AddAODHandler();
  }

  AddTaskMultSelection();
  AddTaskPhysicsSelection(isMC);
  AddTaskPIDResponse(isMC);
  if (isMC && isESD)
    AliMCEventHandler* handlerMC = AddMCHandler();
  AliAnalysisTaskAO2Dconverter* converter = AddTaskAO2Dconverter("");
  //converter->SelectCollisionCandidates(AliVEvent::kAny);
  if (useAliEvCuts)
    converter->SetUseEventCuts(kTRUE);
  if (isMC)
    converter->SetMCMode();
  if (!mgr->InitAnalysis())
    return -1;
  //PH   mgr->SetBit(AliAnalysisManager::kTrueNotify);
  //mgr->SetRunFromPath(244918);
  mgr->PrintStatus();

  mgr->SetDebugLevel(1);
  return mgr->StartAnalysis("localfile", chain, nentries, 0);
}

TChain* CreateLocalChain(const char* txtfile, const char* type, int nfiles)
{
  TString treename = type;
  treename.ToLower();
  treename += "Tree";
  printf("***************************************\n");
  printf("    Getting chain of trees %s\n", treename.Data());
  printf("***************************************\n");
  // Open the file
  ifstream in;
  in.open(txtfile);
  Int_t count = 0;
  // Read the input list of files and add them to the chain
  TString line;
  TChain* chain = new TChain(treename);
  while (in.good()) {
    in >> line;
    if (line.IsNull() || line.BeginsWith("#"))
      continue;
    if (count++ == nfiles)
      break;
    TString esdFile(line);
    TFile* file = TFile::Open(esdFile);
    if (file && !file->IsZombie()) {
      chain->Add(esdFile);
      file->Close();
    } else {
      Error("GetChainforTestMode", "Skipping un-openable file: %s", esdFile.Data());
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
