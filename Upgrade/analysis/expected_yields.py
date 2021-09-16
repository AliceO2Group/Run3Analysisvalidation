#pylint: disable=too-many-locals,too-many-statements, missing-docstring, pointless-string-statement
from array import array
import yaml
# pylint: disable=import-error, no-name-in-module, unused-import
from ROOT import TH1F, TH2F, TCanvas, TGraph, TLatex, gPad, TFile, TF1
from ROOT import gStyle, gROOT, TStyle, TLegendEntry, TLegend

"""
Perform comparison between expected yields of the different baryons based
on dN/dy from different theoretical calculations and different assumptions
for the integrated luminosity of the various ion species.
for the moment we use the pT-shape normalized to unity from
arXiv.1907.12786 and we just multiply for dN/dy of each model.
"""

def analysis(hadron="Omega_ccc"):
    gStyle.SetOptStat(0)
    with open(r'databases/theory_yields.yaml') as fileparamyields:
        paramyields = yaml.load(fileparamyields, Loader=yaml.FullLoader)
    models = paramyields["comparison_models_AA"][hadron]["models"]
    collisions = paramyields["comparison_models_AA"][hadron]["collisions"]
    brmodes = paramyields["comparison_models_AA"][hadron]["brmode"]
    yranges = paramyields["comparison_models_AA"][hadron]["yrange"]
    colors = paramyields["comparison_models_AA"][hadron]["colors"]
    useshape = paramyields["comparison_models_AA"][hadron]["useshape"]
    ymin = paramyields["comparison_models_AA"][hadron]["ymin"]
    ymax = paramyields["comparison_models_AA"][hadron]["ymax"]
    binanal = array('d', paramyields["comparison_models_AA"]["pt_binning"][hadron])

    with open(r'databases/general.yaml') as fileparamgen:
        paramgen = yaml.load(fileparamgen, Loader=yaml.FullLoader)


    histolist = [None]*len(models)

    fin = TFile("../InputsTheory/" + useshape +".root")
    histo_norm = fin.Get("hdNdpt_norm") #this is the pT-shape normalized to unity


    for icase, _ in enumerate(models):
        histolist[icase] = histo_norm.Clone("dNdpt%s%s%s" % \
                (models[icase], collisions[icase], brmodes[icase]))
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

    hempty = TH2F("hempty", ";p_{T};Yields", 100, 0., 10., 100, ymin, ymax)
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

    leg = TLegend(0.1471471, 0.6108291, 0.3018018, 0.8747885, "", "brNDC")
    leg.SetBorderSize(1)
    leg.SetLineColor(0)
    leg.SetLineStyle(1)
    leg.SetLineWidth(1)
    leg.SetFillColor(0)
    leg.SetTextSize(0.022)
    leg.SetFillStyle(1001)

    # each case represents a combination of expected dNdy,
    # collision type and HP on the branching ratio.
    for icase, _ in enumerate(models):
        brmode = brmodes[icase]
        collision = collisions[icase]
        model = models[icase]
        yrange = yranges[icase]

        sigma_aa_b = paramgen["statistics"][collision]["sigmaAA_b"]
        lumiaa_monthi_invnb = paramgen["statistics"][collision]["lumiAA_monthi_invnb"]
        nevt = sigma_aa_b * lumiaa_monthi_invnb * 1e9 #luminosity given in nb-1
        bratio = paramgen["branchingratio"][hadron][brmode]
        yieldmid = paramyields[model][collision][yrange][hadron]
        text = '%s N_{ev}(%s) = %.1f B, BR=%.5f%%' \
                % (model, collision, nevt/1e9, bratio*100)
        scalef = bratio * nevt * yieldmid

        # before rebinning, the dNdpT is multiplied by the binwidth,
        # branching ratios, expected dN/dy and number of events
        # so that the yields can be simplied summed in rebinning process.
        for ibin in range(histolist[icase].GetNbinsX()):
            binwdith = histolist[icase].GetBinWidth(ibin+1)
            yvalue = histolist[icase].GetBinContent(ibin+1)
            histolist[icase].SetBinContent(ibin+1, binwdith*scalef*yvalue)
        histolist[icase] = histolist[icase].Rebin(len(binanal)-1, \
            "histo_pred%s%s%s" % (model, collision, brmode), binanal)
        histolist[icase].SetLineColor(colors[icase])
        histolist[icase].SetMarkerColor(colors[icase])
        histolist[icase].SetLineWidth(2)
        histolist[icase].Draw("same")
        text = text + " Yield(tot)=%.2f" % (histolist[icase].Integral())
        leg.AddEntry(histolist[icase], text, "pF")
    leg.Draw()
    canvas.SaveAs(hadron+"_results.pdf")
    canvas.SaveAs(hadron+"_results.C")
    foutput = TFile("foutput" + hadron + ".root", "recreate")
    for icase, _ in enumerate(models):
        foutput.cd()
        histolist[icase].Write()


analysis("Omega_ccc")
analysis("Omega_cc")
analysis("Xi_cc")
analysis("X3872")
