#!/usr/bin/env python
from ROOT import TH1F, TCanvas, TEfficiency, TFile, TLegend, gPad


def efficiencytracking():

    hadron_list = ["pion","proton","kaon","electron", "muon"]#, "electron", "muon"]
    color_list = [ 1,2, 4, 6, 8]
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")

    c1 = TCanvas("c1", "Efficiency")
    c1.SetCanvasSize(1500, 1500)
    c1.cd()
    gPad.SetLogx()
   # gPad.SetLogy()
    eff_list = []
    hempty = TH1F("hempty", ";p_{T};efficiency", 100, 0.001, 15.0)
    hempty.Draw()
    leg = TLegend(0.1, 0.7, 0.3, 0.9, "")
    leg.SetFillColor(0)

    for i, had in enumerate(hadron_list):
        hnum = fileo2.Get("qa-tracking-efficiency-%s/num" % had)
        hden = fileo2.Get("qa-tracking-efficiency-%s/den" % had)
        hnum.Rebin(4)
        hden.Rebin(4)
        leff = fileo2.Get("qa-tracking-efficiency-%s/Efficiency" % had)
        eff = leff.At(0)
        #eff = TEfficiency(hnum, hden)
        eff.SetLineColor(color_list[i])
        #hden.SetLineColor(color_list[i])
        eff_list.append(eff)
        eff.Draw("same")
        #hden.GetYaxis().SetRange(0, 1000)
        #hden.Draw("same")
        leg.AddEntry(eff_list[i], had)
    leg.Draw()
    c1.SaveAs("efficiency_tracking.pdf")
    c1.SaveAs("efficiency_tracking.root")
   # c1.SaveAs("EtaReco_distribution.pdf")
   # c1.SaveAs("EtaReco_distribution.root")


def efficiencyhadron(had, var):
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
    ceffhf = TCanvas("ceffhf", "A Simple Graph Example")
    ceffhf.SetCanvasSize(1500, 700)
    ceffhf.Divide(2, 1)
    gPad.SetLogy()
    hnum = fileo2.Get("qa-tracking-efficiency-%s/num%s" % (had, var))
    #hnum = fileo2.Get("hf-task-%s-mc/h%sRecSig" % (had, var))
    hden = fileo2.Get("qa-tracking-efficiency-%s/den%s" % (had, var))
    #hden = fileo2.Get("hf-task-%s-mc/h%sGen" % (had, var))
    hnum.Rebin(4)
    hden.Rebin(4)
    eff = TEfficiency(hnum, hden)
    eff.Draw()
    ceffhf.SaveAs("efficiency_hfcand%s%s.pdf" % (had, var))
    ceffhf.SaveAs("efficiency_hfcand%s%s.root" % (had, var))


def pTdistribution (had):
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
    cpt = TCanvas("cpt", "pT distribution")
    cpt.SetCanvasSize(1500, 700)
    cpt.Divide(2, 1)
    hnum = fileo2.Get("qa-tracking-efficiency-%s/num" % had)
    hden = fileo2.Get("qa-tracking-efficiency-%s/den" % had)
    hden.Rebin(4)
    hnum.Rebin(4)
    hnum.SetLineColor(6)
    hden.Draw()
    hnum.Draw("same")
    cpt.SaveAs("efficiency_hfcand%s.pdf" % had)
    cpt.SaveAs("efficiency_hfcand%s.root" % had)


def var_tracking(var):
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
    cvar = TCanvas("cvar", "var distribution")
    cvar.SetCanvasSize(1500, 700)
    cvar.Divide(2, 1)
    hvar = fileo2.Get("qa-tracking-kine/tracking/%s" % var)
    hvar.Rebin(4)
    hvar.Draw()
    cvar.SaveAs("tracking_%s.pdf" % var)
    cvar.SaveAs("tracking_%s.root" % var)

