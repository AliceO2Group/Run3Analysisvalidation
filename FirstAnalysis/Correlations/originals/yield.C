#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"

#include "minimumOldMethod.C"

typedef unsigned int uint;

enum FLAGS {
  FLAG_OLD_ZYAM = 0x1,
  FLAG_REMOVE_WING = 0x2,
  FLAG_SHIFT_INTEGRAL = 0x4
};

#define DECL_V0MCENT static Double_t Nct[] = {5.0e-02, 5.5e-01, 3.0e+00, 7.5e+00, 1.5e+01, 2.5e+01, 3.5e+01, 4.5e+01, 5.5e+01, 8.0e+01}; // cent bins for V0M
#define DECL_REFMULT08 static Double_t Nct[] = {2.5, 8.0, 13.0, 20.5, 30.5, 40.5, 50.5, 60.5, 70.5, 80.5, 90.5};                         //,98.0}; //bin1 is 2.5; bin0 is overflow
#define DECL_REFMULT08FINE static Double_t Nct[] = {1.750, 4.500, 6.500, 9.000, 11.500, 14.000, 18.000, 23.000, 28.000, 33.000, 38.000, 43.000, 48.000, 53.000, 58.000, 63.000, 68.000, 73.000, 78.000, 83.000, 88.000, 93.000};

#define RESULT_SET 0 // 0: default, 1: zbins, 2: pythia, 3: V0M cent

#if RESULT_SET == 0
DECL_REFMULT08
static TString inputPath = "corrProdBin1";
static TString outputPath = "resultsProd";
#elif RESULT_SET == 1
DECL_REFMULT08
static TString inputPath = "corrProdBin1ZBins";
static TString outputPath = "resultsProdzbins";
#elif RESULT_SET == 2
DECL_REFMULT08
static TString inputPath = "corrPythia";
static TString outputPath = "resultsPythia";
#elif RESULT_SET == 3
DECL_V0MCENT
static TString inputPath = "corrV0M";
static TString outputPath = "resultsV0M";
#elif RESULT_SET == 4
DECL_REFMULT08
static TString inputPath = "corrTRKM";
static TString outputPath = "resultsTRKM";
#endif

static const uint Nbins = sizeof(Nct) / sizeof(Nct[0]);

static TH1 *hSig, *hSub; // hack
double chi2(const double* par)
{
  double chi2 = 0.0;
  for (uint i = 0; i < hSig->GetNbinsX(); i++) {
    double delta = hSig->GetBinContent(i + 1) - hSub->GetBinContent(i + 1) * par[0] - (2.0 * par[2] * TMath::Cos(hSig->GetBinCenter(i + 1) * 2.0) + 2.0 * par[3] * TMath::Cos(hSig->GetBinCenter(i + 1) * 3.0) + 1.0) * par[1];
    double sigErr = hSig->GetBinError(i + 1);
    double subErrPar = hSub->GetBinError(i + 1) * par[0];
    chi2 += delta * delta / (sigErr * sigErr + subErrPar * subErrPar);
  }
  return chi2;
}

// hack: poor ROOT::Math::TFitResult implementation has no error checking
static bool errorAbort = false;
void ErrorHandler(int level, Bool_t abort, const char* loc, const char* msg)
{
  errorAbort = false;
  printf("%s: %s\n", loc, msg);
  if (strcmp(loc, "Fit") == 0 || strcmp(loc, "TFitResultPtr") == 0) {
    errorAbort = true;
  }
}

