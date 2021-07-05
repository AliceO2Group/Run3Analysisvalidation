#!/usr/bin/env python

import array

from ROOT import TCanvas, TFile, TLatex, TLegend, gPad, gROOT, gStyle

path_file = "../codeHF/AnalysisResults_O2.root"

gStyle.SetOptStat(0)
gStyle.SetFrameLineWidth(1)
gStyle.SetTitleSize(0.045, "x")
gStyle.SetTitleSize(0.045, "y")
gStyle.SetMarkerSize(1)
gStyle.SetLabelOffset(0.015, "x")
gStyle.SetLabelOffset(0.02, "y")
gStyle.SetTickLength(-0.02, "x")
gStyle.SetTickLength(-0.02, "y")
gStyle.SetTitleOffset(1.1, "x")
gStyle.SetTitleOffset(1.0, "y")

gROOT.SetBatch(True)


def saveCanvas(canvas, title):
    format_list = [".pdf", ".png"]
    for fileFormat in format_list:
        canvas.SaveAs(title + fileFormat)


def kinematic_plots(var, particle, detector, hp):
    fileo2 = TFile(path_file)
    cres = TCanvas("cres", "resolution distribution")
    cres.SetCanvasSize(1600, 1000)
    cres.cd()
    num = fileo2.Get(
        "qa-tracking-rejection-%s/tracking%ssel%s/%seta" % (particle, detector, hp, var)
    )
    den = fileo2.Get("qa-tracking-rejection-%s/tracking/%seta" % (particle, var))
    # gPad.SetLogz()
    num.Divide(den)
    num.Draw("coltz")
    num.GetYaxis().SetTitle("#eta")
    num.GetYaxis().SetTitleOffset(1.0)
    num.GetZaxis().SetRangeUser(0.01, 1)
    num.SetTitle("Fraction of %s selected by %s as %s" % (particle, detector, hp))
    saveCanvas(
        cres, "contamination/%seta_%sSelfrom%sas%s" % (var, particle, detector, hp)
    )


def ratioparticle(
    var="pt",
    numname="Electron",
    selnum="RICHSelHpElLoose",
    denname="Pion",
    selden="RICHSelHpElLoose",
    label="e/#pi",
):
    fileo2 = TFile(path_file)
    cres = TCanvas("cres", "resolution distribution")
    cres.SetCanvasSize(1600, 1000)
    cres.cd()
    cres.SetLogy()

    num2d = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (numname, selnum, var))
    den2d = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (denname, selden, var))
    num = num2d.ProjectionX("num", 1, num2d.GetXaxis().GetNbins())
    den = den2d.ProjectionX("den", 1, den2d.GetXaxis().GetNbins())
    num.Divide(den)
    num.Draw("coltz")
    num.GetYaxis().SetTitle(label)
    num.GetXaxis().SetTitle("p_{T}")
    # num.SetMinimum(0.001)
    # num.SetMaximum(2.0)
    num.GetYaxis().SetTitleOffset(1.0)
    # num.GetZaxis().SetRangeUser(0.01, 1)
    # nameresult = "Fraction of %s selected by %s over %s selected by %s" % (num,selnum,den,selden)
    canvas = "Fractionof%s%sOver%s%s" % (numname, selnum, denname, selden)
    # num.SetTitle(nameresult)
    saveCanvas(cres, "rejection/%s" % (canvas))


def is_e_not_pi_plots(particle):
    fileo2 = TFile(path_file)
    task = "qa-rejection-general"
    folder_gm = "h%sRICHSelHpElTight" % particle
    folder_alt = "h%sRICHSelHpElTightAlt" % particle
    folder_diff = "h%sRICHSelHpElTightAltDiff" % particle
    hist = "pteta"
    hist_gm = fileo2.Get("%s/%s/%s" % (task, folder_gm, hist))
    hist_gm.SetTitle("%s isRICHElTight" % particle)
    hist_alt = fileo2.Get("%s/%s/%s" % (task, folder_alt, hist))
    hist_alt.SetTitle("%s isElectronAndNotPion" % particle)
    hist_diff = fileo2.Get("%s/%s/%s" % (task, folder_diff, hist))
    hist_diff.SetTitle("%s isRICHElTight != isElectronAndNotPion" % particle)
    cepi = TCanvas("cepi", "e not pi selection")
    cepi.SetCanvasSize(1600, 1000)
    cepi.Divide(2, 2)
    cepi.cd(1)
    hist_gm.Draw("colz")
    cepi.cd(2)
    hist_alt.Draw("colz")
    cepi.cd(3)
    hist_diff.Draw("colz")
    # num.GetYaxis().SetTitleOffset(1.0)
    # num.GetZaxis().SetRangeUser(0.01, 1)
    saveCanvas(cepi, "contamination/is_e_not_pi_%s" % particle)


