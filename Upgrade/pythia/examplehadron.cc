// This is a simple macro to generate pythia predictions

#include <algorithm>
#include <iostream>
#include <vector>
#include <math.h>
#include <iomanip>
#include <string>
#include <cstring>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include "Pythia8/Pythia.h"
#include "TTree.h"
#include "THnSparse.h"
#include "TProfile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "TClonesArray.h"
#include "TFile.h"
#include "TList.h"
#include "TVector3.h"
#include "TMath.h"
#include "TNtuple.h"
#include "TString.h"
#include "TRandom3.h"
#include "TH1D.h"
#include "fastjet/PseudoJet.hh"
//#include "fastjet/ClusterSequence.hh"
//#include "fastjet/ClusterSequenceArea.hh"
#include <ctime>
#include <iostream> // needed for io
#include <cstdio>   // needed for io
#include <valarray>
#include <time.h>       /* time */
//#include <yaml.h>
//include <stdio.h>
//include <glib.h>
#include <yaml-cpp/yaml.h>

using namespace Pythia8;

int main(int argc, char* argv[]) {
    (void)argc;
    std::string mycase = argv[1];
    int cislo = -1;                 //unique number for each file
    cislo = atoi(argv[2]);
    // number of parallel jobs to be run. Be aware that each single file
    // will be normalized by this number to make sure that the merged output
    // file has the proper normalization!
    int n_jobs = -1;
    n_jobs = atoi(argv[3]);

    YAML::Node node = YAML::LoadFile("config.yaml");
    YAML::Node nodecase = node[mycase.data()];

    const std::string myhadronname = nodecase["myhadronname"].as<std::string>();
    const std::string myhadronlatex = nodecase["myhadronlatex"].as<std::string>();
    int pdgparticle = nodecase["pdgparticle"].as<int>();
    float correction = nodecase["correction"].as<float>();
    int maxnevents = nodecase["maxneventsperjob"].as<int>();
    int tune = nodecase["tune"].as<int>();
    int beamidA = nodecase["beamidA"].as<int>();
    int beamidB = nodecase["beamidB"].as<int>();
    float eCM = nodecase["eCM"].as<float>();
    const std::string pythiamode = nodecase["pythiamode"].as<std::string>();
    const std::string outputfile = nodecase["outputfile"].as<std::string>();
    double nptbins = nodecase["nptbins"].as<int>();
    double ptmin = nodecase["ptmin"].as<float>();
    double ptmax = nodecase["ptmax"].as<float>();
    double ymin = nodecase["ymin"].as<float>();
    double ymax = nodecase["ymax"].as<float>();
    const std::string extramode = nodecase["extramode"].as<std::string>();

    //END OF CONFIGURATION


    // Generator. Process selection. LHC initialization. Histogram.
    Pythia pythia;
    pythia.readString(Form("%s", pythiamode.data()));
    pythia.readString(Form("Main:numberOfEvents = %d", maxnevents));
    pythia.readString("Next:numberShowEvent = 0");
    pythia.readString(Form("Tune:pp = %d", tune));
    pythia.readString(Form("Beams:idA = %d", beamidA));
    pythia.readString(Form("Beams:idB = %d", beamidB));
    pythia.readString(Form("Beams:eCM = %f", eCM));

    pythia.readString("Random:setSeed = on");
    pythia.readString(Form("Random:seed = %d",cislo));

    if (extramode=="mode2") {
        std::cout<<"Running with mode2"<<std::endl;
        pythia.readString("ColourReconnection:mode = 1");
        pythia.readString("ColourReconnection:allowDoubleJunRem = off");
        pythia.readString("ColourReconnection:m0 = 0.3");
        pythia.readString("ColourReconnection:allowJunctions = on");
        pythia.readString("ColourReconnection:junctionCorrection = 1.20");
        pythia.readString("ColourReconnection:timeDilationMode = 2");
        pythia.readString("ColourReconnection:timeDilationPar = 0.18");
        pythia.readString("StringPT:sigma = 0.335");
        pythia.readString("StringZ:aLund = 0.36");
        pythia.readString("StringZ:bLund = 0.56");
        pythia.readString("StringFlav:probQQtoQ = 0.078");
        pythia.readString("StringFlav:ProbStoUD = 0.2");
        pythia.readString("StringFlav:probQQ1toQQ0join = 0.0275,0.0275,0.0275,0.0275");
        pythia.readString("MultiPartonInteractions:pT0Ref = 2.15");
        pythia.readString("BeamRemnants:remnantMode = 1");
        pythia.readString("BeamRemnants:saturation =5");
    }
    pythia.init();

    TFile *fout = new TFile(outputfile.data(), "recreate");

    fout->cd();
    TH1F*hparticlept = new TH1F("hchargedparticles_pt", ";p_{T};charged particle dN/dp_{T}", nptbins, ptmin, ptmax);
    TH1F*hptyields_unnorm = new TH1F(Form("h%syieldsvspt_unnorm", myhadronname.data()), ";p_{T} (GeV);unnormalized yield (particle+anti)", nptbins, ptmin, ptmax);
    TH1F*hptcross = new TH1F(Form("h%scrossvspt", myhadronname.data()), Form(";p_{T} (GeV);%s d#sigma^{PYTHIA}/dp_{T} (#mu b/GeV)", myhadronlatex.data()), nptbins, ptmin, ptmax);
    TH1F*hycharmcross = new TH1F("hycharmcross", ";y;%s d#sigma_{c}^{PYTHIA}/dy (#mu b)", 61, -30.5, 30.5);
    TH1F*hycross = new TH1F("hycross", ";y;%s d#sigma_{HF}^{PYTHIA}/dy (#mu b)", 61, -30.5, 30.5);
    TH2F*hptycharmcross = new TH2F("hptcharmcross", ";p_{T} (GeV); y", 100, 0., 100.,60, -30., 30.);

    // Begin event loop. Generate event. Skip if error. List first one.
    int nmyhadron = 0;
    for (int iEvent = 0; iEvent < maxnevents; ++iEvent) {

        pythia.next();

        for (int i = 0; i < pythia.event.size(); ++i) {
            if(pythia.event[i].pT()<0 || pythia.event[i].pT()>1.e+5) continue;
            if(pythia.event[i].idAbs()==4) {
		hycharmcross->Fill(pythia.event[i].y());
	        hptycharmcross->Fill(pythia.event[i].pT(), pythia.event[i].y());
	    }
	    if(pythia.event[i].idAbs()==pdgparticle){
		hycross->Fill(pythia.event[i].y());
	    }
	    if(pythia.event[i].y()<ymin || pythia.event[i].y()>ymax) continue;

            hparticlept->Fill(pythia.event[i].pT());
            if(pythia.event[i].idAbs()==pdgparticle) {
                ++nmyhadron;
                hptyields_unnorm->Fill(pythia.event[i].pT());
            }
            // End of event loop. Statistics. Histogram. Done.
        }
    }
    pythia.stat();
    int nbinspT=hptcross->GetNbinsX();
    double norm_fact = correction*pythia.info.sigmaGen()*1000/(2*n_jobs*pythia.info.nAccepted());
    printf("norm fact %f\n", norm_fact);
    double contentmyhadron[nbinspT], binwidthmyhadron[nbinspT];

    for(int ibin=0; ibin < nbinspT; ibin++) {
        contentmyhadron[ibin] = hptyields_unnorm->GetBinContent(ibin);
        binwidthmyhadron[ibin]= hptyields_unnorm->GetBinWidth(ibin);
        printf("bin %d, Content %f, binwidth %f\n",ibin, contentmyhadron[ibin], binwidthmyhadron[ibin]);
        hptcross ->SetBinContent(ibin, contentmyhadron[ibin]*norm_fact/binwidthmyhadron[ibin]);
    }
    const int nbinx = hptycharmcross->GetNbinsX();
    const int nbiny = hptycharmcross->GetNbinsY();

    double contentmyhadronxy[nbinx][nbinx];
    double binwidthmyhadronxy[nbinx][nbinx];
    for(int ibinx=0; ibinx < nbinx; ibinx++){
        for(int ibiny=0; ibiny < nbiny; ibiny++){
            contentmyhadronxy[ibinx][ibiny] = hptycharmcross->GetBinContent(ibinx+1, ibiny+1);
            binwidthmyhadronxy[ibinx][ibiny]= hptycharmcross->GetXaxis()->GetBinWidth(ibinx+1)*hptycharmcross->GetYaxis()->GetBinWidth(ibiny+1);
            hptycharmcross->SetBinContent(ibinx+1, ibiny+1, contentmyhadronxy[ibinx][ibiny]*norm_fact/binwidthmyhadronxy[ibinx][ibiny]);
        }
    }
    hycharmcross->Scale(norm_fact);
    hycross->Scale(norm_fact);
    printf("nAccepted %ld, nTried %ld\n", pythia.info.nAccepted(), pythia.info.nTried());
    printf("pythia.info.sigmaGen() %f\n", pythia.info.sigmaGen());
    printf("N myhadron %d", nmyhadron);
    hptcross -> Write();
    hptyields_unnorm->Write();
    hptycharmcross->Write();
    hycharmcross->Write();
    hycross->Write();
    fout->Write();
    return 0;
}
