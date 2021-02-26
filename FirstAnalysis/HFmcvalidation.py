from ROOT import TCanvas, TFile, TGaxis, TLegend, gStyle


def count_HFquarks_perColl(var):
    fileo2 = TFile("../codeHF/CCbar_pp_AnalysisResults_O2_500files.root")
    cquark = TCanvas("cquark", "%s per collision" % var)
    cquark.SetCanvasSize(900, 700)
    cquark.Divide(2, 1)
    hq = fileo2.Get("hf-mc-validation-gen/hCountAverage%s" % var)
    hq.GetXaxis().SetRangeUser(0.0, 8.0)
    hq.SetTitle("Number of %s quarks per collision" % var)
    hq.Draw("")
    cquark.SaveAs("QuarkCountsPerCollision%s.pdf" % var)
    cquark.SaveAs("QuarkCountsPerCollision%s.root" % var)
    cquark.SaveAs("QuarkCountsPerCollision%s.png" % var)


def count_Particle_perColl(var):
    fileo2 = TFile("../codeHF/CCbar_pp_AnalysisResults_O2_500files.root")
    cparticle = TCanvas("cparticle", "%s per collision" % var)
    cparticle.SetCanvasSize(900, 700)
    cparticle.Divide(2, 1)
    hp = fileo2.Get("hf-mc-validation-gen/hCouterPerCollision%s" % var)
    hp.Draw("")
    cparticle.SaveAs("%sPerCollision.pdf" % var)
    cparticle.SaveAs("%sPerCollision.root" % var)
    cparticle.SaveAs("%sPerCollision.png" % var)


def momentum_Conservation(var):
    fileo2 = TFile("../codeHF/CCbar_pp_AnalysisResults_O2_500files.root")
    cmomentum = TCanvas("cmomentum", "Momentum Conservation %s" % var)
    cmomentum.SetCanvasSize(900, 700)
    cmomentum.Divide(2, 1)
    if var == "P":
        hp = fileo2.Get("hf-mc-validation-gen/h%sdiffMotherDaughterGen" % var)
        hp.SetTitle("Momentum Conservtion: magnitude (Gen)")
    else:
        hp = fileo2.Get("hf-mc-validation-gen/h%sDiffMotherDaughterGen" % var)
        hp.SetTitle("Momentum Conservation: %s component" % var)
    hp.Draw("")
    cmomentum.SaveAs("%sconservation.pdf" % var)
    cmomentum.SaveAs("%sconservation.root" % var)
    cmomentum.SaveAs("%sconservation.png" % var)


def momentum_check():
    fileo2 = TFile("../codeHF/CCbar_pp_AnalysisResults_O2_500files.root")
    cMomCheck = TCanvas("cMomCheck", "Momentum Conservation Check")
    cMomCheck.SetCanvasSize(900, 700)
    hMomCheck = fileo2.Get("hf-mc-validation-gen/hMomentumCheck")
    hMomCheck.Draw()
    cMomCheck.SaveAs("MomentumCheck.pdf")
    cMomCheck.SaveAs("MomentumCheck.root")
    cMomCheck.SaveAs("MomentumCheck.png")


def p_diff_reco_MC():
    gStyle.SetOptStat(0)
    components_list = ["x", "y", "z"]  # , "electron", "muon"]
    color_list = [1, 2, 4]
    fileo2 = TFile("../codeHF/CCbar_pp_AnalysisResults_O2_500files.root")
    cpt = TCanvas("c1", "Momentum Difference")
    cpt.SetCanvasSize(1500, 1500)
    cpt.cd()
    mom_list = []
    leg = TLegend(0.1, 0.7, 0.4, 0.9, "")
    leg.SetFillColor(0)
    for i, comp in enumerate(components_list):
        hp = fileo2.Get("hf-mc-validation-rec/histP%s" % comp)
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
    cpt.SaveAs("p_recoMC_diff.pdf")
    cpt.SaveAs("p_recoMC_diff.png")
    cpt.SaveAs("p_recoMC_diff.root")


def secondary_vertex_reco_MC():
    gStyle.SetOptStat(0)
    TGaxis.SetMaxDigits(3)
    components_list = ["x", "y", "z"]  # , "electron", "muon"]
    color_list = [1, 2, 4]
    fileo2 = TFile("../codeHF/CCbar_pp_AnalysisResults_O2_500files.root")
    csecvertex = TCanvas("c1", "Secondary Vertex Difference")
    csecvertex.SetCanvasSize(1000, 700)
    csecvertex.cd()
    secvertex_list = []
    leg = TLegend(0.1, 0.7, 0.4, 0.9, "Sec. Vertex:")
    leg.SetFillColor(0)
    for i, comp in enumerate(components_list):
        hsecvertex = fileo2.Get("hf-mc-validation-rec/histSecV%s" % comp)
        hsecvertex.Rebin(1)
        hsecvertex.SetLineColor(color_list[i])
        hsecvertex.SetLineWidth(2)
        secvertex_list.append(hsecvertex)
        if i != 0:
            hsecvertex.Draw("same")
        else:
            hsecvertex.SetTitle("Secondary Vertex difference Rec-Gen")
            hsecvertex.GetXaxis().SetTitle("V_{i}^{reco}-V_{i}^{gen} (cm)")
            # hsecvertex.GetXaxis().SetRangeUser(-1.20, 1.10)
            hsecvertex.Draw()
        leg.AddEntry(secvertex_list[i], "%s component" % comp)
    leg.Draw()
    csecvertex.SaveAs("SecVertex_recoMC_diff.pdf")
    csecvertex.SaveAs("SecVertex_recoMC_diff.png")
    csecvertex.SaveAs("SecVertex_recoMC_diff.root")


def decayLength_reco_MC():
    # x, y, z
    fileo2 = TFile("../codeHF/CCbar_pp_AnalysisResults_O2_500files.root")
    cDecLenRes = TCanvas("cDecLenRes", "Decay Length")
    cDecLenRes.SetCanvasSize(900, 700)
    hDecLenRes = fileo2.Get("hf-mc-validation-rec/histDecLen")
    hDecLenRes.Rebin(2)
    hDecLenRes.Draw()
    cDecLenRes.SaveAs("Resolution_DecayLength.pdf")
    cDecLenRes.SaveAs("Resolution_DecayLength.root")
    cDecLenRes.SaveAs("Resolution_DecayLength.png")


count_HFquarks_perColl("C")
count_HFquarks_perColl("Cbar")
count_HFquarks_perColl("B")
count_HFquarks_perColl("Bbar")
count_Particle_perColl("Dzero")
count_Particle_perColl("Dplus")
count_Particle_perColl("Dstar")
count_Particle_perColl("LambdaC")
momentum_check()
momentum_Conservation("P")
momentum_Conservation("Px")
momentum_Conservation("Py")
momentum_Conservation("Pz")
momentum_Conservation("Pt")
p_diff_reco_MC()
secondary_vertex_reco_MC()
decayLength_reco_MC()
