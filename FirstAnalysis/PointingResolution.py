#!/usr/bin/env python
from array import array

from ROOT import TF1, TCanvas, TFile, TGaxis, TLatex, TLegend, TMath, TObjArray, gStyle


def printCanvas(canvas, title):
    format_list = [".png", ".pdf", ".root"]
    for fileFormat in format_list:
        canvas.SaveAs(title + fileFormat)


def ImpactParResolutionRPhiVsPt(filename, tag="", fit_range=20):
    gStyle.SetOptStat(0)
    f = TFile(filename, "READ")
    cres = TCanvas("cres", "resolution distribution")
    cres.SetCanvasSize(1500, 700)
    canv = f.Get("cres")
    g = canv.FindObject("impactParameterRPhiVsPt")
    f.Close()
    tit = g.GetTitle()
    g.SetTitle(tit + tag)
    a = TObjArray()
    fun = TF1("gaus", "gaus", -fit_range, fit_range)
    g.FitSlicesY(fun, 0, -1, 0, "QNR", a)
    for i in a:
        i.SetName(i.GetName() + tag)
        i.SetTitle(tag)
    a[2].SetLineColor(2)
    a[2].SetMarkerStyle(20)
    a[2].SetMarkerColor(2)
    gStyle.SetErrorX(0)
    cRPhiVsPt = TCanvas("cRPhiVsPt", "impactParameterRPhiVsPt")
    cRPhiVsPt.SetCanvasSize(900, 700)
    cRPhiVsPt.Divide(2, 1)
    cRPhiVsPt.SetGridx()
    cRPhiVsPt.SetGridy()
    cRPhiVsPt.SetLogx()
    cRPhiVsPt.SetLogy()
    # a[2].Scale(10)
    a[2].SetTitle("")
    a[2].GetXaxis().SetRangeUser(0.1, 10)
    a[2].GetYaxis().SetRangeUser(0.8, 14)
    a[2].GetXaxis().SetTitle("Transverse Momentum (GeV/#it{c})")
    a[2].GetXaxis().SetTitleOffset(1.2)
    a[2].GetXaxis().CenterTitle()
    a[2].GetYaxis().SetTitle("R-#phi Pointing Resolution (#mum)")
    a[2].GetYaxis().CenterTitle()
    a[2].Draw("")

    printCanvas(cRPhiVsPt, "RPhiPointingResolution%s" % tag)


def plot_together(f1, f2, var):
    fpp = TFile(f1, "READ")
    fKr = TFile(f2, "READ")
    c1 = fpp.Get("c%sVsPt" % var)
    c2 = fKr.Get("c%sVsPt" % var)
    h1 = c1.FindObject("impactParameter%sVsPt_2pp" % var)
    h2 = c2.FindObject("impactParameter%sVsPt_2KrKr" % var)
    # h1.Rebin(2)
    # h2.Rebin(2)
    h1.GetYaxis().SetRangeUser(0.9, 30)
    h1.GetXaxis().SetRangeUser(0.1, 100)
    cp_res = TCanvas("cp_res", "impactParameter%sVsPt" % var)
    cp_res.SetCanvasSize(900, 700)
    cp_res.SetGridx()
    cp_res.SetGridy()
    cp_res.SetLogx()
    cp_res.SetLogy()

    nbins = h1.GetNbinsX()
    binp = array("d")
    max = h1.GetXaxis().GetBinUpEdge(nbins)
    min = h1.GetXaxis().GetBinLowEdge(1)
    if min <= 0:
        min = 0.01
    lmin = TMath.Log10(min)
    ldelta = (TMath.Log10(max) - lmin) / (nbins)
    for i in range(0, nbins):
        binp.insert(i, TMath.Exp(TMath.Log(10) * (lmin + i * ldelta)))
    binp.insert(nbins, max + 1)
    h1.GetXaxis().Set(nbins, binp)
    h2.GetXaxis().Set(nbins, binp)

    h1.SetLineColor(4)
    h1.SetMarkerStyle(20)
    h1.SetMarkerColor(4)
    h1.Draw()
    h2.Draw("same")
    leg = TLegend(0.11, 0.12, 0.5, 0.3)
    leg.SetHeader("PYTHIA 8.2 Monash 2013", "C")
    leg.SetTextSize(0.035)
    leg.AddEntry(h1, "pp #sqrt{#it{s}} = 14 TeV")
    leg.AddEntry(h2, "Kr-Kr #sqrt{#it{s}_{NN}} = 6.46 TeV")
    leg.Draw("same")
    latex = TLatex()
    latex0 = TLatex()
    latex1 = TLatex()
    latex.SetTextAlign(12)
    latex.SetTextFont(62)
    latex.DrawLatex(0.5, 22, "ALICE 3 Projections")
    latex1.SetTextAlign(12)
    latex1.SetTextFont(42)
    latex1.SetTextSize(0.04)
    latex1.DrawLatex(0.5, 15, "#it{R}_{innermost layer} = 5mm, |#eta|< 2")
    latex0.SetTextAlign(12)
    latex0.SetTextFont(42)
    # latex0.DrawLatex(1.8,15, "|#eta|< 2")
    printCanvas(cp_res, "%sPointingResolutionTOT_log_novtx" % var)


