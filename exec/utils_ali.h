// AliPhysics utilities

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