void processYield(const char* pinFileFname = "dphi_corr10.root", double absDeltaEtaMin = 1.4, double absDeltaEtaMax = 2.0, const char* poutFileName = "yield_12.root", uint flags = 0)
{
  TFile* pf = new TFile(pinFileFname, "read");
  TFile* pfout = new TFile(poutFileName, "recreate");
  pfout->cd();

  SetErrorHandler(ErrorHandler);
  // const double absDeltaEtaMin = 1.2; //1.6
  // const double absDeltaEtaMax = 2.0; //1.8
  const double absDeltaPhi = 1.3; // y-projection range

  const uint trigCount = 3;
  TH1D* pltf1[trigCount][trigCount][Nbins] = {0};

  for (uint itrig = 0; itrig < trigCount; ++itrig) {
    for (uint iassoc = 0; iassoc <= itrig; ++iassoc) {
      TGraphErrors* pridgeYield = new TGraphErrors(Nbins);
      TGraphErrors* pfarYield = new TGraphErrors(Nbins);
      TGraphErrors* pfragYield = new TGraphErrors(Nbins);
      for (uint ib = 0; ib < Nbins; ++ib) {
        TH2D* ph = (TH2D*)pf->Get(Form("dphi_%u_%u_%u", itrig, iassoc, ib));
        if (!ph) {
          printf("No histograms corresponding mult bin %u. (itrig=%u, iassoc=%u)\n", ib, itrig, iassoc);
          continue;
        }

        TH2D* phe = (TH2D*)ph->Clone();

        // double norm = ph->GetYaxis()->GetBinWidth(1);
        double norm = 2.0 * (absDeltaEtaMax - absDeltaEtaMin);
        ph->Scale(1.0 / norm);

        int a = phe->GetXaxis()->FindBin(TMath::Pi() - 1.5);
        int b = phe->GetXaxis()->FindBin(TMath::Pi() + 1.5);
        TH1D* phe0 = phe->ProjectionY(Form("proj_deta_%u_%u_%u", itrig, iassoc, ib), a, b, "e");
        phe0->Write();

        a = phe->GetXaxis()->FindBin(-1.5);
        b = phe->GetXaxis()->FindBin(+1.5);
        TH1D* phe0JetPeak = phe->ProjectionY(Form("proj_detaJetPeak_%u_%u_%u", itrig, iassoc, ib), a, b, "e");
        phe0JetPeak->Write();

        // DZYAM 1.
        norm = 2.0 * (absDeltaPhi);
        phe->Scale(1.0 / norm);

        // wing ----------------------------------------
        if (flags & FLAG_REMOVE_WING) {
          // project at phi
          int a = ph->GetXaxis()->FindBin(TMath::Pi() - 1.5);
          int b = ph->GetXaxis()->FindBin(TMath::Pi() + 1.5);
          TH1D* pe = ph->ProjectionY(Form("proj_deta_%u_%u_%u_scaler", itrig, iassoc, ib), a, b, "e");
          pe->Fit("pol0", "0QSE");
          pe->Divide(pe->GetFunction("pol0"));
          for (uint i = 1; i < ph->GetNbinsX(); ++i)
            for (uint j = 1; j < ph->GetNbinsY(); ++j) {
              double z = pe->GetBinContent(j);
              if (z <= 0.0)
                continue;
              ph->SetBinContent(i, j, ph->GetBinContent(i, j) / z);
              ph->SetBinError(i, j, ph->GetBinError(i, j) / z);
            }
          ph->Write();
        }

        // DZYAM 2.
        a = phe->GetXaxis()->FindBin(-absDeltaPhi);
        b = phe->GetXaxis()->FindBin(absDeltaPhi);
        TH1D* phe1 = phe->ProjectionY(Form("proj_deta_Dzyam_%u_%u_%u", itrig, iassoc, ib), a, b, "e");

        //---------------------------------------------
        a = ph->GetYaxis()->FindBin(absDeltaEtaMin);
        b = ph->GetYaxis()->FindBin(absDeltaEtaMax);
        TH1D* pp = ph->ProjectionX(Form("proj_dphi_P_%u_%u_%u", itrig, iassoc, ib), a, b, "e");

        a = ph->GetYaxis()->FindBin(-absDeltaEtaMax);
        b = ph->GetYaxis()->FindBin(-absDeltaEtaMin);
        TH1D* pn = ph->ProjectionX(Form("proj_dphi_N_%u_%u_%u", itrig, iassoc, ib), a, b, "e");

        TH1D* phf1 = (TH1D*)pp->Clone(Form("proj_dphi_%u_%u_%u", itrig, iassoc, ib));
        phf1->Add(pp, pn, 0.5, 0.5);
        pltf1[itrig][iassoc][ib] = (TH1D*)phf1->Clone();

        // method switch
        TF1* pfit = new TF1(Form("fit_%u_%u_%u", itrig, iassoc, ib), "[0]+[1]*(1+2*[2]*TMath::Cos(x)+2*[3]*TMath::Cos(2*x)+2*[4]*TMath::Cos(3*x))");
        pfit->SetParNames("czyam", "c", "v1", "v2", "v3");
        pfit->FixParameter(0, 0.0);
        TFitResultPtr r = phf1->Fit(pfit, "0QSE", "", -TMath::Pi() / 2.0, 3.0 / 2.0 * TMath::Pi());
        // if(!r.fPointer || r->IsEmpty() || !r->IsValid())
        //	continue;
        if (errorAbort) // || r->IsEmpty() || !r->IsValid())
          continue;

        // double phiMinX = pfit->GetMinimumX(-TMath::Pi()/2.0,3.0/2.0*TMath::Pi());
        double phiMinX = pfit->GetMinimumX(-TMath::Pi() / 2.0, 3.0 / 2.0 * TMath::Pi());
        double phiMin = pfit->Eval(phiMinX);
        double czyam = phiMin;
        /*int phiMinBin = phf1->GetMinimumBin();
        double phiMinX = phf1->GetBinCenter(phiMinBin);
        double phiMin = phf1->GetBinContent(phiMinBin);
        double czyam = phiMin;*/

        double fitErr;
        r->GetConfidenceIntervals(1, 1, 1, &phiMinX, &fitErr, 0.683, false);
        //
        if (flags & FLAG_OLD_ZYAM)
          ComputeMinimum(phf1, czyam, fitErr, false);

        for (uint i = 1; i < phf1->GetXaxis()->GetNbins() + 1; ++i) {
          double y = phf1->GetBinContent(i);
          double yerr = phf1->GetBinError(i);
          phf1->SetBinContent(i, TMath::Max(y - czyam, 0.0));
          phf1->SetBinError(i, TMath::Sqrt(yerr * yerr + fitErr * fitErr));
        }
        pfit->FixParameter(0, -czyam);

        double etaMin = phe1->GetMinimum(1e-5); // pfit->GetMinimumX(-TMath::Pi()/2.0,3.0/2.0*TMath::Pi());
        double dzyam = etaMin;
        // DZYAM 3.
        for (uint i = 1; i < phe1->GetXaxis()->GetNbins() + 1; ++i) {
          double y = phe1->GetBinContent(i);
          double yerr = phe1->GetBinError(i);
          phe1->SetBinContent(i, TMath::Max(y - dzyam, 0.0));
          // phe1->SetBinError(i,yerr);
        }

        int phiIntShift = (flags & FLAG_SHIFT_INTEGRAL ? 1 : 0);

        // near-side integration
        // Y_ridge^near
        a = phf1->GetXaxis()->FindBin(-TMath::Abs(phiMinX)) + phiIntShift;
        b = phf1->GetXaxis()->FindBin(TMath::Abs(phiMinX)) + phiIntShift;
        double YridgeErr;
        double Yridge = phf1->IntegralAndError(a, b, YridgeErr, "width");
        // printf("<N> = %.1lf, C_ZYAM = %.4f, Yridge = %.4lf pm %.6lf\n",Nct[ib],czyam,Yridge,YridgeErr);

        pridgeYield->SetPoint(ib, Nct[ib], Yridge);
        pridgeYield->SetPointError(ib, 0.0, YridgeErr);
        // printf("dNdeta = {}, Yridge = {}".format(dNdeta[ib+1],Yridge));

        // far-side integration
        // Y_frag^far
        double YfarErr1, YfarErr2;
        double Yfar = phf1->IntegralAndError(b, phf1->GetNbinsX(), YfarErr1, "width") + phf1->IntegralAndError(1, a, YfarErr2, "width");
        double YfarErr = TMath::Sqrt(YfarErr1 * YfarErr1 + YfarErr2 * YfarErr2);
        // double YfarErr = YfarErr1;
        pfarYield->SetPoint(ib, Nct[ib], Yfar);
        pfarYield->SetPointError(ib, 0.0, YfarErr);

        // near-side jet fragmentation yield
        // Y_frag^near
        a = phe1->GetXaxis()->FindBin(-1.3); //|deta| range < 1.3
        b = phe1->GetXaxis()->FindBin(+1.3);
        double YfragErr;
        double Yfrag = phe1->IntegralAndError(a, b, YfragErr, "width");

        pfragYield->SetPoint(ib, Nct[ib], Yfrag);
        pfragYield->SetPointError(ib, 0.0, YfragErr);

        // printf("ib=%u, far/near=%f\n",ib,Yfar/Yfrag);

        phf1->Write();
        phe1->Write();
        pfit->Write();

        delete pfit;
      }
      pridgeYield->Write(Form("ridgeYield_%u_%u", itrig, iassoc));
      pfarYield->Write(Form("farYield_%u_%u", itrig, iassoc));
      pfragYield->Write(Form("fragYield_%u_%u", itrig, iassoc));
      delete pridgeYield;
      delete pfarYield;
      delete pfragYield;
    }
  }

  ROOT::Math::Minimizer* pmin = ROOT::Math::Factory::CreateMinimizer("Minuit2", "");
  pmin->SetMaxFunctionCalls(10000000); // for Minuit/Minuit2
  pmin->SetMaxIterations(10000);       // for GSL
  pmin->SetTolerance(0.001);
  pmin->SetPrintLevel(1);

  ROOT::Math::Functor f(&chi2, 4);
  // double step[4] = {0.01,0.01,0.01,0.01};
  double params[4] = {2, 4, 0.02, 0.02};

  pmin->SetFunction(f);

  for (uint itrig = 0; itrig < trigCount; ++itrig) {
    for (uint iassoc = 0; iassoc <= itrig; ++iassoc) {
      if (!pltf1[itrig][iassoc][0]) {
        printf("No low multiplicity template (itrig=%u, iassoc=%u).\n", itrig, iassoc);
        continue;
      }
      TGraphErrors* pvn[2] = {
        new TGraphErrors(Nbins - 1),
        new TGraphErrors(Nbins - 1)};
      //--------- original template fit
      // first-bin results to be used for the "improved" TF
      double Fperi, FperiErr;
      double Gperi, GperiErr;
      double V2peri, V2periErr;
      double V3peri, V3periErr;
      for (uint ib = 1; ib < Nbins; ++ib) {
        if (!pltf1[itrig][iassoc][ib])
          continue;
        hSig = pltf1[itrig][iassoc][ib];
        hSub = pltf1[itrig][iassoc][0];

        pmin->SetVariable(0, "F", params[0], 0.01);
        pmin->SetVariable(1, "G", params[1], 0.01);
        pmin->SetVariable(2, "v2", params[2], 0.01);
        pmin->SetVariable(3, "v3", params[3], 0.01);

        pmin->SetPrintLevel(0);
        pmin->Minimize();

        if (ib <= 1) {
          Fperi = pmin->X()[0];
          FperiErr = pmin->Errors()[0];
          Gperi = pmin->X()[1];
          GperiErr = pmin->Errors()[1];
          V2peri = pmin->X()[2];
          V2periErr = pmin->Errors()[2];
          V3peri = pmin->X()[3];
          V3periErr = pmin->Errors()[3];
        }
        // printf("ib=%u: F=%f\n",ib,pmin->X()[0]);

        for (uint i = 0; i < 2; ++i) {
          double vn = TMath::Sqrt(pmin->X()[i + 2]);
          double vnerr = TMath::Abs(0.5 / vn * pmin->Errors()[i + 2]);
          pvn[i]->SetPoint(ib - 1, Nct[ib], vn);
          pvn[i]->SetPointError(ib - 1, 0, vnerr);
        }
      }
      pvn[0]->Write(Form("v2_%u_%u", itrig, iassoc));
      pvn[1]->Write(Form("v3_%u_%u", itrig, iassoc));
      //--------- fits for the correction (improved TF)
      // central bin fit with fixed low mult F
      for (uint ib = 2; ib < Nbins; ++ib) {
        if (!pltf1[itrig][iassoc][ib])
          continue;
        hSig = pltf1[itrig][iassoc][ib];
        hSub = pltf1[itrig][iassoc][1];

        pmin->SetVariable(0, "F", Fperi, 0.01);
        pmin->FixVariable(0);
        pmin->SetVariable(1, "G", params[1], 0.01);
        pmin->SetVariable(2, "v2", params[2], 0.01);
        pmin->SetVariable(3, "v3", params[3], 0.01);

        pmin->SetPrintLevel(0);
        pmin->Minimize();

        pmin->ReleaseVariable(0);

        double Vnc[2] = {
          pmin->X()[2] - Fperi * Gperi / (pmin->X()[1] - Fperi * Gperi) * (pmin->X()[2] - V2peri),
          pmin->X()[3] - Fperi * Gperi / (pmin->X()[1] - Fperi * Gperi) * (pmin->X()[3] - V3peri)};
        struct ErrC {
          static double C(double V, double Verr, double Vperi, double VperiErr, double G, double Fperi, double FperiErr, double Gerr, double Gperi, double GperiErr)
          {
            double e = pow(1 - (Fperi * Gperi) / (G + Fperi * Gperi), 2) * pow(Verr, 2) + pow(GperiErr, 2) * pow((pow(Fperi, 2) * Gperi * (V - Vperi)) / pow(G + Fperi * Gperi, 2) - (Fperi * (V - Vperi)) / (G + Fperi * Gperi), 2) + pow(FperiErr, 2) * pow((Fperi * pow(Gperi, 2) * (V - Vperi)) / pow(G + Fperi * Gperi, 2) - (Gperi * (V - Vperi)) / (G + Fperi * Gperi), 2) + (pow(Fperi, 2) * pow(Gerr, 2) * pow(Gperi, 2) * pow(V - Vperi, 2)) / pow(G + Fperi * Gperi, 4) + (pow(Fperi, 2) * pow(Gperi, 2) * pow(VperiErr, 2)) / pow(G + Fperi * Gperi, 2);
            return sqrt(e);
          };
        };
        double VncErr[2] = {
          ErrC::C(pmin->X()[2], pmin->Errors()[2], V2peri, V2periErr, Fperi, FperiErr, pmin->X()[1], pmin->Errors()[1], Gperi, GperiErr),
          ErrC::C(pmin->X()[3], pmin->Errors()[3], V3peri, V3periErr, Fperi, FperiErr, pmin->X()[1], pmin->Errors()[1], Gperi, GperiErr)};
        for (uint i = 0; i < 2; ++i) {
          double vn = TMath::Sqrt(Vnc[i]);
          double vnerr = TMath::Abs(0.5 / vn * VncErr[i]);
          pvn[i]->SetPoint(ib - 1, Nct[ib], vn);
          pvn[i]->SetPointError(ib - 1, 0, vnerr);
        }
      }
      pvn[0]->Write(Form("v2_ITF_%u_%u", itrig, iassoc));
      pvn[1]->Write(Form("v3_ITF_%u_%u", itrig, iassoc));

      //
      delete pvn[0];
      delete pvn[1];
    }
  }

  pf->Close();
  delete pf;
  pfout->Close();
  delete pfout;
}

