// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "../exec/utilitiesAli.h"

#ifdef __CLING__
// Tell ROOT where to find AliRoot headers
R__ADD_INCLUDE_PATH($ALICE_ROOT)
// Tell ROOT where to find AliPhysics headers
R__ADD_INCLUDE_PATH($ALICE_PHYSICS)

#include "PWGPP/PilotTrain/AddTaskCDBconnect.C"

#endif

Long64_t RunJetTaskLocal(TString txtfile = "./list_ali.txt",
                         TString jsonfilename = "dpl-config_std.json",
                         Bool_t isMC = kFALSE,
                         Bool_t useO2Vertexer = kFALSE,
                         Bool_t useAliEventCuts = kFALSE)
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
    Fatal("CreateLocalChain", "Failed to create chain from file %s", txtfile.Data());
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

  // CDBconnect task
  AliTaskCDBconnect* taskCDB = AddTaskCDBconnect();
  taskCDB->SetFallBackToRaw(kTRUE);

  // Apply the event selection
  AliPhysicsSelectionTask* physSelTask = reinterpret_cast<AliPhysicsSelectionTask*>(gInterpreter->ProcessLine(Form(".x %s(%d)", gSystem->ExpandPathName("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C"), isMC)));

  AliAnalysisTaskEmcalJetValidation* taskJet = reinterpret_cast<AliAnalysisTaskEmcalJetValidation*>(gInterpreter->ProcessLine(Form(".x %s(\"\",\"%s\",%d)", gSystem->ExpandPathName("$ALICE_PHYSICS/PWGJE/EMCALJetTasks/macros/AddTaskEmcalJetValidation.C"), jsonfilename.Data(), isMC)));
  if (useAliEventCuts) {
    taskJet->SetUseAliEventCuts(useAliEventCuts);
  }
  // if (useO2Vertexer) {
  //   taskJet->SetUseO2Vertexer();
  // }

  mgr->InitAnalysis();
  mgr->PrintStatus();
  return mgr->StartAnalysis("local", chainESD);
};
