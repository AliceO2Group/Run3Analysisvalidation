#!/usr/bin/env python
from ROOT import TH1F, TCanvas, TEfficiency, TFile, TLatex, TLegend, gPad, gStyle


def saveCanvas(canvas, title):
    format_list = [".png", ".pdf", ".root"]
    for fileFormat in format_list:
        canvas.SaveAs(title + fileFormat)


def efficiencytracking(fileo2, var):
    # plots the efficiency vs pT, eta and phi for all the species(it extracts the
    # Efficiency from qa - tracking - efficiency if you have ran with-- make - eff)
    hadron_list = [
        "pion",
        "proton",
        "kaon",
        "electron",
        "muon",
    ]
    color_list = [1, 2, 4, 6, 8]
    marker_list = [20, 21, 22, 34, 45]
    # fileo2 = TFile("../codeHF/AnalysisResults_O2_pT1_piel_rebin.root")

    c1 = TCanvas("c1", "Efficiency")
    gStyle.SetOptStat(0)
    gStyle.SetErrorX(0)
    gStyle.SetFrameLineWidth(2)
    gStyle.SetTitleSize(0.045, "x")
    gStyle.SetTitleSize(0.045, "y")
    gStyle.SetMarkerSize(1)
    gStyle.SetLabelOffset(0.015, "x")
    gStyle.SetLabelOffset(0.02, "y")
    gStyle.SetTickLength(-0.02, "x")
    gStyle.SetTickLength(-0.02, "y")
    gStyle.SetTitleOffset(1.1, "x")
    gStyle.SetTitleOffset(1.0, "y")

    c1.SetCanvasSize(800, 600)
    c1.cd()
    c1.SetGridy()
    c1.SetGridx()
    eff_list = []

    if var == "Pt":
        hempty = TH1F(
            "hempty", ";Transverse Momentum(GeV/#it{c});Efficiency", 100, 0.05, 10
        )
        gPad.SetLogx()
    elif var == "Eta":
        hempty = TH1F("hempty", ";Pseudorapidity;Efficiency", 100, -4.0, 4.0)
    elif var == "Phi":
        hempty = TH1F("hempty", ";Azimuthal angle(rad);Efficiency", 100, 0.0, 6.0)

    hempty.GetYaxis().SetRangeUser(0, 1.15)
    # if var == "Pt":
    #    hempty.GetXaxis().SetRangeUser(0.0001, 10)
    hempty.GetYaxis().CenterTitle()
    hempty.GetXaxis().CenterTitle()
    hempty.GetXaxis().SetNoExponent()
    hempty.GetXaxis().SetMoreLogLabels(1)
    hempty.Draw()
    leg = TLegend(0.4, 0.12, 0.9, 0.35, "P")
    leg.SetNColumns(2)
    leg.SetTextSize(0.04)
    leg.SetHeader("PYTHIA 8.2 Kr-Kr #sqrt{#it{s}_{NN}} = 6.46 TeV", "C")
    leg.SetFillColor(0)
    latex = TLatex()
    latex0 = TLatex()
    latex.SetTextAlign(12)
    latex.SetTextFont(62)
    latex.DrawLatex(0.09, 1.05, "ALICE 3 Projections")
    latex0.SetTextAlign(12)
    latex0.SetTextFont(42)
    latex0.SetTextSize(0.045)
    latex0.DrawLatex(0.71, 0.45, "#it{R}_{innermost layer} = 5mm, |#eta|< 1")

    for i, had in enumerate(hadron_list):
        leff = fileo2.Get("qa-tracking-efficiency-%s/Efficiency" % had)
        if var == "Pt":
            eff = leff.At(0)
        elif var == "Eta":
            eff = leff.At(1)
        elif var == "Phi":
            eff = leff.At(2)
        gPad.Update()
        eff.Paint("p")
        gr = eff.GetPaintedGraph().Clone()
        for j in range(0, gr.GetN()):
            gr.GetEXlow()[j] = 0
            gr.GetEXhigh()[j] = 0

        gr.SetLineColor(color_list[i])
        gr.SetMarkerColor(color_list[i])
        gr.SetMarkerStyle(marker_list[i])
        eff_list.append(gr)
        gr.Draw(" same p")
        leg.AddEntry(eff_list[i], had, "p")
    leg.Draw()
    saveCanvas(c1, "efficiency_tracking_%s_KrKr_eta01_final" % var)


def efficiencyhadron(had, var):
    # extract the efficiency vs pT for single species(D0, Lc, Jpsi)
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")
    ceffhf = TCanvas("ceffhf", "A Simple Graph Example")
    ceffhf.SetCanvasSize(1500, 700)
    ceffhf.Divide(2, 1)
    gPad.SetLogy()
    # hnum = fileo2.Get("qa-tracking-efficiency-%s/%s/num" % (had, var))
    hnum = fileo2.Get("hf-task-%s-mc/h%sRecSig" % (had, var))
    # hden = fileo2.Get("qa-tracking-efficiency-%s/%s/den" % (had, var))
    hden = fileo2.Get("hf-task-%s-mc/h%sGen" % (had, var))
    hnum.Rebin(4)
    hden.Rebin(4)
    eff = TEfficiency(hnum, hden)
    eff.Draw()
    saveCanvas(ceffhf, "efficiency_hfcand%s%s" % (had, var))


