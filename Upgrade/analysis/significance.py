#pylint: disable=too-many-locals,too-many-statements, missing-docstring, pointless-string-statement
from math import sqrt
from array import array
import yaml
# pylint: disable=import-error, no-name-in-module, unused-import, too-many-arguments
from ROOT import TH1F, TH2F, TCanvas, TGraph, TLatex, gPad, TFile, TF1
from ROOT import gStyle, gROOT, TStyle, TLegendEntry, TLegend

"""
Macro to perform significance estimation of a given decay channel
in a given collision system. The ingredients are:
    - efficiency vs pt
    - estimated background yield within 3sigma below the peak / event
      bkgperevent is already multiplied by binwidth
    - expected signal yield / event = dN/dpT * binwidth * BR * efficiency
"""

def analysis(hadron="Lambda_c", collision="pp14p0", yrange="absy3p0", \
        brmode="central", model="Pyhia8mode2", use_unnorm=1):
    gStyle.SetOptStat(0)
    with open(r'databases/significance.yaml') as filesignificance:
        paramsignificance = yaml.load(filesignificance, Loader=yaml.FullLoader)
    ymin = paramsignificance[hadron][collision][yrange]["ymin"]
    ymax = paramsignificance[hadron][collision][yrange]["ymax"]
    #bin of the final analysis, has to be the binning of efficiency, bkg histos
    binanal = array('d', paramsignificance[hadron][collision][yrange]["binning"])
    nfileyieldth = paramsignificance[hadron][collision][yrange]["theoryfile"]

    nfileeff = paramsignificance[hadron][collision][yrange]["efffile"]
    nhistoeff = paramsignificance[hadron][collision][yrange]["histoeff"]
    nfilebkg = paramsignificance[hadron][collision][yrange]["bkgfile"]
    nhistobkg = paramsignificance[hadron][collision][yrange]["histobkg"]
    nhistoyieldth = paramsignificance[hadron][collision][yrange]["histoyield"]
    nhistoyieldth_norm = paramsignificance[hadron][collision][yrange]["histoyield_norm"]

    with open(r'databases/general.yaml') as fileparamgen:
        paramgen = yaml.load(fileparamgen, Loader=yaml.FullLoader)
    with open(r'databases/theory_yields.yaml') as fileyields:
        paramyields = yaml.load(fileyields, Loader=yaml.FullLoader)

    textcollision = paramgen["text_string"][collision]
    textrapid = paramgen["text_string"][yrange]
    textmodel = paramgen["text_string"][model]

    sigma_aa_b = paramgen["statistics"][collision]["sigmaAA_b"]
    lumiaa_monthi_invnb = paramgen["statistics"][collision]["lumiAA_monthi_invnb"]
    nevt = sigma_aa_b * lumiaa_monthi_invnb * 1e9
    #nevt = 2.*1e9
    bratio = paramgen["branchingratio"][hadron][brmode]
    decaychannel = paramgen["latexparticle"][hadron]

    yieldmid = paramyields[model][collision][yrange][hadron]
    text = '%s, N_{ev} = %.0f 10^{12}' % (textmodel, nevt/1e12)
    text_a = '%s, %s, BR=%.2f%%' % (decaychannel, textrapid, bratio*100)
    text_b = 'ALICE3 projection, with IRIS, no PID, %s' % textcollision
    fileeff = TFile(nfileeff)
    histoeff = fileeff.Get(nhistoeff)
    filebkg = TFile(nfilebkg)
    hbkgperevent = filebkg.Get(nhistobkg)

    fileyieldth = TFile(nfileyieldth)
    histoyieldth = None

    if use_unnorm == 1:
        histodndptth = fileyieldth.Get(nhistoyieldth)
        histodndptth.Scale(1./70000.) #TEMPORARY this is a fix to account for the
                                      #conversion from a cross-section in mub
                                      #to yields, sigma=70000 mub
    else:
        histodndptth = fileyieldth.Get(nhistoyieldth_norm)
        histodndptth.Scale(yieldmid)

    histoyieldth = histodndptth.Clone("histoyieldth")

    for ibin in range(histoyieldth.GetNbinsX()):
        binwdith = histoyieldth.GetBinWidth(ibin+1)
        yieldperevent = histoyieldth.GetBinContent(ibin+1)*binwdith*bratio
        histoyieldth.SetBinContent(ibin+1, yieldperevent)
        histoyieldth.SetBinError(ibin+1, 0.)
    histoyieldth = histoyieldth.Rebin(len(binanal)-1, \
             "histoyieldth", binanal)
    histosignfperevent = histoyieldth.Clone("histosignfperevent")
    histosignf = histoyieldth.Clone("histosignf")
    histosigoverbkg = histoyieldth.Clone("histosigoverbkg")

    canvas = TCanvas("canvas", "A Simple Graph Example", 881, 176, 668, 616)
    gStyle.SetOptStat(0)
    canvas.SetHighLightColor(2)
    canvas.Range(-1.25, -4.625, 11.25, 11.625)
    canvas.SetFillColor(0)
    canvas.SetBorderMode(0)
    canvas.SetBorderSize(2)
    canvas.SetLogy()
    canvas.SetFrameBorderMode(0)
    canvas.SetFrameBorderMode(0)
    canvas.cd()
    gPad.SetLogy()

    hempty = TH2F("hempty", ";p_{T} (GeV/c); Significance(3#sigma)", 100, 0., 10., 100, ymin, ymax)
    hempty.GetXaxis().SetTitle("p_{T} (GeV/c)")
    hempty.GetXaxis().SetLabelFont(42)
    hempty.GetXaxis().SetTitleOffset(1)
    hempty.GetXaxis().SetTitleFont(42)
    hempty.GetYaxis().SetLabelFont(42)
    hempty.GetYaxis().SetTitleOffset(1.35)
    hempty.GetYaxis().SetTitleFont(42)
    hempty.GetZaxis().SetLabelFont(42)
    hempty.GetZaxis().SetTitleOffset(1)
    hempty.GetZaxis().SetTitleFont(42)
    hempty.Draw()

    histosignf = histosignfperevent.Clone("histosignf")
    for ibin in range(histoyieldth.GetNbinsX()):
        yieldperevent = histoyieldth.GetBinContent(ibin+1)
        bkgperevent = hbkgperevent.GetBinContent(ibin+1)
        eff = histoeff.GetBinContent(ibin+1)
        signalperevent = eff*yieldperevent
        significanceperevent = 0
        if bkgperevent > 0:
            significanceperevent = signalperevent/sqrt(signalperevent+bkgperevent)
        signaloverbkg = 0
        if bkgperevent > 0:
            signaloverbkg = signalperevent/bkgperevent
        histosignfperevent.SetBinContent(ibin+1, significanceperevent)
        histosignfperevent.SetBinError(ibin+1, 0.)
        histosignf.SetBinContent(ibin+1, significanceperevent*sqrt(nevt))
        histosignf.SetBinError(ibin+1, 0.)
        histosigoverbkg.SetBinContent(ibin+1, signaloverbkg)
        histosigoverbkg.SetBinError(ibin+1, 0.)


    histosignfperevent.SetLineColor(1)
    histosignfperevent.SetMarkerColor(1)
    histosignfperevent.SetLineWidth(1)
    histosignf.SetLineColor(1)
    histosignf.SetMarkerColor(1)
    histosignf.SetLineWidth(2)
    histosignf.Draw("same")
    t_b = TLatex()
    t_b.SetNDC()
    t_b.SetTextFont(42)
    t_b.SetTextColor(1)
    t_b.SetTextSize(0.035)
    t_b.SetTextAlign(12)
    t_b.DrawLatex(0.2, 0.85, text_b)
    t_c = TLatex()
    t_c.SetNDC()
    t_c.SetTextFont(42)
    t_c.SetTextColor(1)
    t_c.SetTextSize(0.03)
    t_c.SetTextAlign(12)
    t_c.DrawLatex(0.2, 0.80, text)
    t_a = TLatex()
    t_a.SetNDC()
    t_a.SetTextFont(42)
    t_a.SetTextColor(1)
    t_a.SetTextSize(0.035)
    t_a.SetTextAlign(12)
    t_a.DrawLatex(0.2, 0.75, text_a)
    canvas.SaveAs(hadron+"_results.pdf")
    canvas.SaveAs(hadron+"_results.C")

    foutput = TFile("foutput" + hadron + ".root", "recreate")
    foutput.cd()
    histoeff.Write()
    hbkgperevent.Write()
    histosignfperevent.Write()
    histoyieldth.Write()
    histosignf.Write()
    histodndptth.Write()
    histosigoverbkg.Write()
#analysis("Lambda_c", "pp14p0", "absy1p44", "central", "Pyhia8mode2", 1)
analysis("Jpsitoee", "pp14p0", "absy1p44", "central", "Pyhia8monash", 1)
analysis("X3872", "pp14p0", "absy1p44", "central", "Pyhia8monash", 1)
