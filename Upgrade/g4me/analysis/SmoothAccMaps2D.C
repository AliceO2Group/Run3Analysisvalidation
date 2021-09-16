#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TStyle.h"

enum part_t{kMIDElectron, kMIDMuon, kMIDPion, kMIDKaon, kMIDProton, kNPartTypes};
const char* partName[kNPartTypes] = {"electron", "muon", "pion", "kaon", "proton"};

TFile *fileIn = 0;

TH2D *accEffMuonPID_2D[kNPartTypes]={0}, *accEffMuonPID_2D_Param[kNPartTypes]={0};
TH1D *hTmp = 0;
TF1 *fitFunction = 0;

TCanvas *cnv[kNPartTypes]={0};

double GausPlusConstant(double *var, double *par);

//====================================================================================================================================================

void SmoothAccMaps2D(const char *nameInputFile) {

  gStyle -> SetOptStat(0);

  fileIn = new TFile(nameInputFile);

  fitFunction = new TF1("fitFunction",GausPlusConstant,-2,2,4);

  double offset, eta;

  for (int iPart=0; iPart<kNPartTypes; iPart++) {

    accEffMuonPID_2D[iPart]       = (TH2D*) fileIn->Get(Form("accEffMuonPID_2D_%s",partName[iPart]));
    accEffMuonPID_2D_Param[iPart] = (TH2D*) accEffMuonPID_2D[iPart] -> Clone(Form("accEffMuonPID_2D_Param_%s",partName[iPart]));
    accEffMuonPID_2D_Param[iPart] -> Smooth(1);

    for (int iBinMom=1; iBinMom<=accEffMuonPID_2D_Param[iPart]->GetNbinsY(); iBinMom++) {
      
      hTmp = accEffMuonPID_2D_Param[iPart]->ProjectionX("hTmp",iBinMom,iBinMom);

      offset = 0.5*(hTmp->GetBinContent(1)+hTmp->GetBinContent(hTmp->GetNbinsX()));
      
      fitFunction -> SetParameter(0, offset);
      fitFunction -> SetParameter(1, hTmp->GetBinContent(hTmp->GetBinContent(hTmp->GetNbinsX()/2)) - offset);
      fitFunction -> FixParameter(2, 0);
      fitFunction -> SetParameter(3, 0.4);

      hTmp -> Fit(fitFunction,"Q","",-1.6, 1.6);

      for (int iBinEta=1; iBinEta<=accEffMuonPID_2D_Param[iPart]->GetNbinsX(); iBinEta++) {
	
	eta = accEffMuonPID_2D_Param[iPart]->GetXaxis()->GetBinCenter(iBinEta);
	accEffMuonPID_2D_Param[iPart] -> SetBinContent(iBinEta,iBinMom,fitFunction->Eval(eta));

      }
						       
    }


  }

  for (int iPart=0; iPart<kNPartTypes; iPart++) {
  
    cnv[iPart] = new TCanvas(Form("cnv_%ss",partName[iPart]),Form("cnv_%ss",partName[iPart]),1200,600);
    cnv[iPart] -> Divide(2,1);

    cnv[iPart] -> cd(1);
    gPad -> SetLogy();
    accEffMuonPID_2D[iPart] -> GetZaxis() -> SetRangeUser(0, 0.05);
    accEffMuonPID_2D[iPart] -> Draw("colz");
    cnv[iPart] -> cd(2);
    gPad -> SetLogy();
    accEffMuonPID_2D_Param[iPart] -> GetZaxis() -> SetRangeUser(0, 0.05);
    accEffMuonPID_2D_Param[iPart] -> Draw("colz");

  }
    
}

//====================================================================================================================================================

double GausPlusConstant(double *var, double *par) {

  return par[0] + par[1] * TMath::Gaus(var[0], par[2], par[3]);

}

//====================================================================================================================================================
