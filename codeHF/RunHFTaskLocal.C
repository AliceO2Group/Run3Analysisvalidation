#include "../exec/utils_ali.h"

Long64_t RunHFTaskLocal(TString txtfile = "./list_ali.txt",
                        TString jsonfilename = "dpl-config_std.json",
                        Bool_t isMC = kFALSE,
                        Bool_t useO2Vertexer = kFALSE,
                        Bool_t useAliEventCuts = kFALSE,
                        Bool_t doJets = kFALSE,
                        Bool_t doJetMatching = kFALSE,
                        Bool_t doJetSubstructure = kFALSE)
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

  if (isMC) {
    AliMCEventHandler* handler = new AliMCEventHandler();
    handler->SetReadTR(kFALSE);
    mgr->SetMCtruthEventHandler(handler);
  }

  // Apply the event selection
  AliPhysicsSelectionTask* physSelTask = reinterpret_cast<AliPhysicsSelectionTask*>(gInterpreter->ProcessLine(Form(".x %s(%d)", gSystem->ExpandPathName("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C"), isMC)));

  AliAnalysisTaskHFSimpleVertices* tasktr3 = reinterpret_cast<AliAnalysisTaskHFSimpleVertices*>(gInterpreter->ProcessLine(Form(".x %s(\"\",\"%s\",%d)", gSystem->ExpandPathName("$ALICE_PHYSICS/PWGHF/vertexingHF/macros/AddTaskHFSimpleVertices.C"), jsonfilename.Data(), isMC)));
  if (useAliEventCuts) {
    tasktr3->SetUseAliEventCuts(useAliEventCuts);
  }
  if (useO2Vertexer) {
    tasktr3->SetUseO2Vertexer();
  }
  tasktr3->SetUseCandidateAnalysisCuts();
  tasktr3->SetDoJetFinding(doJets);
  tasktr3->SetJetMatching(doJetMatching);
  tasktr3->SetDoJetSubstructure(doJetSubstructure);

  mgr->InitAnalysis();
  mgr->PrintStatus();
  return mgr->StartAnalysis("local", chainESD);
};
