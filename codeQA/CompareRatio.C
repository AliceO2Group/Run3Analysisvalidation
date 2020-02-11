#include <TH1F.h>
#include <TFile.h>
#include <TROOT.h>
#include "TCanvas.h"
#include "TLegend.h"
#include <iostream>
#include "AliESDEventMFT.h"
#include "AliESDtrack.h"
#include <TTree.h>
#include "TStyle.h"

Bool_t CompareRatio(){

  gROOT->SetStyle("Plain");	
  gStyle->SetOptStat(0);
  gStyle->SetOptStat(0000);
  gStyle->SetPalette(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);
  gStyle->SetOptTitle(0);

  TFile *fRun3 = new TFile("AnalysisResults.root");
  TFile *fRun1 = new TFile("QA-ITS1.root");

  // run 1
  TH1F* hpt_nocuts_Run1 = (TH1F*)fRun1->Get("hpt_nocuts");
  TH1F* hrun_number_Run1 = (TH1F*)fRun1->Get("hrun_number");
  TH1F* hfCYY_Run1 = (TH1F*)fRun1->Get("hfCYY");
  TH1F* hfCZY_Run1 = (TH1F*)fRun1->Get("hfCZY");
  TH1F* hfCZZ_Run1 = (TH1F*)fRun1->Get("hfCZZ");
  TH1F* hfCSnpY_Run1 = (TH1F*)fRun1->Get("hfCSnpY");
  TH1F* hfCSnpZ_Run1 = (TH1F*)fRun1->Get("hfCSnpZ");
  TH1F* hfCSnpSnp_Run1 = (TH1F*)fRun1->Get("hfCSnpSnp");
  TH1F* hfCTglY_Run1 = (TH1F*)fRun1->Get("hfCTglY");
  TH1F* hfCTglZ_Run1 = (TH1F*)fRun1->Get("hfCTglZ");
  TH1F* hfCTglSnp_Run1 = (TH1F*)fRun1->Get("hfCTglSnp");
  TH1F* hfCTglTgl_Run1 = (TH1F*)fRun1->Get("hfCTglTgl");
  TH1F* hfC1PtY_Run1 = (TH1F*)fRun1->Get("hfC1PtY");
  TH1F* hfC1PtZ_Run1 = (TH1F*)fRun1->Get("hfC1PtZ");
  TH1F* hfC1PtSnp_Run1 = (TH1F*)fRun1->Get("hfC1PtSnp");
  TH1F* hfC1PtTgl_Run1 = (TH1F*)fRun1->Get("hfC1PtTgl");
  TH1F* hfC1Pt21Pt2_Run1 = (TH1F*)fRun1->Get("hfC1Pt21Pt2");

  // run 3
  TH1F* hpt_nocuts_Run3 = (TH1F*)fRun3->Get("validation-qa/hpt_nocuts");
  TH1F* hrun_number_Run3 = (TH1F*)fRun3->Get("validation-qa/hrun_number");
  TH1F* hfCYY_Run3 = (TH1F*)fRun3->Get("validation-qa/hfCYY");
  TH1F* hfCZY_Run3 = (TH1F*)fRun3->Get("validation-qa/hfCZY");
  TH1F* hfCZZ_Run3 = (TH1F*)fRun3->Get("validation-qa/hfCZZ");
  TH1F* hfCSnpY_Run3 = (TH1F*)fRun3->Get("validation-qa/hfCSnpY");
  TH1F* hfCSnpZ_Run3 = (TH1F*)fRun3->Get("validation-qa/hfCSnpZ");
  TH1F* hfCSnpSnp_Run3 = (TH1F*)fRun3->Get("validation-qa/hfCSnpSnp");
  TH1F* hfCTglY_Run3 = (TH1F*)fRun3->Get("validation-qa/hfCTglY");
  TH1F* hfCTglZ_Run3 = (TH1F*)fRun3->Get("validation-qa/hfCTglZ");
  TH1F* hfCTglSnp_Run3 = (TH1F*)fRun3->Get("validation-qa/hfCTglSnp");
  TH1F* hfCTglTgl_Run3 = (TH1F*)fRun3->Get("validation-qa/hfCTglTgl");
  TH1F* hfC1PtY_Run3 = (TH1F*)fRun3->Get("validation-qa/hfC1PtY");
  TH1F* hfC1PtZ_Run3 = (TH1F*)fRun3->Get("validation-qa/hfC1PtZ");
  TH1F* hfC1PtSnp_Run3 = (TH1F*)fRun3->Get("validation-qa/hfC1PtSnp");
  TH1F* hfC1PtTgl_Run3 = (TH1F*)fRun3->Get("validation-qa/hfC1PtTgl");
  TH1F* hfC1Pt21Pt2_Run3 = (TH1F*)fRun3->Get("validation-qa/hfC1Pt21Pt2");

  TLegend * legend = new TLegend(0.5,0.7,0.8,0.9);

  TCanvas* cv1=new TCanvas("cv1","Vertex",1600,700);
  cv1->Divide(3,2);
  gPad-> SetLogy();

  cv1 -> cd(1);
  hpt_nocuts_Run1->GetXaxis()->SetTitle("#it{p}_{T} (GeV)");
  hpt_nocuts_Run1->SetLineColor(2);
  hpt_nocuts_Run1->SetLineWidth(2);
  hpt_nocuts_Run1->Divide(hpt_nocuts_Run3);
  hpt_nocuts_Run1->Draw();
  legend->AddEntry(hpt_nocuts_Run1,"Run1 data/Run3 data","f");
  legend->Draw();

  cv1->cd(2);
  hrun_number_Run1->GetXaxis()->SetTitle("run number");
  hrun_number_Run1->SetLineColor(2);
  hrun_number_Run1->SetLineWidth(2);
  hrun_number_Run1->Divide(hrun_number_Run3);
  hrun_number_Run1->Draw();

  cv1->cd(3);
  hfCYY_Run1->GetXaxis()->SetTitle("CYY");
  hfCYY_Run1->SetLineColor(2);
  hfCYY_Run1->SetLineWidth(2);
  hfCYY_Run1->Divide(hfCYY_Run3);
  hfCYY_Run1->Draw();

  cv1->cd(4);
  hfCZY_Run1->GetXaxis()->SetTitle("CZY");
  hfCZY_Run1->SetLineColor(2);
  hfCZY_Run1->SetLineWidth(2);
  hfCZY_Run1->Divide(hfCZY_Run3);
  hfCZY_Run1->Draw();

  cv1->cd(5);
  hfCZZ_Run1->GetXaxis()->SetTitle("CZZ");
  hfCZZ_Run1->SetLineColor(2);
  hfCZZ_Run1->SetLineWidth(2);
  hfCZZ_Run1->Divide(hfCZZ_Run3);
  hfCZZ_Run1->Draw();

  cv1->cd(6);
  hfCSnpY_Run1->GetXaxis()->SetTitle("CSnpY");
  hfCSnpY_Run1->SetLineColor(2);
  hfCSnpY_Run1->SetLineWidth(2);
  hfCSnpY_Run1->Divide(hfCSnpY_Run3);
  hfCSnpY_Run1->Draw();

  cv1->SaveAs("cv1_rat.png");


  TCanvas* cv2=new TCanvas("cv2","Vertex",1600,700);
  cv2->Divide(3,2);
  gPad-> SetLogy();

  cv2 -> cd(1);
  hfCSnpZ_Run1->GetXaxis()->SetTitle("CSnpZ");
  hfCSnpZ_Run1->SetLineColor(2);
  hfCSnpZ_Run1->SetLineWidth(2);
  hfCSnpZ_Run1->Draw();
  hfCSnpZ_Run1->Divide(hfCSnpZ_Run3);
  hfCSnpZ_Run1->Draw();

  cv2->cd(2);
  hfCSnpSnp_Run1->GetXaxis()->SetTitle("SnpSnp");
  hfCSnpSnp_Run1->SetLineColor(2);
  hfCSnpSnp_Run1->SetLineWidth(2);
  hfCSnpSnp_Run1->Divide(hfCSnpSnp_Run3);
  hfCSnpSnp_Run1->Draw();

  cv2->cd(3);
  hfCTglY_Run1->GetXaxis()->SetTitle("CTglY");
  hfCTglY_Run1->SetLineColor(2);
  hfCTglY_Run1->SetLineWidth(2);
  hfCTglY_Run1->Divide(hfCTglY_Run3);
  hfCTglY_Run1->Draw();

  cv2->cd(4);
  hfCTglZ_Run1->GetXaxis()->SetTitle("CTglZ");
  hfCTglZ_Run1->SetLineColor(2);
  hfCTglZ_Run1->SetLineWidth(2);
  hfCTglZ_Run1->Divide(hfCTglZ_Run3);
  hfCTglZ_Run1->Draw();

  cv2->cd(5);
  hfCTglSnp_Run1->GetXaxis()->SetTitle("CTglSnp");
  hfCTglSnp_Run1->SetLineColor(2);
  hfCTglSnp_Run1->SetLineWidth(2);
  hfCTglSnp_Run1->Divide(hfCTglSnp_Run3);
  hfCTglSnp_Run1->Draw();

  cv2->cd(6);
  hfCTglTgl_Run1->GetXaxis()->SetTitle("CTglTgl");
  hfCTglTgl_Run1->SetLineColor(2);
  hfCTglTgl_Run1->SetLineWidth(2);
  hfCTglTgl_Run1->Divide(hfCTglTgl_Run3);
  hfCTglTgl_Run1->Draw();

  cv2->SaveAs("cv2_rat.png");


  TCanvas* cv3=new TCanvas("cv3","Vertex",1600,700);
  cv3->Divide(3,2);
  gPad-> SetLogy();

  cv3 -> cd(1);
  hfC1PtY_Run1->GetXaxis()->SetTitle("C1PtY");
  hfC1PtY_Run1->SetLineColor(2);
  hfC1PtY_Run1->SetLineWidth(2);
  hfC1PtY_Run1->Divide(hfC1PtY_Run3);
  hfC1PtY_Run1->Draw();
  legend->Draw();

  cv3->cd(2);
  hfC1PtZ_Run1->GetXaxis()->SetTitle("C1PtZ");
  hfC1PtZ_Run1->SetLineColor(2);
  hfC1PtZ_Run1->SetLineWidth(2);
  hfC1PtZ_Run1->Divide(hfC1PtZ_Run3);
  hfC1PtZ_Run1->Draw();

  cv3->cd(3);
  hfC1PtSnp_Run1->GetXaxis()->SetTitle("C1PtSnp");
  hfC1PtSnp_Run1->SetLineColor(2);
  hfC1PtSnp_Run1->SetLineWidth(2);
  hfC1PtSnp_Run1->Divide(hfC1PtSnp_Run3);
  hfC1PtSnp_Run1->Draw();

  cv3->cd(4);
  hfC1PtTgl_Run1->GetXaxis()->SetTitle("C1PtTgl");
  hfC1PtTgl_Run1->SetLineColor(2);
  hfC1PtTgl_Run1->SetLineWidth(2);
  hfC1PtTgl_Run1->Divide(hfC1PtTgl_Run3);
  hfC1PtTgl_Run1->Draw();

  cv3->cd(5);
  hfC1Pt21Pt2_Run1->GetXaxis()->SetTitle("C1Pt21Pt2");
  hfC1Pt21Pt2_Run1->SetLineColor(2);
  hfC1Pt21Pt2_Run1->SetLineWidth(2);
  hfC1Pt21Pt2_Run1->Divide(hfC1Pt21Pt2_Run3);
  hfC1Pt21Pt2_Run1->Draw();

  cv3->SaveAs("cv3_rat.png");

  return true;
}



