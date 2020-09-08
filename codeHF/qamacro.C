void qamacro(){
  TFile f("AnalysisResults.root");    
  TH2F*hptDResvspt=(TH2F*)f.Get("qa-tracking-resolution/ptDResvspt");    
  TH2F*hptDResvseta=(TH2F*)f.Get("qa-tracking-resolution/ptDResvseta");    
  TProfile*pptDResvspt=hptDResvspt->ProfileX();    
  TProfile*pptDResvseta=hptDResvseta->ProfileX();    
  TCanvas*ctrackres=new TCanvas("ctrackres", "ctrackres", 1000, 500);    
  ctrackres->Divide(2,1);    
  ctrackres->cd(1);
  pptDResvspt->Draw();
  ctrackres->cd(2);
  pptDResvseta->Draw();
  ctrackres->SaveAs("ctrackres.pdf");    
}