def tracking_resolution(var1, mode, var2, var3):
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
    cres = TCanvas("cres", "resolution distribution")
    cres.SetCanvasSize(1500, 700)
    if var1 == "eta":
        heta = fileo2.Get("qa-tracking-resolution/eta/etaDiffMCRecoVsEta%s" % mode)
        heta.Draw("coltz")
        cres.SaveAs("Resolution_%s_%s.pdf" % (var1, var2))
        cres.SaveAs("Resolution_%s_%s.root" % (var1, var2))
    if var1 == "impactParameter":
        himp_par = fileo2.Get("qa-tracking-resolution/%s/%s%sVs%s" % (var1, var1, var2, var3))
        himp_par.Draw("coltz")
        cres.SaveAs("impact_parameter_resolution%sVs%s.pdf" %(var2,var3))
    if var1 == "impactParameterError":
        himp_par_err = fileo2.Get("qa-tracking-resolution/impactParameter/%s%sVs%s" % (var1, var2, var3))
        himp_par_err.Draw("coltz")
        cres.SaveAs("impactParameterError_resolution%sVs%s.pdf" %(var2,var3))
    if var1 == "phi":
        hphi = fileo2.Get("qa-tracking-resolution/phi/phiDiffMCRec")
        hphi.Draw()
        cres.SaveAs("phi_resolution.pdf")
        cres.SaveAs("phi_resolution.root")
    if var1 == "pt":
        h_pt= fileo2.Get("qa-tracking-resolution/pt/ptResolutionVs%s" % var2)
        h_pt.Draw("coltz")
        cres.SaveAs("ptResolutionVs%s.pdf" % var2)
        cres.SaveAs("ptResolutionVs%s.root" % var2)

def vertex_distributions (var):
    #x, y, z
   fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
   cvertex = TCanvas("cvertex", "vertex ")
   cvertex.SetCanvasSize(1500, 700)
   cvertex.Divide(2, 1)
   hvertex = fileo2.Get("qa-global-observables/collision/%s" % var)
   hvertex.Draw()
   cvertex.SaveAs("vertex_%s.pdf" % var)
   cvertex.SaveAs("vertex_%s.root" % var)

def drawhisto (had,var):
   fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
   cvarhf = TCanvas("cvarhf", "A Simple Graph Example")
   cvarhf.SetCanvasSize(1500, 700)
   cvarhf.Divide(2, 1)
   #hnum = fileo2.Get("qa-tracking-efficiency-%s/num%s" % (had, var))
   hden = fileo2.Get("qa-tracking-efficiency-%s/den%s" % (had, var))
   hden.Rebin(4)
   hden.Draw()
 #  hnum.Draw("same")  
   cvarhf.SaveAs("Var_hfcand%s%s.pdf" % (had, var))
   cvarhf.SaveAs("Var_hfcand%s%s.root" % (had, var))

efficiencytracking()
#efficiencyhadron("kaon", "Eta")
drawhisto("kaon", "")
#drawhisto("kaon", "Eta")
#efficiencyhadron("kaon", "Eta")
#drawhisto("electron", "")
#drawhisto("electron", "Eta")
pTdistribution("kaon")
pTdistribution("pion")
pTdistribution("proton")
pTdistribution("electron")
pTdistribution("muon")
vertex_distributions("X")
vertex_distributions("Y")
vertex_distributions("Z")
var_tracking("eta")
tracking_resolution("eta", "MC", "", "")
tracking_resolution("impactParameter","", "RPhi", "Pt")
tracking_resolution("impactParameter","", "RPhi", "Phi")
tracking_resolution("impactParameter","", "RPhi", "Eta")
tracking_resolution("impactParameter","", "Z", "Pt")
tracking_resolution("impactParameter","", "Z", "Phi")
tracking_resolution("impactParameter","", "Z", "Eta")
tracking_resolution("impactParameterError","", "RPhi", "Pt")
tracking_resolution("impactParameterError","", "RPhi", "Phi")
tracking_resolution("impactParameterError","", "RPhi", "Eta")
tracking_resolution("impactParameterError","", "Z", "Pt")
tracking_resolution("impactParameterError","", "Z", "Phi")
tracking_resolution("impactParameterError","", "Z", "Eta")
tracking_resolution("pt", "", "Eta", "")
#efficiencyhadron("lc", "Pt")
