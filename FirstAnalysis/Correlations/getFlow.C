//  this macro calculates final flow based on V_ndelta results
//  obtained from template fits in doTemplate() macro

int nBinspTtrig = 6;
double binspTtrig[] = {0.2, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0};
int nBinspTref = 1;
double binspTref[] = {0.2, 3.0};
const int nBinsMult = 7;
double binsMult[] = {0, 10, 20, 30, 40, 50, 60, 80, 100, 200};

TH1D *hDifferentialV2[nBinsMult];
TH1D *hDiffFlow_v2[nBinsMult];

void DrawHist(TH1D* h, Style_t marker, Color_t color, const char* draw);

void getFlow(const char* inputFileName = "./templateResult.root", const char* outputFileName = "./flow.root", const char* outputPlotsName = "./plots", bool savePlots = true)
{

  TFile *inFile = TFile::Open(Form("%s", inputFileName), "read");

  //  do reference flow
  TH1D *hReferenceV2 = (TH1D*)inFile->Get("hReferenceV2");

  TH1D *hRefFlow_v2 = new TH1D("hRefFlow_v2", "reference flow; N_{ch}; v_{2}", nBinsMult, binsMult);

  for (int iMult = 1; iMult < nBinsMult; iMult++) {
    double Vdelta2 = hReferenceV2->GetBinContent(iMult+1);
    double Vdelta2err = hReferenceV2->GetBinError(iMult+1);

    double v2, v2err;
    if (Vdelta2 > 0) {
      v2 = TMath::Sqrt(Vdelta2);
      v2err = (1./2) * (1./TMath::Sqrt(Vdelta2)) * Vdelta2err;
    } else {
      v2 = 0;
      v2err = 0;
    }

    printf("iMult = %d: vn = %f +- %f \n", iMult, v2, v2err);

    hRefFlow_v2->SetBinContent(iMult+1, v2);
    hRefFlow_v2->SetBinError(iMult+1, v2err);

    //  do differential flow
    hDifferentialV2[iMult] = (TH1D*)inFile->Get(Form("hDifferentialV2_%d", iMult));

    hDiffFlow_v2[iMult] = new TH1D(Form("hDiffFlow_v2_%d", iMult), Form("differential flow, %.0f < N_{ch} < %.0f; p_{T}; v_{2}", binsMult[iMult], binsMult[iMult+1]), nBinspTtrig, binspTtrig);

    for (int ipTtrig = 0; ipTtrig < nBinspTtrig; ipTtrig++) {
      double Vdelta2Diff = hDifferentialV2[iMult]->GetBinContent(ipTtrig+1);
      double Vdelta2Differr = hDifferentialV2[iMult]->GetBinError(ipTtrig+1);

      double v2diff, v2differr;
      if (Vdelta2Diff > 0 && Vdelta2 > 0){
        v2diff = Vdelta2Diff / TMath::Sqrt(Vdelta2);
        v2differr = TMath::Sqrt( TMath::Power(1./TMath::Sqrt(Vdelta2), 2) * TMath::Power(Vdelta2err, 2)
                                + TMath::Power(0.5*(Vdelta2Diff/TMath::Power(Vdelta2, 3./2)), 2) * TMath::Power(Vdelta2Differr, 2));
      } else {
        v2diff = 0;
        v2differr = 0;
      }

      printf("iMult = %d, ipT = %d: vn = %f +- %f \n", iMult, ipTtrig, v2, v2err);

      hDiffFlow_v2[iMult]->SetBinContent(ipTtrig, v2diff);
      hDiffFlow_v2[iMult]->SetBinError(ipTtrig, v2differr);
    }

    // plot differential flow
    auto canDiffFlow_v2 = new TCanvas;
    DrawHist(hDiffFlow_v2[iMult], kFullCircle, kBlue+1, "ep"); 
    if (savePlots) canDiffFlow_v2->SaveAs(Form("%s/v2_mult%d.png", outputPlotsName, iMult));

  }

  //  plot refernce flow
  auto canRefFlow_v2 = new TCanvas;
  DrawHist(hRefFlow_v2, kFullCircle, kBlue+1, "ep"); 
  if (savePlots) canRefFlow_v2->SaveAs(Form("%s/v2.png", outputPlotsName));

}
//--------------------------
void DrawHist(TH1D* h, Style_t marker, Color_t color, const char* draw)
{
  h->SetMarkerStyle(marker);
  h->SetMarkerColor(color);
  h->SetLineColor(color);

  h->SetStats(0);

  h->Draw(draw);
}
