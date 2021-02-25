#!/usr/bin/env python
from ROOT import (  
    TCanvas,
    TFile,
)


def p_diff_reco_MC(var):
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
    cpt = TCanvas("cpt", "pT distribution")
    cpt.SetCanvasSize(1500, 700)
    cpt.Divide(2, 1)
    csecv = TCanvas("csecv", "sec vtx distribution")
    csecv.SetCanvasSize(1500, 700)
    csecv.Divide(2, 1)
    hp = fileo2.Get("hf-mc-validation-rec/histP%s" % var)
    hsecv = fileo2.Get("hf-mc-validation-rec/histSecV%s" % var)
    hp.Draw("")
    hsecv.Draw("")
    cpt.SaveAs("p_recoMC_diff%s.pdf" % var)
    cpt.SaveAs("p_recoMC_diff%s.root" % var)
    csecv.SaveAs("sec_vertex_recoMC_diff_%s.pdf" % var)
    csecv.SaveAs("sec_vertex_recoMC_diff_%s.root" % var)

    p_diff_reco_MC("x")
    p_diff_reco_MC("y")
    p_diff_reco_MC("z")