def ImpactParResolutionZVsPt(filename, tag="", fit_range=20):
    gStyle.SetOptStat(0)
    fileo2 = TFile(filename, "READ")
    canv = fileo2.Get("cres")
    g = canv.GetPrimitive("impactParameterZVsPt")
    fileo2.Close()
    tit = g.GetTitle()
    g.SetTitle(tit + tag)
    a = TObjArray()
    fun = TF1("gaus", "gaus", -fit_range, fit_range)
    g.FitSlicesY(fun, 0, -1, 0, "QNR", a)
    for i in a:
        i.SetName(i.GetName() + tag)
        i.SetTitle(tag)
    a[1].SetLineColor(3)
    a[2].SetLineColor(2)
    a[2].SetMarkerStyle(20)
    a[2].SetMarkerColor(2)
    cZVsPt = TCanvas("cZVsPt", "impactParameterZVsPt")
    cZVsPt.SetCanvasSize(900, 700)
    cZVsPt.Divide(2, 1)
    cZVsPt.SetGridx()
    cZVsPt.SetGridy()
    cZVsPt.SetLogx()
    cZVsPt.SetLogy()
    a[2].SetTitle("")
    a[2].GetXaxis().SetRangeUser(0.1, 10)
    a[2].GetYaxis().SetRangeUser(0.8, 40)
    a[2].GetXaxis().SetTitle("Transverse Momentum (GeV/#it{c})")
    a[2].GetXaxis().SetTitleOffset(1.2)
    a[2].GetXaxis().CenterTitle()
    a[2].GetYaxis().SetTitle("Z Pointing Resolution (#mum)")
    a[2].GetYaxis().CenterTitle()
    a[2].Draw("")
    printCanvas(cZVsPt, "ZPointingResolution%s" % tag)


def PtResolutionVsVar(filename, var, tag=""):
    gStyle.SetOptStat(0)
    gStyle.SetOptTitle(0)
    TGaxis.SetMaxDigits(3)
    fileo2 = TFile(filename, "READ")
    canv = fileo2.Get("cres")
    g = canv.GetPrimitive("ptResolutionVs%s" % var)
    fileo2.Close()
    tit = g.GetTitle()
    g.SetTitle(tit + tag)
    a = TObjArray()
    g.FitSlicesY(0, 0, -1, 0, "QNR", a)
    for i in a:
        i.SetName(i.GetName() + tag)
        i.SetTitle(tag)
    a[1].SetLineColor(3)
    a[2].SetLineColor(2)
    a[2].SetMarkerStyle(20)
    a[2].SetMarkerColor(2)
    cPtVsPt = TCanvas("cPtResVsPt", "ptResolutionVsPt")
    cPtVsPt.SetCanvasSize(900, 700)
    cPtVsPt.Divide(2, 1)
    cPtVsPt.SetGridx()
    cPtVsPt.SetGridy()
    if var == "Pt":
        cPtVsPt.SetLogx()
    cPtVsPt.SetLogy()
    if var == "Pt":
        a[2].GetXaxis().SetRangeUser(0.1, 11)
        a[2].GetYaxis().SetRangeUser(0.01, 0.2)
    if var == "Eta":
        a[2].GetYaxis().SetRangeUser(0.008, 0.2)
    a[2].GetXaxis().SetTitle("Transverse Momentum (GeV/#it{c})")
    a[2].GetXaxis().SetTitleOffset(1.0)
    a[2].GetXaxis().CenterTitle()
    # a[2].GetYaxis().SetTitle("Transverse momentum resolution")
    if var == "Pt":
        a[2].GetXaxis().SetTitle("#eta")
    a[2].GetYaxis().SetLabelSize(0.04)
    a[2].GetYaxis().CenterTitle()
    a[2].Draw("")
    printCanvas(cPtVsPt, "PtResolutionVs%s%s" % (var, tag))


