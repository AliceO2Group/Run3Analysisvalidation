#include "TMath.h"
#include <stdio.h>



void GetEmCalThicknessVsZ(const Double_t Rint=100, const Double_t ThicknessEta0=30, const Double_t etaMax=1.6 )
{
  const Int_t deltaZ=10;
  Double_t zMax=0;

  FILE *myfile = fopen ("EmCalThicknessVsZ.txt", "w");
  Double_t thickness=0;
  zMax=(Rint+ThicknessEta0)*(1-exp(-2*etaMax))/(2*exp(-etaMax))+deltaZ;
  for (Int_t z=0; z<zMax; z+=deltaZ)
  {
    thickness=sin(atan(Rint/(z*1.0)))*ThicknessEta0;
    fprintf(myfile,"%6d  %3d\n",z, TMath::Nint(thickness));
    if (z){fprintf(myfile, "%6d  %3d\n",-z, TMath::Nint(thickness));}
  }
  fclose(myfile);
}