# caculate the contamination of particles for muon and el
def contaminationvspar(
    var="pt",
    lep="Electron",
    har1="Pion",
    all="All",
    har2="Kaon",
    pid="RICHSelHpElTightAlt",
    label="Contamination",
):
    fileo2 = TFile(path_file)
    cres = TCanvas("cres", "resolution distribution")
    cres.SetCanvasSize(1600, 1000)
    cres.cd()
    # cres.SetLogy()

    leg = TLegend(0.15, 0.45, 0.4, 0.65)
    leg.SetFillColor(0)
    leg.SetBorderSize(0)

    num2d = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (har1, pid, var))
    den2d = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (all, pid, var))
    num = num2d.ProjectionX("num", 1, num2d.GetXaxis().GetNbins())
    den = den2d.ProjectionX("den", 1, den2d.GetXaxis().GetNbins())

    num2d1 = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (lep, pid, var))
    den2d1 = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (all, pid, var))
    num1 = num2d1.ProjectionX("num1", 1, num2d1.GetXaxis().GetNbins())
    den1 = den2d1.ProjectionX("den1", 1, den2d1.GetXaxis().GetNbins())

    num2d2 = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (har2, pid, var))
    den2d2 = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (all, pid, var))
    num2 = num2d2.ProjectionX("num2", 1, num2d2.GetXaxis().GetNbins())
    den2 = den2d2.ProjectionX("den2", 1, den2d2.GetXaxis().GetNbins())

    ptbins = array.array(
        "d",
        [
            0.15,
            0.25,
            0.3,
            0.35,
            0.4,
            0.45,
            0.5,
            0.6,
            0.7,
            0.8,
            0.9,
            1.0,
            1.1,
            1.2,
            1.3,
            1.4,
            1.5,
            1.75,
            2.0,
            2.25,
            2.5,
            4.0,
        ],
    )

    num_new = num.Rebin(21, "num_new", ptbins)
    den_new = den.Rebin(21, "den_new", ptbins)

    num_new.Sumw2()
    num_new.Sumw2()
    num_new.Divide(den_new)
    num_new.Draw("e")
    num_new.SetLineWidth(2)
    num_new.SetLineColor(2)
    num_new.SetMarkerStyle(25)
    num_new.SetMarkerSize(1.5)
    num_new.SetMarkerColor(2)

    num1_new = num1.Rebin(21, "num1_new", ptbins)
    den1_new = den1.Rebin(21, "den1_new", ptbins)
    num1_new.Sumw2()
    den1_new.Sumw2()
    all = den1_new.Clone()
    den1_new.Add(num1_new, -1)
    den1_new.Divide(all)
    den1_new.Draw("e same")
    den1_new.SetLineWidth(2)
    den1_new.SetLineColor(1)
    den1_new.SetMarkerStyle(26)
    den1_new.SetMarkerSize(1.5)
    den1_new.SetMarkerColor(1)

    num2_new = num2.Rebin(21, "num2_new", ptbins)
    den2_new = den2.Rebin(21, "den2_new", ptbins)
    num2_new.Sumw2()
    den2_new.Sumw2()
    num2_new.Divide(den2_new)
    num2_new.Draw(" e same")
    num2_new.SetLineWidth(2)
    num2_new.SetLineColor(4)
    num2_new.SetMarkerStyle(27)
    num2_new.SetMarkerSize(1.5)
    num2_new.SetMarkerColor(4)

    num_new.SetTitle("")
    num_new.GetXaxis().SetTitle("p_{T}(GeV/#it{c})")
    num_new.GetYaxis().SetTitle(label)
    num_new.GetXaxis().SetRangeUser(0.0, 4)
    num_new.GetYaxis().SetRangeUser(0, 1)

    t = TLatex(8, 8, "ALICE3 O2 Performance")
    t.SetNDC()
    t.SetTextAlign(10)
    t.SetTextColor(1)
    t.SetTextSize(0.035)
    t.SetTextAlign(12)
    t.DrawLatex(0.15, 0.85, "ALICE3 O2 Performance study")
    t.SetTextSize(0.025)
    t.DrawLatex(0.15, 0.8, "PYTHIA 8 pp #sqrt{s} = 14TeV ")

    if lep == "Muon":
        t.DrawLatex(0.15, 0.75, "MID PID ")
    else:
        t.DrawLatex(0.15, 0.75, "TOF + RICH(3#sigma_{#pi}^{RICH} rej) PID ")

    t.DrawLatex(0.15, 0.7, "Layout v1, |#eta|< 1.1, B = 0.5T ")

    leg.AddEntry(den1_new, "Total")
    leg.AddEntry(num_new, "Pion")

    if lep == "Muon":
        leg.AddEntry(num2_new, "Kaon")
    else:
        leg.AddEntry(num2_new, "Muon")

    leg.Draw()
    canvas = "contamination of %s " % (lep)
    saveCanvas(cres, "%s" % (canvas))


