#include "TMath.h"
#include <stdio.h>
#include "TGraphErrors.h"

const Int_t nbEta=6;

void GetAbsoThicknessVsZ(const Double_t Rint=162, const Double_t ThicknessEta0=70, const Double_t etaMax=1.6 )
{
  const Int_t deltaZ=20;
  const Int_t NbCyl=4;
  Double_t zMax=0;

  const Double_t eta[nbEta]={0.0,0.5,1.0,1.2,1.4,1.5};
  const Double_t expThick[nbEta]={70,65,65,60,60,50};
  Double_t zVsEta[nbEta]={0};
  TGraphErrors *graphThicknessVsZ = new TGraphErrors(nbEta);

  graphThicknessVsZ->SetName("tgraph");
  graphThicknessVsZ->SetTitle("Ideal thickness of the absorber as a function of z");

  FILE *myfile = fopen ("AbsoThicknessVsZ.txt", "w");

  Int_t thickness=0;
  zMax=(Rint+ThicknessEta0)*(1-exp(-2*etaMax))/(2*exp(-etaMax))+deltaZ;
  Int_t zCenter[NbCyl]={0, 120, 260, (320+int(zMax))/2};
  Int_t halfLength[NbCyl]={40, 80, 60, (int(zMax)-320)/2};
  printf("%d\n", int(zMax));

  const Int_t cylThick[NbCyl]={70,65,60,50};

  for (int i=0; i<nbEta; i++)
  {
    zVsEta[i]=Rint*(exp(eta[i])-exp(-eta[i]))/2;
    graphThicknessVsZ->SetPoint (i, zVsEta[i], expThick[i]);
    graphThicknessVsZ->SetPointError (i, 0, 5);

  }



  for (Int_t j=0; j<NbCyl; j++)
  {
    fprintf(myfile,"%6d  %3d %6d\n",zCenter[j], cylThick[j], halfLength[j]);
    if (j){fprintf(myfile, "%6d  %3d %6d\n",-zCenter[j], cylThick[j], halfLength[j]);}
  }
  fclose(myfile);

  /*graphThicknessVsZ->GetXaxis()->SetTitle("z (cm)");
  graphThicknessVsZ->GetYaxis()->SetTitle("Abso thickness (cm)");
  graphThicknessVsZ->SetMarkerStyle(21);
  graphThicknessVsZ->Draw("apl");*/
  //s->Draw("same");


}
