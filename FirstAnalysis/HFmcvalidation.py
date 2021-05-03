#!/usr/bin/env python
from ROOT import TCanvas, TFile, TGaxis, TLatex, TLegend, gStyle


def printCanvas(canvas, title):
    format_list = [".png", ".pdf", ".root"]
    for fileFormat in format_list:
        canvas.SaveAs(title + fileFormat)


def count_HFquarks_perColl(file, var):
    cquark = TCanvas("cquark", "%s per collision" % var)
    cquark.SetCanvasSize(900, 700)
    cquark.Divide(2, 1)
    hq = file.Get("hf-mc-validation-gen/hCountAverage%s" % var)
    hq.GetXaxis().SetRangeUser(0.0, 8.0)
    hq.SetTitle("Number of %s quarks per collision" % var)
    hq.Draw("")
    printCanvas(cquark, "QuarkCountsPerCollision%s" % var)


def count_Particle_perColl(file, var):
    cparticle = TCanvas("cparticle", "%s per collision" % var)
    cparticle.SetCanvasSize(900, 700)
    cparticle.Divide(2, 1)
    hp = file.Get("hf-mc-validation-gen/hCouterPerCollision%s" % var)
    hp.Draw("")
    printCanvas(cparticle, "%sPerCollision" % var)


def momentum_Conservation(file, var):
    cmomentum = TCanvas("cmomentum", "Momentum Conservation %s" % var)
    cmomentum.SetCanvasSize(900, 700)
    cmomentum.Divide(2, 1)
    if var == "P":
        hp = file.Get("hf-mc-validation-gen/h%sdiffMotherDaughterGen" % var)
        hp.SetTitle("Momentum Conservtion: magnitude (Gen)")
    else:
        hp = file.Get("hf-mc-validation-gen/h%sDiffMotherDaughterGen" % var)
        hp.SetTitle("Momentum Conservation: %s component" % var)
    hp.Draw("")
    printCanvas(cmomentum, "%sconservation" % var)


def momentum_check(file):
    cMomCheck = TCanvas("cMomCheck", "Momentum Conservation Check")
    cMomCheck.SetCanvasSize(900, 700)
    hMomCheck = file.Get("hf-mc-validation-gen/hMomentumCheck")
    hMomCheck.Draw()
    printCanvas(cMomCheck, "MomentumCheck")


def p_diff_reco_MC(file):
    gStyle.SetOptStat(0)
    components_list = ["x", "y", "z"]
    color_list = [1, 2, 4]
    cpt = TCanvas("c1", "Momentum Difference")
    cpt.SetCanvasSize(1000, 700)
    cpt.cd()
    mom_list = []
    leg = TLegend(0.1, 0.7, 0.4, 0.9, "")
    leg.SetFillColor(0)
    for i, comp in enumerate(components_list):
        hp = file.Get("hf-mc-validation-rec/histP%s" % comp)
        hp.Rebin(2)
        hp.SetLineColor(color_list[i])
        hp.SetLineWidth(2)
        mom_list.append(hp)
        if i != 0:
            hp.Draw("same")
        else:
            hp.SetTitle("Momentum difference Rec-Gen")
            hp.GetXaxis().SetTitle("p_{i}^{reco}-p_{i}^{gen} (MeV/#it{c})")
            hp.GetXaxis().SetRangeUser(-0.3, 0.3)
            hp.Draw()
        leg.AddEntry(mom_list[i], "P%s component" % comp)
    leg.Draw()
    printCanvas(cpt, "p_recoMC_diff")