# caculate the lepton over hadron with different PID case
def leptonvshadron(
    var="pt",
    lep="Electron",
    har1="Pion",
    all="All",
    har2="Kaon",
    pid1="NoSel",
    pid2="RICHSelHpElLoose",
    label="ratio lepton over hadron",
):
    fileo2 = TFile(path_file)
    cres = TCanvas("cres", "resolution distribution")
    cres.SetCanvasSize(3500, 1000)
    cres.Divide(3, 1)
    leg = TLegend(0.55, 0.7, 0.89, 0.89)
    leg.SetFillColor(0)
    leg.SetBorderSize(0)
    leg.SetNColumns(2)

    leg1 = TLegend(0.5, 0.75, 0.89, 0.89)
    leg1.SetFillColor(0)
    leg1.SetBorderSize(0)
    leg1.SetNColumns(2)

    leg2 = TLegend(0.5, 0.75, 0.89, 0.89)
    leg2.SetFillColor(0)
    leg2.SetBorderSize(0)
    leg2.SetNColumns(2)

    total2d = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (all, pid1, var))
    num2d = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (lep, pid1, var))
    num2d2 = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (lep, pid2, var))
    den2d = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (har1, pid1, var))
    den2d1 = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (har1, pid2, var))
    den2d2 = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (har2, pid1, var))
    den2d3 = fileo2.Get("qa-rejection-general/h%s%s/%seta" % (har2, pid2, var))

    total = total2d.ProjectionX("total", 1, total2d.GetXaxis().GetNbins())
    num = num2d.ProjectionX("num", 1, num2d.GetXaxis().GetNbins())
    num2 = num2d2.ProjectionX("num2", 1, num2d2.GetXaxis().GetNbins())
    den = den2d.ProjectionX("den", 1, den2d.GetXaxis().GetNbins())
    den1 = den2d1.ProjectionX("den1", 1, den2d1.GetXaxis().GetNbins())
    den2 = den2d2.ProjectionX("den2", 1, den2d2.GetXaxis().GetNbins())
    den3 = den2d3.ProjectionX("den3", 1, den2d3.GetXaxis().GetNbins())

    cres.cd(1)
    gPad.SetLogy()
    gPad.SetLogx()

    num.SetLineColor(5)
    num2.SetLineColor(2)
    den.SetLineColor(3)
    den1.SetLineColor(4)
    den2.SetLineColor(6)
    den3.SetLineColor(1)

    num.SetLineWidth(2)
    num2.SetLineWidth(2)
    den.SetLineWidth(2)
    den1.SetLineWidth(2)
    den2.SetLineWidth(2)
    den3.SetLineWidth(2)

    num.Scale(1.0 / total.Integral())
    num2.Scale(1.0 / total.Integral())
    den.Scale(1.0 / total.Integral())
    den1.Scale(1.0 / total.Integral())
    den2.Scale(1.0 / total.Integral())
    den3.Scale(1.0 / total.Integral())

    den.Draw("")
    num.Draw("same")
    num2.Draw("same")
    den2.Draw("same")
    den1.Draw("same")
    den3.Draw("same")

    den.SetTitle("")
    den.GetXaxis().SetRangeUser(0.0, 20.0)
    den.GetXaxis().SetTitleOffset(1.2)

    if pid2 == "RICHSelHpElLoose":
        leg.AddEntry(num, "e")
        leg.AddEntry(num2, "e (3#sigma_{e}^{RICH} PID)")
        leg.AddEntry(den, "#pi")
        leg.AddEntry(den1, "#pi  (3#sigma_{e}^{RICH} PID)")
        leg.AddEntry(den2, "K")
        leg.AddEntry(den3, "K (3#sigma_{e}^{RICH} PID)")

    if pid2 == "RICHSelHpElTightAlt":
        leg.AddEntry(num, "e")
        leg.AddEntry(num2, "e (TOF + RICH(3#sigma_{#pi}^{RICH} rej) PID)")
        leg.AddEntry(den, "#pi")
        leg.AddEntry(den1, "#pi (TOF + RICH(3#sigma_{#pi}^{RICH} rej) PID)")
        leg.AddEntry(den2, "K")
        leg.AddEntry(den3, "K (TOF + RICH(3#sigma_{#pi}^{RICH} rej) PID)")

    if pid2 == "MID":
        leg.AddEntry(num, "#mu")
        leg.AddEntry(num2, "#mu (MID PID)")
        leg.AddEntry(den, "#pi")
        leg.AddEntry(den1, "#pi (MID PID)")
        leg.AddEntry(den2, "K")
        leg.AddEntry(den3, "K (MID PID)")
        den.GetXaxis().SetRangeUser(0.7, 20.0)

    leg.Draw()

    ptbins_el = array.array(
        "d", [0.15, 0.25, 0.3, 0.5, 0.7, 1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 6.0, 8.0, 10.0]
    )

    num_new = num.Rebin(13, "num_new", ptbins_el)
    num1_new = num.Rebin(13, "num1_new", ptbins_el)
    num2_new = num2.Rebin(13, "num2_new", ptbins_el)
    num3_new = num2.Rebin(13, "num3_new", ptbins_el)
    den_new = den.Rebin(13, "den_new", ptbins_el)
    den1_new = den1.Rebin(13, "den1_new", ptbins_el)
    den2_new = den2.Rebin(13, "den2_new", ptbins_el)
    den3_new = den3.Rebin(13, "den3_new", ptbins_el)

    cres.cd(2)
    gPad.SetLogx()
    gPad.SetLogy()
    num_new.Sumw2()
    den_new.Sumw2()
    num_new.Divide(den_new)
    num_new.Draw("e")
    num_new.SetLineWidth(2)
    num_new.SetLineColor(3)
    num_new.SetMarkerStyle(25)
    num_new.SetMarkerSize(1.5)
    num_new.SetMarkerColor(3)

    num2_new.Sumw2()
    den1_new.Sumw2()
    num2_new.Divide(den1_new)
    num2_new.Draw("e same")
    num2_new.SetLineWidth(2)
    num2_new.SetLineColor(4)
    num2_new.SetMarkerStyle(26)
    num2_new.SetMarkerSize(1.5)
    num2_new.SetMarkerColor(4)

    num_new.GetYaxis().SetRangeUser(2e-4, 10e3)
    num_new.SetTitle("")
    num_new.GetXaxis().SetRangeUser(0.15, 10.0)
    num_new.GetXaxis().SetTitleOffset(1.2)

    if pid2 == "RICHSelHpElTightAlt":

        leg2.AddEntry(num_new, "e/#pi")
        leg2.AddEntry(num2_new, "e/#pi(TOF + RICH(3#sigma_{#pi}^{RICH} rej) PID)")

    if pid2 == "RICHSelHpElLoose":

        leg2.AddEntry(num_new, "e/#pi")
        leg2.AddEntry(num2_new, "e/#pi(3#sigma_{e}^{RICH} PID)")

    if pid2 == "MID":

        leg2.AddEntry(num_new, "#mu/#pi")
        leg2.AddEntry(num2_new, "#mu/#pi(MID PID)")
        num_new.GetXaxis().SetRangeUser(0.7, 20)

    leg2.Draw()

    cres.cd(3)
    gPad.SetLogx()
    gPad.SetLogy()
    num1_new.Sumw2()
    den2_new.Sumw2()
    num1_new.Divide(den2_new)
    num1_new.Draw(" e  ")
    num1_new.SetLineWidth(2)
    num1_new.SetLineColor(6)
    num1_new.SetMarkerStyle(25)
    num1_new.SetMarkerSize(1.5)
    num1_new.SetMarkerColor(6)

    num3_new.Sumw2()
    num3_new.Sumw2()
    den3_new.Divide(den3_new)
    num3_new.Draw(" e  same")
    num3_new.SetLineWidth(2)
    num3_new.SetLineColor(1)
    num3_new.SetMarkerStyle(26)
    num3_new.SetMarkerSize(1.5)
    num3_new.SetMarkerColor(1)

    num1_new.GetYaxis().SetRangeUser(2e-4, 10e3)
    num1_new.SetTitle("")
    num1_new.GetXaxis().SetRangeUser(0.15, 10.0)
    num1_new.GetXaxis().SetTitleOffset(1.2)

    t = TLatex(8, 8, "ALICE3 O2 Performance")
    t.SetNDC()
    t.SetTextAlign(10)
    t.SetTextColor(1)
    t.SetTextSize(0.025)
    t.SetTextAlign(12)
    t.DrawLatex(0.12, 0.85, "ALICE3 O2 Performance study")
    t.SetTextSize(0.02)
    t.DrawLatex(0.15, 0.8, "PYTHIA 8 pp #sqrt{s} = 14TeV ")
    t.DrawLatex(0.15, 0.7, "Layout v1, |#eta|< 1.1, B = 0.5T ")

    if pid2 == "RICHSelHpElTightAlt":
        leg1.AddEntry(num1_new, "e/K")
        leg1.AddEntry(num3_new, "e/K(TOF + RICH(4#sigma_{#pi}^{RICH} rej) PID)")

        t.DrawLatex(0.15, 0.75, "TOF + RICH(3#sigma_{#pi}^{RICH} rej) PID ")

    if pid2 == "RICHSelHpElLoose":
        leg1.AddEntry(num1_new, "e/K")
        leg1.AddEntry(num3_new, "e/K(3#sigma_{e}^{RICH} PID)")

        t.DrawLatex(0.15, 0.75, "3#sigma_{e}^{RICH} PID ")

    if pid2 == "MID":
        leg1.AddEntry(num1_new, "#mu/K")
        leg1.AddEntry(num3_new, "#mu/K(MID PID)")

        t.DrawLatex(0.15, 0.75, "MID PID ")
        num1_new.GetXaxis().SetRangeUser(0.7, 20)

    leg1.Draw()
    canvas = "distru_ratio_%s" % (pid2)
    saveCanvas(cres, "%s" % (canvas))  # num.SetTitle(nameresult)