def plot_together(file1, file2):
    hadron_list = [
        "pion",
        "electron",
    ]
    color_list = [4, 6, 5, 7]
    marker_list = [4, 4, 22, 22]
    c1 = TCanvas("c1", "Efficiency")
    gStyle.SetOptStat(0)
    gStyle.SetErrorX(0)
    gStyle.SetFrameLineWidth(2)
    gStyle.SetTitleSize(0.045, "x")
    gStyle.SetTitleSize(0.045, "y")
    gStyle.SetMarkerSize(0.5)
    gStyle.SetLabelOffset(0.015, "x")
    gStyle.SetLabelOffset(0.02, "y")
    gStyle.SetTickLength(-0.02, "x")
    gStyle.SetTickLength(-0.02, "y")
    gStyle.SetTitleOffset(1.1, "x")
    gStyle.SetTitleOffset(1.0, "y")
    c1.SetCanvasSize(800, 600)
    c1.cd()
    c1.SetGridy()
    c1.SetGridx()
    eff_list = []
    hempty = TH1F("hempty", ";Pseudorapidity;Efficiency", 100, -4.0, 4.0)
    hempty.GetYaxis().SetRangeUser(0, 1.3)
    hempty.GetYaxis().CenterTitle()
    hempty.GetXaxis().CenterTitle()
    hempty.GetXaxis().SetNoExponent()
    hempty.GetXaxis().SetMoreLogLabels(1)
    hempty.Draw()
    leg = TLegend(0.5, 0.11, 0.9, 0.4, "P")
    leg.SetNColumns(2)
    leg.SetTextSize(0.03)
    leg.SetHeader("PYTHIA 8.2   Kr-Kr #sqrt{#it{s}_{NN}} = 6.46 TeV", "C")
    leg.SetFillColor(0)
    latex = TLatex()
    latex0 = TLatex()
    latex.SetTextAlign(12)
    latex.SetTextFont(62)
    latex.DrawLatex(-3.9, 1.22, "ALICE 3 Projections")
    latex0.SetTextAlign(12)
    latex0.SetTextFont(42)
    latex0.SetTextSize(0.045)
    latex0.DrawLatex(-3.9, 1.12, "#it{R}_{innermost layer} = 5mm")
    for i, had in enumerate(hadron_list):
        leff = file1.Get("qa-tracking-efficiency-%s/Efficiency" % had)
        eff = leff.At(1)
        gPad.Update()
        eff.Paint("p")
        gr = eff.GetPaintedGraph().Clone()
        for j in range(0, gr.GetN()):
            gr.GetEXlow()[j] = 0
            gr.GetEXhigh()[j] = 0

        gr.SetLineColor(color_list[i])
        gr.SetMarkerColor(color_list[i])
        gr.SetMarkerStyle(marker_list[i])
        eff_list.append(gr)
        gr.Draw(" same p")
        leg.AddEntry(eff_list[i], had + "_{#it{p}_{T} > 0.3 GeV/#it{c}}", "p")
    for i, had in enumerate(hadron_list):
        leff2 = file2.Get("qa-tracking-efficiency-%s/Efficiency" % had)
        eff2 = leff2.At(1)
        gPad.Update()
        eff2.Paint("p")
        gr2 = eff2.GetPaintedGraph().Clone()
        for j in range(0, gr2.GetN()):
            gr2.GetEXlow()[j] = 0
            gr2.GetEXhigh()[j] = 0

        gr2.SetLineColor(color_list[i])
        gr2.SetMarkerColor(color_list[i])
        gr2.SetMarkerStyle(marker_list[i + 2])
        eff_list.append(gr2)
        gr2.Draw(" same p")
        leg.AddEntry(eff_list[i + 2], had + "_{#it{p}_{T} > 1 GeV/#it{c}}", "p")
        # leg.AddEntry(gr2, "pion #it{p} > 1 GeV/c", "p")

    leg.Draw()
    saveCanvas(c1, "efficiency_tracking_eta_KrKr_werner_final")


# fileo2 = TFile("/home/mmazzill/alice/Run3Analysisvalidation/codeHF/AnalysisResults_O2_pt03.root")
# fileo2_2 = TFile("/home/mmazzill/alice/Run3Analysisvalidation/codeHF/AnalysisResults_O2_pt1_pp.root")

fileo2 = TFile(
    "/data/Run5/plots/krkr_0428/KrKr_6p460TeV_inel_200K_sc3_werner_22042021_pt0.3_eta2.root"
)
fileo2_2 = TFile(
    "/data/Run5/plots/krkr_0428/KrKr_6p460TeV_inel_200K_sc3_werner_22042021_pt1_eta2.root"
)
plot_together(fileo2, fileo2_2)

fileo2 = TFile(
    "/home/mmazzill/alice/Run3Analysisvalidation/codeHF/AnalysisResults_O2_KrKr_eta01.root"
)
# fileo2_2 = TFile("../codeHF/AnalysisResults_O2_pT1_piel_rebin.root")
var_list = ["Pt", "Eta", "Phi"]
hfhadron_list = ["d0", "dplus", "lc", "xic", "jpsi"]

for var in var_list:
    efficiencytracking(fileo2, var)

efficiencytracking(fileo2_2, "eta")

# for had in hfhadron_list:
#    efficiencyhadron(had, "Pt")
