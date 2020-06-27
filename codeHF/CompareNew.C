Bool_t CompareNew(TString filerun3="AnalysisResults.root", TString filerun1="Vertices2prong-ITS1.root", double mass=1.8){

  gROOT->SetStyle("Plain");	
  gStyle->SetOptStat(0);
  gStyle->SetOptStat(0000);
  gStyle->SetPalette(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);
  gStyle->SetOptTitle(0);

  TFile *fRun3 = new TFile(filerun3.Data());
  TFile *fRun1 = new TFile(filerun1.Data());

  const int nhisto = 4;
  TString histonameRun1[nhisto] = {"hpt_nocuts",
  				   "hpt_cuts",
                                   "hmass0",
                                   "hmassP"};
  TString histonameRun3[nhisto] = {"produce-sel-track/hpt_nocuts",
  			           "produce-sel-track/hpt_cuts",
                                   "vertexerhf-hftrackindexskimscreator/hmass2pre",
  			           "vertexerhf-hftrackindexskimscreator/hmass3pre"};
  TString xaxis[nhisto] = {"p_{T} before selections",
  			   "p_{T} after selections",
  		           "2-prong presel mass",
  		           "3-prong presel mass"};
 
  TH1F* hRun1[nhisto]; 
  TH1F* hRun3[nhisto]; 
  for (int index=0; index<nhisto; index++){
    hRun1[index] = (TH1F*)fRun1->Get(histonameRun1[index].Data());
    hRun3[index] = (TH1F*)fRun3->Get(histonameRun3[index].Data());
  }

  TCanvas* cv=new TCanvas("cv","Vertex",1600,1600);
  cv->Divide(3,3);
  gPad-> SetLogy();
  for (int index=0; index<nhisto; index++){
    cv->cd(index+1); 
    hRun1[index]->SetLineColor(1);
    hRun1[index]->SetLineWidth(3);
    hRun3[index]->SetLineColor(2);
    hRun3[index]->SetLineWidth(1);
    hRun1[index]->GetXaxis()->SetTitle(xaxis[index].Data());
    hRun1[index]->Draw();
    hRun3[index]->Draw("same");
    TLegend * legend = new TLegend(0.5,0.7,0.8,0.9);
    legend->AddEntry(hRun1[index],"Run1","f");
    legend->AddEntry(hRun3[index],"Run3","f");
    legend->Draw();
  }
  cv->SaveAs("optimisedcomp.pdf");
  return true; 
}



