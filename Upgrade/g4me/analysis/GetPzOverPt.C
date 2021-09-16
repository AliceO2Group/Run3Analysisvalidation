//converts eta into pzOverPt

void GetPzOverPt()
{
  const Double_t etaMin=-1.60;
  const Double_t etaMax=1.60;
  const Double_t step=0.10;
  Int_t nbEta = (etaMax-etaMin)/step + 1;
  TLorentzVector convertor;
  Double_t pzOverpt=0, eta=0;

  FILE *myfile = fopen ("File_pzOverpt.txt", "w");
  for (int i=0; i<nbEta; i++)
  {
    eta=etaMin+i*step;
    convertor.SetPtEtaPhiM(1,eta,0,1);
    pzOverpt=convertor.Pz()/convertor.Px();
    fprintf(myfile,"%4.2f %4.2f\n",eta, pzOverpt);

  }
  fclose(myfile);
}