# kinematic_plots("p", "pion", "MID", "Muon")
# kinematic_plots("p", "mu", "MID", "Muon")
# kinematic_plots("p", "pion", "TOF", "Electron")
# kinematic_plots("p", "pion", "RICH", "Electron")
# kinematic_plots("pt", "pion", "RICH", "Electron")
# kinematic_plots("p", "kaon", "RICH", "Electron")
# kinematic_plots("pt", "kaon", "RICH", "Electron")
# kinematic_plots("p", "pion", "TOF", "Kaon")
# kinematic_plots("p", "pion", "RICH", "Kaon")

ratioparticle(
    "pt", "Electron", "RICHSelHpElTight", "Electron", "NoSel", "e/e RICHSelHpElTight"
)
ratioparticle("pt", "Electron", "NoSel", "Pion", "NoSel", "e/#pi No cuts")
ratioparticle(
    "pt", "Electron", "RICHSelHpElTight", "Pion", "RICHSelHpElTight", "Tight e/#pi "
)
ratioparticle("pt", "Muon", "MID", "Pion", "MID", "MIDSel")
ratioparticle("pt", "Pion", "RICHSelHpElTight", "Pion", "NoSel", "Contamination")
ratioparticle("pt", "Pion", "MID", "Pion", "NoSel", "Contamination MID")

