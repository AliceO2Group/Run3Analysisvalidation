#!/usr/bin/python3

from ROOT import (
    TFile,
    gROOT,
    gStyle,
    TLegend,
    TCanvas,
    gPad,
    gSystem,
    TLatex,
    TPaveText,
)
from sys import argv


def get(f, hn, d1, d3=None, V=True):
    if "/" not in d1:
        d1 = d1 + "/"
    if not d3:
        d3 = d1
    if "/" not in d3:
        d3 = d3 + "/"
    if V:
        print("Getting", hn, "from", f[0].GetName(), "and", f[1].GetName())
    h = [f[0].Get(d1 + hn), f[1].Get(d3 + hn)]
    # Set style of histos
    h[0].SetTitle("Run2")
    h[0].SetLineColor(2)
    h[0].SetLineStyle(7)
    h[0].SetLineWidth(2)
    h[1].SetTitle("Run3")
    for i, j in enumerate(h):
        if not j:
            f[i].ls()
        j.SetDirectory(0)
    return h


ldrawn = []


def drawrange(x, y, xtit="", ytit=""):
    if type(x) != type([]):
        if xtit == "":
            xtit = x.GetXaxis().GetTitle()
        x = [x.GetXaxis().GetBinLowEdge(1), x.GetXaxis().GetBinUpEdge(x.GetNbinsX())]
    ldrawn.append(gPad.DrawFrame(x[0], y[0], x[1], y[1], ";" + xtit + ";" + ytit))
    ldrawn[-1].GetYaxis().CenterTitle()


latexdrawn = []


def draw(h, opt="", copy=False, labelize=True):
    hd = h
    if copy:
        print("Drawing", h, "as a copy")
        hd = h.DrawCopy(opt)
        if type(copy) == type(""):
            hd.SetName(hd.GetName() + copy)
        else:
            hd.SetName(hd.GetName() + "copy")
        hd.SetDirectory(0)
    else:
        h.Draw(opt)
    if labelize and "same" not in opt:
        latex = TLatex(0.2, 0.95, hd.GetName())
        latex.SetNDC()
        latex.Draw()
        latexdrawn.append(latex)
    gPad.Update()
    ldrawn.append(hd)
    return hd


def drawcounts(h):
    txt = TPaveText(0.75, 0.55, 0.88, 0.7, "NDC")
    for i in h:
        txt.AddText("{}={}".format(i.GetTitle(), i.GetEntries()))
    txt.Draw()
    latexdrawn.append(txt)
    gPad.Update()


def drawdiff(h, opt="", useabs=True):
    hd = draw(h[0], opt=opt, copy=" Difference")
    if useabs:
        if "TH2" in h[0].ClassName():
            for i in range(0, h[0].GetNbinsX()):
                for j in range(0, h[0].GetNbinsY()):
                    hd.SetBinContent(
                        i + 1,
                        j + 1,
                        abs(
                            h[0].GetBinContent(i + 1, j + 1)
                            - h[1].GetBinContent(i + 1, j + 1)
                        ),
                    )
    else:
        hd.Add(h[1], -1)
    gPad.Modified()
    gPad.Update()


def nextpad():
    pn = gPad.GetName()
    cn = gPad.GetCanvas().GetName()
    pad = pn.replace(cn, "").replace("_", "")
    if pad == "":
        gPad.cd(1)
    else:
        gPad.GetCanvas().cd(int(pad) + 1)


legends = []


def makelegend(h):
    l = TLegend(0.7, 0.7, 0.9, 0.9)
    for i in h:
        l.AddEntry(i, "", "f")
    # l.SetHeader("Legend","C"); // option "C" allows to center the header
    legends.append(l)


def drawtwo(h, logx=False, logy=False, project=True, ratio=True, V=True):
    def equals(h1, h2, Ck):
        cmd = ["h1." + Ck, "h2." + Ck]
        cmdR = [eval(cmd[0]), eval(cmd[1])]
        V = cmdR[0] == cmdR[1]
        if not V:
            raise ValueError(h1, "differs from", h2, Ck, cmd, cmdR)

    for i in "GetName GetNbinsX GetXaxis().GetTitle".split():
        equals(h[0], h[1], i + "()")
    nextpad()
    if logy:
        gPad.SetLogy()
    if logx:
        gPad.SetLogx()
    hrun2 = h[0]
    hrun3 = h[1]
    if project and "TH2" in h[0].ClassName():
        hrun2 = h[0].ProjectionY("Y1_" + h[0].GetName())
        hrun3 = h[1].ProjectionY("Y2_" + h[1].GetName())
    if V:
        print("Drawing", h)
    if "TH2" in hrun2.ClassName() and not project:
        draw(hrun2, "COLZ")
        nextpad()
        if logy:
            gPad.SetLogy()
        if logx:
            gPad.SetLogx()
        draw(hrun3, "COLZ")
    else:
        draw(hrun2)
        draw(hrun3, "same")
        drawcounts([hrun2, hrun3])
    print(
        "Entries of", hrun2.GetName(), hrun2.GetEntries(), "vs", hrun3.GetEntries(),
    )
    if "TH1" in h[0].ClassName():
        legends[-1].Draw()
    if ratio:
        nextpad()
        drawrange(hrun2, [0.5, 1.5], ytit="Run2/Run3")
        hratio = draw(hrun2, "same", copy=True, labelize=False)
        hratio.SetTitle(hratio.GetTitle() + " Ratio")
        hratio.Divide(hrun3)
        gPad.Update()


