// Plotting utilities

void SetCanvas(TCanvas* can, int nPadsX, int nPadsY)
{
  can->Divide(nPadsX, nPadsY, 0.005, 0.005);
}

void SetPad(TVirtualPad* pad, bool logScale)
{
  pad->SetBottomMargin(0.11);
  pad->SetLeftMargin(0.1);
  pad->SetTopMargin(0.08);
  pad->SetRightMargin(0.08);
  if (logScale) {
    pad->SetLogy();
  }
}

void SetHistogram(TH1* his, Float_t yMin, Float_t yMax, Float_t marginLow, Float_t marginHigh, bool logScale)
{
  Float_t textsize = 0.05;
  his->GetYaxis()->SetTitleSize(textsize);
  his->GetXaxis()->SetTitleSize(textsize);
  his->GetYaxis()->SetTitleOffset(1.0);
  his->GetXaxis()->SetTitleOffset(1.0);
  Float_t k = 1. - marginHigh - marginLow;
  Float_t yRange;
  if (logScale && yMin > 0 && yMax > 0) {
    yRange = yMax / yMin;
    his->GetYaxis()->SetRangeUser(yMin / std::pow(yRange, marginLow / k), yMax * std::pow(yRange, marginHigh / k));
  } else {
    yRange = yMax - yMin;
    his->GetYaxis()->SetRangeUser(yMin - marginLow / k * yRange, yMax + marginHigh / k * yRange);
  }
}