void yield()
{
  struct {
    const char* psrcFile;
    const char* plabel;
    double deltaEtaMin;
    double deltaEtaMax;
    uint flags;
  } configs[] = {
#if RESULT_SET == 0 // default set of results - wagon run 1234 (RefMult08)
    {"dphi_corr_Prod.root", "default", 1.4, 2.0, 0},
    {"dphi_corr_Prod_zv10.root", "zvtx10", 1.4, 2.0, 0},
    {"dphi_corr_Prod_zv6.root", "zvtx6", 1.4, 2.0, 0},
    {"dphi_corr_Prod.root", "deta12", 1.2, 2.0, 0},
    {"dphi_corr_Prod.root", "deta16", 1.6, 2.0, 0},
    {"dphi_corr_Prod.root", "oldZYAM", 1.4, 2.0, FLAG_OLD_ZYAM},
    {"dphi_corr_Prod.root", "noWing", 1.4, 2.0, FLAG_REMOVE_WING},
    {"dphi_corr_Prod.root", "shiftInt", 1.4, 2.0, FLAG_SHIFT_INTEGRAL},
    {"dphi_corr_Prod_eta10.root", "eta10", 1.4, 2.0, 0},
    {"dphi_corr_Prod_eta10.root", "eta10_deta16", 1.6, 2.0, 0},
    {"dphi_corr_Prod_eta12.root", "eta12", 1.4, 2.0, 0},
    {"dphi_corr_Prod_eta12.root", "eta12_deta16", 1.6, 2.0, 0},
#elif RESULT_SET == 1 // narrow z-bins
    {"dphi_corr_Prod_zv-8to-6.root", "zvtxN86", 1.4, 2.0, 0},
    {"dphi_corr_Prod_zv-6to-4.root", "zvtxN64", 1.4, 2.0, 0},
    {"dphi_corr_Prod_zv-4to-2.root", "zvtxN42", 1.4, 2.0, 0},
    {"dphi_corr_Prod_zv-2to0.root", "zvtxN20", 1.4, 2.0, 0},
    {"dphi_corr_Prod_zv0to2.root", "zvtxP02", 1.4, 2.0, 0},
    {"dphi_corr_Prod_zv2to4.root", "zvtxP24", 1.4, 2.0, 0},
    {"dphi_corr_Prod_zv4to6.root", "zvtxP46", 1.4, 2.0, 0},
    {"dphi_corr_Prod_zv6to8.root", "zvtxP68", 1.4, 2.0, 0},
#elif RESULT_SET == 2 // pythia
    {"dphi_corr_PythiaShoving08.root", "pythiaShoving08", 1.4, 2.0, 0},
  //{"dphi_corr_PythiaShoving12.root","pythiaShoving12",1.4,2.0,0},
  //{"dphi_corr_PythiaSoft08.root","pythiaSoft08",1.4,2.0,0},
  //{"dphi_corr_PythiaSoft12.root","pythiaSoft12",1.4,2.0,0},
  //{"dphi_corr_PythiaSoft08Extend.root","pythiaSoft08Extend",1.4,2.0,0},
  //{"dphi_corr_RejTest.root","pythiaRejTest",1.4,2.0,0},
#elif RESULT_SET == 3 // V0M centrality (needs different binning)
    {"dphi_corr_V0M.root", "default", 1.4, 2.0, 0},
    {"dphi_corr_V0M_zv10.root", "zvtx10", 1.4, 2.0, 0},
    {"dphi_corr_V0M_zv6.root", "zvtx6", 1.4, 2.0, 0},
    {"dphi_corr_V0M_zv1.root", "zvtx1", 1.4, 2.0, 0},
    {"dphi_corr_V0M.root", "deta12", 1.2, 2.0, 0},
    {"dphi_corr_V0M.root", "deta16", 1.6, 2.0, 0},
    {"dphi_corr_V0M.root", "oldZYAM", 1.4, 2.0, FLAG_OLD_ZYAM},
    {"dphi_corr_V0M.root", "noWing", 1.4, 2.0, FLAG_REMOVE_WING},
    {"dphi_corr_V0M.root", "shiftInt", 1.4, 2.0, FLAG_SHIFT_INTEGRAL},
#elif RESULT_SET == 4 // tracks_manual (needs different binning)
    {"dphi_corr_TRKM.root", "default", 1.4, 2.0, 0},
#endif
  };
  for (auto& m : configs) {
    printf("processing %s...\n", m.plabel);
    processYield(Form("%s/%s", inputPath.Data(), m.psrcFile), m.deltaEtaMin, m.deltaEtaMax, Form("%s/yield_%s.root", outputPath.Data(), m.plabel), m.flags);
  }
}