def secondary_vertex_reco_MC(file):
    gStyle.SetOptStat(0)
    gStyle.SetOptTitle(0)
    TGaxis.SetMaxDigits(3)
    components_list = ["x", "y", "z"]
    color_list = [1, 2, 4]
    line_style_list = [6, 2, 3]
    csecvertex = TCanvas("c1", "Secondary Vertex Difference")
    csecvertex.SetCanvasSize(1000, 700)
    csecvertex.SetLogy()
    csecvertex.cd()
    secvertex_list = []
    leg = TLegend(0.7, 0.7, 0.9, 0.9)
    leg.SetTextSize(0.04)
    leg.SetFillColor(0)
    for i, comp in enumerate(components_list):
        hsecvertex = file.Get("hf-mc-validation-rec/histSecV%s" % comp)
        hsecvertex.Rebin(2)
        hsecvertex.SetLineColor(color_list[i])
        hsecvertex.SetLineWidth(3)
        hsecvertex.SetLineStyle(line_style_list[i])
        hsecvertex.GetXaxis().SetRangeUser(-0.01, 0.01)
        hsecvertex.GetYaxis().SetRangeUser(0.1, 1200)
        secvertex_list.append(hsecvertex)
        if i != 0:
            hsecvertex.Draw("same")
            hsecvertex.Fit("gaus", "", "", -0.001, 0.001)
        else:
            hsecvertex.GetYaxis().SetTitle("Counts")
            hsecvertex.GetYaxis().SetTitleSize(0.05)
            hsecvertex.GetYaxis().SetTitleOffset(0.8)
            hsecvertex.GetXaxis().SetTitle("vtx_{reco}^{sec}-vtx_{gen}^{sec} (cm)")
            hsecvertex.GetXaxis().SetTitleSize(0.05)
            hsecvertex.GetXaxis().SetTitleOffset(0.8)
            # hsecvertex.Fit("gaus")
            hsecvertex.Draw("")
        leg.AddEntry(
            secvertex_list[i],
            "#it{%s}_{reco}^{sec} - #it{%s}_{gen}^{sec}" % (comp, comp),
        )
    leg.Draw("p")
    latex = TLatex()
    latex0 = TLatex()
    latex1 = TLatex()
    latex2 = TLatex()
    latex3 = TLatex()
    latex4 = TLatex()
    latex.SetTextAlign(12)
    latex.SetTextFont(62)
    latex.DrawLatex(-0.0085, 600, "ALICE 3 Projections")
    latex1.SetTextAlign(12)
    latex1.SetTextFont(42)
    latex1.SetTextSize(0.045)
    latex1.DrawLatex(-0.0085, 350, "PYTHIA 8.2")
    latex0.SetTextAlign(12)
    latex0.SetTextSize(0.045)
    latex0.SetTextFont(42)
    latex0.DrawLatex(-0.0085, 200, "pp #sqrt{#it{s}} = 14 TeV")
    latex2.SetTextAlign(12)
    latex2.SetTextSize(0.045)
    latex2.SetTextFont(42)
    latex2.DrawLatex(-0.0085, 100, "#it{R}_{innermost layer} = 5mm")
    latex3.SetTextAlign(12)
    latex3.SetTextSize(0.045)
    latex3.SetTextFont(42)
    latex3.DrawLatex(0.0032, 55, "#Lambda_{c}#rightarrow pK#pi, |#eta|< 2")
    latex4.SetTextAlign(12)
    latex4.SetTextSize(0.045)
    latex4.SetTextFont(42)
    latex4.DrawLatex(0.0032, 20, "4 < #it{p}_{T}(#Lambda_{c})< 6 GeV/#it{c}")
    printCanvas(csecvertex, "SecVertex_recoMC_diff_last_4_6")


def decayLength_reco_MC(file):
    cDecLenRes = TCanvas("cDecLenRes", "Decay Length")
    cDecLenRes.SetCanvasSize(900, 700)
    hDecLenRes = file.Get("hf-mc-validation-rec/histDecLen")
    hDecLenRes.Rebin(2)
    hDecLenRes.Draw()
    printCanvas(cDecLenRes, "Resolution_DecayLength")


# fileo2 = TFile("/data/Run5/plots/new_pro/AnalysisResults_O2_pp14TeV_ccbar_10M_sc3werner_20042021.root")
fileo2 = TFile(
    "../codeHF/AnalysisResults_O2_LcpKpi_4_6.root"
)
# Generated Level Validation
quark_list = ["C", "Cbar", "B", "Bbar"]
for quark in quark_list:
    count_HFquarks_perColl(fileo2, quark)
particle_list = ["Dzero", "Dplus", "Dstar", "LambdaC"]
for particle in particle_list:
    count_Particle_perColl(fileo2, particle)
momentum_check(fileo2)
mom_list = ["P", "Px", "Py", "Pz", "Pt"]
for comp in mom_list:
    momentum_Conservation(fileo2, comp)
# Reconstructed Level Validation
p_diff_reco_MC(fileo2)
secondary_vertex_reco_MC(fileo2)
decayLength_reco_MC(fileo2)