ratioparticle(
    "pt",
    "Electron",
    "RICHSelHpElTightAlt",
    "Electron",
    "RICHSelHpElTight",
    "e isElectronAndNotPion/RICHSelHpElTight",
)
ratioparticle(
    "pt",
    "Electron",
    "RICHSelHpElTightAlt",
    "Pion",
    "RICHSelHpElTightAlt",
    "isElectronAndNotPion e/#pi",
)


for p in ("Electron", "Pion", "Kaon", "Muon"):
    is_e_not_pi_plots(p)

contaminationvspar(
    "pt", "Electron", "Pion", "All", "Muon", "RICHSelHpElTightAlt", "Contamination"
)
contaminationvspar("pt", "Muon", "Pion", "All", "Kaon", "MID", "Contamination")

leptonvshadron(
    var="pt",
    lep="Electron",
    har1="Pion",
    all="All",
    har2="Kaon",
    pid1="NoSel",
    pid2="RICHSelHpElLoose",
    label="ratio lepton over hadron",
)

leptonvshadron(
    var="pt",
    lep="Electron",
    har1="Pion",
    all="All",
    har2="Kaon",
    pid1="NoSel",
    pid2="RICHSelHpElTightAlt",
    label="ratio lepton over hadron",
)

leptonvshadron(
    var="pt",
    lep="Muon",
    har1="Pion",
    all="All",
    har2="Kaon",
    pid1="NoSel",
    pid2="MID",
    label="ratio lepton over hadron",
)