canvaslist = []


def makecanvas(cname, ctit, sizex=1600, sizey=1600):
    canvaslist.append(TCanvas(cname, ctit, sizex, sizey))
    return canvaslist[-1]


def compare(filerun3, filerun1):
    f = [TFile(filerun1, "READ"), TFile(filerun3, "READ")]
    # gROOT.SetStyle("Plain")
    gStyle.SetOptStat(0)
    gStyle.SetOptTitle(0)
    # gStyle.SetOptStat(0000)
    # gStyle.SetPalette(0)
    # gStyle.SetCanvasColor(0)
    # gStyle.SetFrameFillColor(0)

    hp_NoCut = get(f, "hp_NoCut", "filterEl-task/", "p-task/")
    hp_TrkCut = get(f, "hp_TrkCut", "filterEl-task/", "p-task/")
    hp_TOFCut = get(f, "hp_TOFCut", "filterEl-task/", "p-task/")
    #
    hlength_NoCut = get(f, "hlength_NoCut", "filterEl-task/", "tofpidqa-task/")
    htime_NoCut = get(f, "htime_NoCut", "filterEl-task/", "tofpidqa-task/")
    hevtime_NoCut = get(f, "hevtime_NoCut", "filterEl-task/", "tofpidqa-task/")
    #
    hp_El = get(f, "hp_El", "filterEl-task/")
    hpt_El = get(f, "hpt_El", "filterEl-task/")
    hlength_El = get(f, "hlength_El", "filterEl-task/")
    htime_El = get(f, "htime_El", "filterEl-task/")
    hp_beta = get(f, "hp_beta", "filterEl-task/")
    hp_beta_El = get(f, "hp_beta_El", "filterEl-task/")
    hp_betasigma_El = get(f, "hp_betasigma_El", "filterEl-task/")

    makelegend(hp_El)

    if True:
        makecanvas("cmom", "Momentum").Divide(2, 3)
        drawtwo(hp_NoCut)
        drawtwo(hp_TrkCut)
        drawtwo(hp_TOFCut)
    #
    if True:
        makecanvas("ctof", "TOFInfo").Divide(2, 3)
        drawtwo(hlength_NoCut, logy=True)
        drawtwo(htime_NoCut, logy=True)
        drawtwo(hevtime_NoCut)
    if True:
        makecanvas("ctofEl", "TOFInfoEl").Divide(2, 3)
        drawtwo(hlength_El)
        drawtwo(htime_El)
        # drawtwo(hevtime_El)
    #
    if True:
        makecanvas("cbetaslice", "BetaSlice").Divide(2, 3)
        drawtwo(hp_beta)
        drawtwo(hp_beta_El)
        drawtwo(hp_betasigma_El)
    #
    if True:
        makecanvas("cmom_El", "Momentum_El").Divide(2, 2)
        drawtwo(hp_El)
        drawtwo(hpt_El)

    #
    if True:
        makecanvas("hp_beta", "Beta").Divide(3, 2)
        drawtwo(hp_beta, project=False, ratio=False)
        nextpad()
        drawdiff(hp_beta, "COLZ")
        drawtwo(hp_beta_El, project=False, ratio=False)
        nextpad()
        drawdiff(hp_beta_El, "COLZ")
    #
    if True:
        makecanvas("hp_betasigma_El", "BetaElSigma").Divide(3)
        nextpad()
        draw(hp_betasigma_El[0], "COLZ")
        nextpad()
        draw(hp_betasigma_El[1], "COLZ")
        nextpad()
        drawdiff(hp_betasigma_El, "COLZ")

    gSystem.ProcessEvents()
    canvaslist[0].SaveAs("plots.pdf[")
    for i in canvaslist:
        i.SaveAs("plots.pdf")
    canvaslist[-1].SaveAs("plots.pdf]")
    input()


compare(argv[1], argv[2])
