#!/usr/bin/python3

from ROOT import TFile, gROOT, gStyle, TLegend, TCanvas, gPad, gSystem
from sys import argv


def compare(filerun3, filerun1):
    f = [TFile(filerun1, "READ"), TFile(filerun3, "READ")]

    def get(hn, d1, d3=None, V=True):
        if "/" not in d1:
            d1 = d1 + "/"
        if not d3:
            d3 = d1
        if "/" not in d3:
            d3 = d3 + "/"
        if V:
            print("Getting", hn, "from", f[0].GetName(), "and", f[1].GetName())
        h = [f[0].Get(d1 + hn), f[1].Get(d3 + hn)]
        for i, j in enumerate(h):
            if not j:
                f[i].ls()
            j.SetDirectory(0)
            j.SetTitle(j.GetName())
        return h

    # gROOT.SetStyle("Plain")
    gStyle.SetOptStat(0)
    # gStyle.SetOptStat(0000)
    # gStyle.SetPalette(0)
    # gStyle.SetCanvasColor(0)
    # gStyle.SetFrameFillColor(0)
    # # gStyle.SetOptTitle(0)

    hptel = get("hptel", "filterEl-task/")
    hp = get("hp", "filterEl-task/", "p-task/")
    hbeta = get("hbeta", "filterEl-task/")
    hbetael = get("hbetael", "filterEl-task/")
    hbetaelsigma = get("hbetaelsigma", "filterEl-task/")

    legend = TLegend(0.5, 0.7, 0.8, 0.9)
    legend.AddEntry(hptel[0], "Run1", "f")
    legend.AddEntry(hptel[1], "Run3", "f")
    # legend.SetHeader("Legend","C"); // option "C" allows to center the header
    cv = TCanvas("cv", "Canvas", 1600, 1600)
    cv.Divide(2)

    def draw(h, opt="", copy=False):
        hd = h
        if copy:
            hd = h.DrawCopy(opt)
            hd.SetDirectory(0)
        else:
            h.Draw(opt)
        gPad.Update()
        return hd

    def drawdiff(h, opt=""):
        hd = draw(h[0], opt=opt, copy=True)
        hd.Add(h[1], -1)
        gPad.Modified()
        gPad.Update()

    def drawtwo(h, pos, logx=False, logy=False, V=True):
        cv.cd(pos)
        if logy:
            gPad.SetLogy()
        if logx:
            gPad.SetLogx()
        # h[0].GetXaxis().SetTitle("#it{p}_{T} (GeV)")
        # if "TH2" in h[0].ClassName():
        #     h[0] = h[0].ProjectionX()
        #     h[1] = h[1].ProjectionX()
        h[0].SetLineColor(2)
        h[0].SetLineWidth(2)
        if V:
            print("Drawing", h)
        draw(h[0])
        draw(h[1], "same")
        print("Entries of", h[0].GetName(), h[0].GetEntries(), "vs", h[1].GetEntries())
        legend.Draw()

    drawtwo(hptel, 1)
    drawtwo(hp, 2)
    # drawtwo(hbetael, 3)
    # drawtwo(hbetaelsigma, 4)
    #
    canbeta = TCanvas("hbeta", "hbeta", 1600, 1600)
    canbeta.Divide(3)
    canbeta.cd(1)
    draw(hbeta[0], "COLZ")
    canbeta.cd(2)
    draw(hbeta[1], "COLZ")
    # drawtwo(hbetaelsigma, 4)
    #
    canbetael = TCanvas("hbetael", "hbetael", 1600, 1600)
    canbetael.Divide(3)
    canbetael.cd(1)
    draw(hbetael[0], "COLZ")
    canbetael.cd(2)
    draw(hbetael[1], "COLZ")
    canbetael.cd(3)
    drawdiff(hbetael, "COLZ")
    #
    canbetaelsigma = TCanvas("hbetaelsigma", "hbetaelsigma", 1600, 1600)
    canbetaelsigma.Divide(3)
    canbetaelsigma.cd(1)
    draw(hbetaelsigma[0], "COLZ")
    canbetaelsigma.cd(2)
    draw(hbetaelsigma[1], "COLZ")
    canbetaelsigma.cd(3)
    drawdiff(hbetaelsigma, "COLZ")

    cv.SaveAs("cv.pdf")
    gSystem.ProcessEvents()
    input()


compare(argv[1], argv[2])
