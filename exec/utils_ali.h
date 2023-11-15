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

// AliPhysics utilities

#ifndef EXEC_UTILS_ALI_H_
#define EXEC_UTILS_ALI_H_

TChain* CreateLocalChain(const char* txtfile, const char* type = "esd", int nfiles = -1)
{
  TString treename = type;
  treename.ToLower();
  treename += "Tree";
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

#endif // EXEC_UTILS_ALI_H_
