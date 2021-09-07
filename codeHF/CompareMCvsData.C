//This is a macro to compare the invariant mass distributions of Monte Carlo and data.

void CompareMCvsData(){
//Set Canvas
	TCanvas *c = new TCanvas("c","MC and data Comparison for Xic task",2600,1100);
	c->Divide(4,3);
	gStyle->SetOptStat(0);
	gStyle->SetPalette(0);
	gStyle->SetCanvasColor(0);
	gStyle->SetFrameFillColor(0);


//Read original root files
	auto fileMC(TFile::Open("AnalysisResults_O2_MC.root","READ"));
	auto fileDA(TFile::Open("AnalysisResults_O2_DATA.root","READ"));

//Get 2-D hmass histos and normalization
	auto hmass_MC(static_cast<TH2F*>((fileMC)->Get("hf-task-xic/hmass")));
	auto hmass_DA(static_cast<TH2F*>((fileDA)->Get("hf-task-xic/hmass")));
	auto hCPA_MC(static_cast<TH2F*>((fileMC)->Get("hf-task-xic/hCPA")));
	auto hCPA_DA(static_cast<TH2F*>((fileDA)->Get("hf-task-xic/hCPA")));
	auto hCt_MC(static_cast<TH2F*>((fileMC)->Get("hf-task-xic/hCt")));
	auto hCt_DA(static_cast<TH2F*>((fileDA)->Get("hf-task-xic/hCt")));
	auto hEta_MC(static_cast<TH2F*>((fileMC)->Get("hf-task-xic/hEta")));
	auto hEta_DA(static_cast<TH2F*>((fileDA)->Get("hf-task-xic/hEta")));
	auto hdeclength_MC(static_cast<TH2F*>((fileMC)->Get("hf-task-xic/hdeclength")));
	auto hdeclength_DA(static_cast<TH2F*>((fileDA)->Get("hf-task-xic/hdeclength")));
	auto hptcand_MC(static_cast<TH1F*>((fileMC)->Get("hf-task-xic/hptcand")));
	auto hptcand_DA(static_cast<TH1F*>((fileDA)->Get("hf-task-xic/hptcand")));
	auto hptprong0_MC(static_cast<TH1F*>((fileMC)->Get("hf-task-xic/hptprong0")));
	auto hptprong0_DA(static_cast<TH1F*>((fileDA)->Get("hf-task-xic/hptprong0")));
	auto hptprong1_MC(static_cast<TH1F*>((fileMC)->Get("hf-task-xic/hptprong1")));
	auto hptprong1_DA(static_cast<TH1F*>((fileDA)->Get("hf-task-xic/hptprong1")));
	auto hptprong2_MC(static_cast<TH1F*>((fileMC)->Get("hf-task-xic/hptprong2")));
	auto hptprong2_DA(static_cast<TH1F*>((fileDA)->Get("hf-task-xic/hptprong2")));
	
	hmass_MC->SetLineColor(kRed);
	hmass_DA->SetLineColor(kBlue);

	hCPA_MC->SetLineColor(kRed);
	hCPA_DA->SetLineColor(kBlue);

	hCt_MC->SetLineColor(kRed);
	hCt_DA->SetLineColor(kBlue);

	hEta_MC->SetLineColor(kRed);
	hEta_DA->SetLineColor(kBlue);

	hdeclength_MC->SetLineColor(kRed);
	hdeclength_DA->SetLineColor(kBlue);

	hptcand_MC->SetLineColor(kRed);
	hptcand_DA->SetLineColor(kBlue);

	hptprong0_MC->SetLineColor(kRed);
	hptprong0_DA->SetLineColor(kBlue);

	hptprong1_MC->SetLineColor(kRed);
	hptprong1_DA->SetLineColor(kBlue);

	hptprong2_MC->SetLineColor(kRed);
	hptprong2_DA->SetLineColor(kBlue);

	auto hmass_MC_px = (hmass_MC)->ProjectionX("hmass_MC_px"); 
	auto hmass_DA_px = (hmass_DA)->ProjectionX("hmass_DA_px"); 

	auto hPt_MC = (hmass_MC)->ProjectionY("hPt_MC"); 
	auto hPt_DA = (hmass_DA)->ProjectionY("hPt_DA");
    
	auto hCPA_MC_px = (hCPA_MC)->ProjectionX("hCPA_MC_px");
	auto hCPA_DA_px = (hCPA_DA)->ProjectionX("hCPA_DA_px");
	
	auto hCt_MC_px = (hCt_MC)->ProjectionX("hCt_MC_px");
	auto hCt_DA_px = (hCt_DA)->ProjectionX("hCt_DA_px");

	auto hEta_MC_px = (hEta_MC)->ProjectionX("hEta_MC_px");
	auto hEta_DA_px = (hEta_DA)->ProjectionX("hEta_DA_px");
		
	auto hdeclength_MC_px = (hdeclength_MC)->ProjectionX("hdeclength_MC_px");
	auto hdeclength_DA_px = (hdeclength_DA)->ProjectionX("hdeclength_DA_px");
    
	hmass_MC_px->Scale(1/(hmass_MC)->GetEntries());
	hmass_DA_px->Scale(1/(hmass_DA)->GetEntries());	
	hPt_MC->Scale(1/(hPt_MC)->GetEntries());
	hPt_DA->Scale(1/(hPt_DA)->GetEntries());	
	hCPA_MC_px->Scale(1/(hCPA_MC_px)->GetEntries());
	hCPA_DA_px->Scale(1/(hCPA_DA_px)->GetEntries());
	hCt_MC_px->Scale(1/(hCt_MC_px)->GetEntries());
	hCt_DA_px->Scale(1/(hCt_DA_px)->GetEntries());
	hEta_MC_px->Scale(1/(hEta_MC_px)->GetEntries());
	hEta_DA_px->Scale(1/(hEta_DA_px)->GetEntries());
	hdeclength_MC_px->Scale(1/(hdeclength_MC_px)->GetEntries());
	hdeclength_DA_px->Scale(1/(hdeclength_DA_px)->GetEntries());
	hptcand_MC->Scale(1/(hptcand_MC)->GetEntries());
	hptcand_DA->Scale(1/(hptcand_DA)->GetEntries());
	hptprong0_MC->Scale(1/(hptprong0_MC)->GetEntries());
	hptprong0_DA->Scale(1/(hptprong0_DA)->GetEntries());
	hptprong1_MC->Scale(1/(hptprong1_MC)->GetEntries());
	hptprong1_DA->Scale(1/(hptprong1_DA)->GetEntries());
	hptprong2_MC->Scale(1/(hptprong2_MC)->GetEntries());
	hptprong2_DA->Scale(1/(hptprong2_DA)->GetEntries());
	

//Draw the plots.
	c->cd(1);
	TLegend *l = new TLegend(0.1,0.65,0.3,0.8);
	l->AddEntry(hmass_MC_px,"MC");
	l->AddEntry(hmass_DA_px,"data");
	l->SetFillStyle(0);
	l->SetLineWidth(0);
	hmass_MC_px->Draw();
	hmass_DA_px->Draw("same");
	l->Draw("same");

	c->cd(2);
	hPt_MC->Draw();
	hPt_DA->Draw("same");

	c->cd(3);
	hCPA_MC_px->Draw();
	hCPA_DA_px->Draw("same");

	c->cd(4);
	hCt_MC_px->Draw();
	hCt_DA_px->Draw("same");

	c->cd(5);
	hEta_MC_px->Draw();
	hEta_DA_px->Draw("same");

	c->cd(6);
	hdeclength_MC_px->Draw();
	hdeclength_DA_px->Draw("same");

	c->cd(7);
	hptcand_MC->Draw();
	hptcand_DA->Draw("same");

	c->cd(8);
	hptprong0_MC->Draw();
	hptprong0_DA->Draw("same");

	c->cd(9);
	hptprong1_MC->Draw();
	hptprong1_DA->Draw("same");

	c->cd(10);
	hptprong2_MC->Draw();
	hptprong2_DA->Draw("same");

	c->SaveAs("comparison_MCvsDATA_Xic.png");
}