def plot_together2(f1, f2, var):
    gStyle.SetOptStat(0)
    gStyle.SetOptTitle(0)
    TGaxis.SetMaxDigits(3)
    fpp = TFile(f1, "READ")
    fKr = TFile(f2, "READ")
    c1 = fpp.Get("cPtResVsPt")
    c2 = fKr.Get("cPtResVsPt")
    h1 = c1.FindObject("ptResolutionVs%s_2pp" % var)
    h2 = c2.FindObject("ptResolutionVs%s_2KrKr" % var)
    cp_res = TCanvas("cp_res", "PtResolutionVsVar")
    cp_res.SetCanvasSize(900, 700)
    cp_res.SetGridx()
    cp_res.SetGridy()
    if var == "Pt":
        cp_res.SetLogx()
    cp_res.SetLogy()

    if var == "Pt":
        h1.GetXaxis().SetTitle("Transverse momentum (GeV/#it{c})")
        h1.GetYaxis().SetTitle("Transverse momentum resolution")
    if var == "Eta":
        h1.GetXaxis().SetTitle("Pseudorapidity #eta (rad)")
    h1.GetYaxis().SetTitleOffset(1.0)
    h1.GetYaxis().SetTitleSize(0.045)
    h1.GetXaxis().SetTitleOffset(1.0)
    h1.GetXaxis().SetTitleSize(0.045)
    h1.SetLineColor(4)
    h1.SetMarkerStyle(20)
    h1.SetMarkerColor(4)
    h1.Draw()
    h2.Draw("same")
    leg = TLegend(0.11, 0.12, 0.5, 0.3)
    leg.SetHeader("PYTHIA 8.2 Monash 2013", "C")
    leg.SetTextSize(0.035)
    leg.AddEntry(h1, "pp #sqrt{#it{s}} = 14 TeV")
    leg.AddEntry(h2, "Kr-Kr #sqrt{#it{s}_{NN}} = 6.46 TeV")
    leg.Draw("same")
    latex = TLatex()
    latex0 = TLatex()
    latex1 = TLatex()
    latex.SetTextAlign(12)
    latex.SetTextFont(62)
    if var == "Pt":
        latex.DrawLatex(0.85, 0.15, "ALICE 3 Projections")
    if var == "Eta":
        latex.DrawLatex(0.0, 0.15, "ALICE 3 Projections")
    latex1.SetTextAlign(12)
    latex1.SetTextFont(42)
    latex1.SetTextSize(0.04)
    if var == "Pt":
        latex1.DrawLatex(0.85, 0.12, "#it{R}_{innermost layer} = 5mm, |#eta|< 2")
    if var == "Eta":
        latex1.DrawLatex(0.0, 0.12, "#it{R}_{innermost layer} = 5mm")
    latex0.SetTextAlign(12)
    latex0.SetTextFont(42)
    # latex0.DrawLatex(1.8,15, "|#eta|< 2")
    printCanvas(cp_res, "%sPtResolutionTOT" % var)


ImpactParResolutionRPhiVsPt(
    "impact_parameter_resolutionRPhiVsPt_pp_novtx.root", "pp", 20
)
ImpactParResolutionRPhiVsPt(
    "impact_parameter_resolutionRPhiVsPt_KrKr_novtx.root", "KrKr", 10
)
PtResolutionVsVar("ptResolutionVsPt_KrKr_werner_2000.root", "Pt", "KrKr")
PtResolutionVsVar("ptResolutionVsPt_pp_werner.root", "Pt", "pp")
PtResolutionVsVar("ptResolutionVsEta_KrKr_werner_2000.root", "Eta", "KrKr")
PtResolutionVsVar("ptResolutionVsEta_pp_werner.root", "Eta", "pp")
plot_together(
    "RPhiPointingResolutionpp.root", "RPhiPointingResolutionKrKr.root", "RPhi"
)
plot_together2("PtResolutionVsPtpp.root", "PtResolutionVsPtKrKr.root", "Pt")
plot_together2("PtResolutionVsEtapp.root", "PtResolutionVsEtaKrKr.root", "Eta")

# p2 = ImpactParResolutionRPhiVsPt("../codeHF/AnalysisResults_O2.root","KrKr", 10)
# plot_together([p1, p2])
ImpactParResolutionZVsPt("impact_parameter_resolutionZVsPt_pp_novtx.root", "pp", 20)
ImpactParResolutionZVsPt("impact_parameter_resolutionZVsPt_KrKr_novtx.root", "KrKr", 10)
plot_together("ZPointingResolutionpp.root", "ZPointingResolutionKrKr.root", "Z")
