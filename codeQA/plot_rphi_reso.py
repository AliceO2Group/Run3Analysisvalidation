#!/usr/bin/env python3

import argparse
from ROOT import TFile, TH1, TLegend, TColor, TCanvas, TGraph, TObjArray

canvases = []


def make_reso(file_name, out_tag, show, thr=0.08):
    f = TFile(file_name, "READ")
    h = f.Get("qa-tracking-resolution/impactParameter/impactParameterRPhiVsPt")
    h.SetDirectory(0)
    f.Close()
    h.SetBit(TH1.kNoStats)
    if show:
        c = "2d" + out_tag
        c = TCanvas(c, c)
        canvases.append(c)
        c.SetLogz()
        h.Draw("COLZ")
    o = TObjArray()
    h.GetYaxis().SetRangeUser(-200, 200)
    h.FitSlicesY(0, 0, -1, 0, "QNR", o)
    h.GetYaxis().SetRange()
    hmean = o.At(1)
    hsigma = o.At(2)
    if show:
        hmean = hmean.DrawCopy("SAME")
        hsigma = hsigma.DrawCopy("SAME")
        hsigma.SetLineColor(2)
        c.Update()
    g = TGraph()
    g.SetName(out_tag)
    g.SetTitle(out_tag)
    g.GetXaxis().SetTitle(h.GetXaxis().GetTitle())
    g.GetYaxis().SetTitle(h.GetYaxis().GetTitle())

    for i in range(1, h.GetNbinsX() + 1):
        x = h.GetXaxis().GetBinCenter(i)
        if x < thr:
            continue
        hh = h.ProjectionY(f"{h.GetName()}_{i}", i, i)
        y = hh.GetRMS()
        y = hsigma.GetBinContent(hsigma.GetXaxis().FindBin(x))
        g.SetPoint(g.GetN(), x, y)
    if show:
        can2 = "1d" + out_tag
        can2 = TCanvas(can2, can2)
        canvases.append(can2)
        can2.SetLogy()
        can2.SetLogx()
        g.SetMarkerStyle(8)
        g.Draw()
        can2.Update()
    print(g.Eval(0.1))
    # input("press enter to continue")
    g.SaveAs(f"{out_tag}.root")
    return g


def main(input_files, tags, show=True):
    g = []
    for i, j in enumerate(input_files):
        t = tags[i]
        # print(i, j, t)
        g.append(make_reso(j, t, show))
    can = TCanvas("all", "all")
    can.DrawFrame(
        0,
        0,
        1,
        200,
        f";{g[0].GetXaxis().GetTitle()};Resolution {g[0].GetYaxis().GetTitle()};",
    )
    cols = ["#e41a1c", "#377eb8", "#4daf4a", "#984ea3"]
    leg = TLegend(0.9, 0.1, 1, 0.9)
    for j, i in enumerate(g):
        print("Drawing", i)
        c = TColor.GetColor(cols[j])
        i.SetLineColor(c)
        i.SetLineWidth(2)
        leg.AddEntry(i, "", "l")
        i.Draw("sameL")
    leg.Draw()
    can.Update()
    input("press enter to continue")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Runner for O2 analyses")
    parser.add_argument("files", type=str, nargs="+", help="Running files")
    parser.add_argument(
        "--tags", "-t", type=str, default=[], nargs="+", help="Tag for output files"
    )
    parser.add_argument("-s", action="store_true", help="Verbose mode")
    args = parser.parse_args()
    main(args.files, args.tags, args.s)
