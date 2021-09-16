#pylint: disable=too-many-locals,too-many-statements, missing-docstring, pointless-string-statement
from array import array
import yaml
import pandas as pd #pylint: disable=import-error
# pylint: disable=import-error, no-name-in-module, unused-import
from ROOT import TH1F, TH2F, TCanvas, TGraph, TLatex, gPad, TFile, TF1
from ROOT import gStyle, gROOT, TStyle, TLegendEntry

"""
read predictions from arXiv.1907.12786.
The predictions are given for PbPb collisions at 2.76 TeV at mid-rapidity
for one unit of rapidity (therefore |y|<0.5).
"""
def read_predictions(hadron="Omega_ccc"):
    collision = "PbPb"

    # the option below do2pipnorm switches on/off the rinormalization of the
    #extracted spectrum,  which are given as 2*pi*pt dN/dpT.
    #For obtaining the dN/dpt one has to multiply for 2*pi*pt.
    do2pipnorm = 1
    energy = 2.76

    #miny,maxy are the yaxis min and max for plotting reasons only.
    if hadron == "Lambda_c":
        filename = "Lambda_c_ptdep_Stat_ChoLee_2_PbPb2p76_absy0p5"
        miny = 0.0001
        maxy = 10.
    if hadron == "Xi_cc":
        filename = "Xi_cc_ptdep_Stat_ChoLee_2_PbPb2p76_absy0p5"
        miny = 1.e-8
        maxy = 1.e-2
    if hadron == "Omega_cc":
        filename = "Omega_cc_ptdep_Stat_ChoLee_2_PbPb2p76_absy0p5"
        miny = 1.e-8
        maxy = 1.e-2
    if hadron == "Omega_ccc":
        filename = "Omega_ccc_ptdep_Stat_ChoLee_2_PbPb2p76_absy0p5"
        miny = 1.e-9
        maxy = 1.e-5
    if hadron == "X3872":
        filename = "X3872_ptdep_Stat_ChoLee_2_PbPb2p76_absy0p5"
        miny = 1.e-8
        maxy = 1.e-2

    with open(r'databases/general.yaml') as fileparam:
        param = yaml.load(fileparam, Loader=yaml.FullLoader)
    latexname = param["latexparticle"][hadron]

    #the binning of the ROOT histo in which the predictions will
    #be saved is hardcoded.
    nbins = 50
    minb = 0.
    maxb = 50.
    width = (maxb-minb)/float(nbins)

    legendtext = '%s %.2f TeV, arXiv.1907.12786 (Coal.2)' % (collision, energy)
    dfpred = pd.read_csv("../InputsTheory/" + filename+".csv")
    ptcand = dfpred["pt"]
    cross = dfpred["cross"]

    ptcand_val = array('f', ptcand)
    cross_val = array('f', cross)
    if do2pipnorm == 1:
        cross_val_ptscaled = array('f', [2*3.14*a*b for a, b in zip(ptcand_val, cross_val)])
    if do2pipnorm == 0:
        cross_val_ptscaled = cross_val

    #number of data points in each predictions
    npoints = 25

    grpred = TGraph(npoints, ptcand_val, cross_val_ptscaled)
    grpred.SetName("gdNdpt")
    grpred.SetLineColor(1)
    grpred.SetLineWidth(4)
    grpred.SetMarkerColor(1)
    grpred.SetMarkerStyle(21)
    grpred.SetTitle('')
    grpred.GetXaxis().SetTitle('p_{T} (GeV)')
    grpred.GetYaxis().SetTitle('dN/dp_{T} (%s) (GeV^{-1})' % latexname)

    histo = TH1F("hdNdpt", ";p_{T}; #Delta N/#Delta p_{T}, |y|<0.5", nbins, minb, maxb)
    norm = 0.
    for i in range(nbins-1):
        #to allow for the use of a arbitrary binning, we use the Eval function
        #of TGraph. We set to zero the prediction for xvalues for which data
        #are not given (8 GeV/c in this case). Given that most of the
        #cross-section is at low pT the bias is negligible
        yvalue = grpred.Eval(i*width+width/2.)
        if i*width+width/2. > 8.:
            yvalue = 0.
        histo.SetBinContent(i+1, yvalue)
        print(i+1, i*width+width/2., yvalue)
        norm = norm + width*yvalue
    fout = TFile("../InputsTheory/" + filename + ".root", "recreate")
    fout.cd()
    grpred.Write()
    histo.Write()
    histo_norm = TH1F("hdNdpt_norm", \
        ";p_{T}; #Delta N/#Delta p_{T}, |y|<0.5, norm = 1", nbins, minb, maxb)
    for i in range(nbins-1):
        histo_norm.SetBinContent(i+1, histo.GetBinContent(i+1)/norm)
    histo_norm.Write()

    cpred = TCanvas("cpred", "A Simple Graph Example", 881, 176, 668, 616)
    gStyle.SetOptStat(0)
    cpred.SetHighLightColor(2)
    cpred.Range(-1.25, -4.625, 11.25, 11.625)
    cpred.SetFillColor(0)
    cpred.SetBorderMode(0)
    cpred.SetBorderSize(2)
    cpred.SetLogy()
    cpred.SetFrameBorderMode(0)
    cpred.SetFrameBorderMode(0)
    cpred.cd()
    gPad.SetLogy()
    hempty = TH2F("hempty", ";p_{T};dN/d p_{T}", 100, 0., 10., 100, miny, maxy)
    hempty.GetYaxis().SetTitle('dN/dp_{T} (%s) (GeV^{-1})' % latexname)
    hempty.GetXaxis().SetTitle("p_{T}")
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
    grpred.Draw('Psame')
    histo.SetLineColor(1)
    histo.SetLineWidth(2)
    histo.Draw("same")
    tex = TLatex()
    tex.SetNDC()
    tex.SetTextFont(40)
    tex.SetTextColor(1)
    tex.SetTextSize(0.03)
    tex.SetTextAlign(12)
    tex.DrawLatex(0.12, 0.85, legendtext + ", dNdy=%.8f" % norm)
    #tex.Draw()
    cpred.SaveAs("plots/" + filename+".pdf")

read_predictions("Omega_ccc")
read_predictions("Omega_cc")
read_predictions("Xi_cc")
read_predictions("X3872")
read_predictions("Lambda_c")
