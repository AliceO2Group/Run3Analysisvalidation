
Bool_t ComputeMinimum(TH1D* h1, Double_t& min, Double_t& minError, Bool_t Debug){
    if(h1){
        //min[] = 0; minError = 0;
        const Int_t Nbins = h1->GetXaxis()->GetNbins();
        Double_t min1 = 0;
        Double_t minError1 = 0;
        
        Double_t minStack[5];
        Double_t minStackError[5];

        minStack[0] = h1->GetBinContent(7); minStackError[0] = h1->GetBinError(7);
        minStack[1] = h1->GetBinContent(8); minStackError[1] = h1->GetBinError(8);
        minStack[2] = h1->GetBinContent(9); minStackError[2] = h1->GetBinError(9);
        minStack[3] = h1->GetBinContent(10); minStackError[3] = h1->GetBinError(10);
        minStack[4] = h1->GetBinContent(11); minStackError[4] = h1->GetBinError(11);

        Double_t ped = h1->GetMinimum();
        if(Debug){cout<<"ped = "<<ped<<endl;}
        for (int iBin=2; iBin<Nbins; iBin++) {
    
            min1 = h1->GetBinContent(iBin);
            minError1 = h1->GetBinError(iBin);
            if(min1<minStack[0]){
                minStack[0] = min1; minStackError[0] = minError1;
            }else if(min1<minStack[1]){
                minStack[1] = min1; minStackError[1] = minError1;
            }else if(min1<minStack[2]){
                minStack[2] = min1; minStackError[2] = minError1;
            }else if(min1<minStack[3]){
                minStack[3] = min1; minStackError[3] = minError1;
            }else if(min1<minStack[4]){
                minStack[4] = min1; minStackError[4] = minError1;
            }

            if(Debug){cout<<"iBin = "<<iBin<<", minStack[0] = "<<minStack[0]<<" , minStack[1] = "<<minStack[1]<<" , minStack[2] = "<<minStack[2]<<endl;}

        }
        min = (minStack[0] + minStack[1] + minStack[2] + minStack[3] + minStack[4])/5.;
        minError = 0.2*sqrt(minStackError[0]*minStackError[0] + minStackError[1]*minStackError[1] + minStackError[2]*minStackError[2] + minStackError[3]*minStackError[3]  + minStackError[4]*minStackError[4]);
	if(Debug) cout<<" min = "<<min<<" , minError = "<<minError<<endl;
        return kTRUE;
    }else{return kFALSE;}
}

