from ROOT import TCanvas, TFile, TGaxis, TLegend,gStyle


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
    cparticle.Divide(2,1)
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
    cpt.SetCanvasSize(1500, 1500)
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
    TGaxis.SetMaxDigits(3)
    components_list = ["x", "y", "z"]
    color_list = [1, 2, 4]
    csecvertex = TCanvas("c1", "Secondary Vertex Difference")
    csecvertex.SetCanvasSize(1000, 700)
    csecvertex.cd()
    secvertex_list = []
    leg = TLegend(0.1, 0.7, 0.4, 0.9, "Sec. Vertex:")
    leg.SetFillColor(0)
    for i, comp in enumerate(components_list):
        hsecvertex = file.Get("hf-mc-validation-rec/histSecV%s" % comp)
        hsecvertex.Rebin(1)
        hsecvertex.SetLineColor(color_list[i])
        hsecvertex.SetLineWidth(2)
        secvertex_list.append(hsecvertex)
        if i != 0:
            hsecvertex.Draw("same")
        else:
            hsecvertex.SetTitle("Secondary Vertex difference Rec-Gen")
            hsecvertex.GetXaxis().SetTitle("V_{i}^{reco}-V_{i}^{gen} (cm)")
            hsecvertex.Draw()
        leg.AddEntry(secvertex_list[i], "%s component" % comp)
    leg.Draw()
    printCanvas(csecvertex, "SecVertex_recoMC_diff")


def decayLength_reco_MC(file):
    cDecLenRes = TCanvas("cDecLenRes", "Decay Length")
    cDecLenRes.SetCanvasSize(900, 700)
    hDecLenRes = file.Get("hf-mc-validation-rec/histDecLen")
    hDecLenRes.Rebin(2)
    hDecLenRes.Draw()
    printCanvas(cDecLenRes, "Resolution_DecayLength")


fileo2 = TFile("../codeHF/CCbar_pp_AnalysisResults_O2_500files.root")
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
