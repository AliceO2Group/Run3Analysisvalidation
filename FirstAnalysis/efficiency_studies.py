#!/ usr / bin / env python
from ROOT import (  # pylint: disable=import-error, no-name-in-module, unused-import
    TH1F,
    TCanvas,
    TEfficiency,
    TFile,
    TLegend,
    gPad,
)


def efficiencytracking():
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
    fileo2 = TFile("../codeHF/AnalysisResults_O2.root")

    c1 = TCanvas("c1", "Efficiency")
    c1.SetCanvasSize(1500, 1500)
    c1.cd()
    gPad.SetLogx()
    # gPad.SetLogy()
    c2 = TCanvas("c2", "Efficiency vs eta")
    c2.SetCanvasSize(1500, 1500)
    c2.cd()
    c3 = TCanvas("c3", "Efficiency vs phi")
    c3.SetCanvasSize(1500, 1500)
    c3.cd()
    eff_list = []
    hempty = TH1F("hempty", ";p_{T};efficiency", 100, 0.001, 15.0)
    hempty.Draw()
    leg = TLegend(0.1, 0.7, 0.3, 0.9, "")
    leg.SetFillColor(0)
    leg_eta = TLegend(0.1, 0.7, 0.3, 0.9, "")
    leg_eta.SetFillColor(0)
    leg_phi = TLegend(0.1, 0.7, 0.3, 0.9, "")
    leg_phi.SetFillColor(0)

    for i, had in enumerate(hadron_list):
        # hnum = fileo2.Get("qa-tracking-efficiency-%s/num" % had)
        # hden = fileo2.Get("qa-tracking-efficiency-%s/den" % had)
        # hnum.Rebin(4)
        # hden.Rebin(4)
        leff = fileo2.Get("qa-tracking-efficiency-%s/Efficiency" % had)
        eff = leff.At(0)
        eff_eta = leff.At(1)
        eff_phi = leff.At(2)
        eff.SetLineColor(color_list[i])
        # hden.SetLineColor(color_list[i])
        eff_list.append(eff)
        eff.Draw("same")
        leg.AddEntry(eff_list[i], had)
        eff_eta.SetLineColor(color_list[i])
        eff_list.append(eff_eta)
        eff_eta.Draw("same")
        leg_eta.AddEntry(eff_list[i], had)
        eff_phi.SetLineColor(color_list[i])
        eff_list.append(eff_phi)
        eff_phi.Draw("same")
        leg_phi.AddEntry(eff_list[i], had)
    # hden.GetYaxis().SetRange(0, 1000)
    # hden.Draw("same")
    leg.Draw()
    leg_eta.Draw()
    leg_phi.Draw()
    c1.SaveAs("efficiency_tracking.pdf")
    c1.SaveAs("efficiency_tracking.root")
    c2.SaveAs("efficiency_tracking_eta.pdf")
    c2.SaveAs("efficiency_tracking_eta.root")
    c3.SaveAs("efficiency_tracking_phi.pdf")
    c3.SaveAs("efficiency_tracking_phi.root")


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
    ceffhf.SaveAs("efficiency_hfcand%s%s.pdf" % (had, var))
    ceffhf.SaveAs("efficiency_hfcand%s%s.root" % (had, var))

    efficiencytracking()
    efficiencyhadron("lc", "Pt")
