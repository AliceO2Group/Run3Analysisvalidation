#!/usr/bin/env python3

"""
Script to plot together canvases in different files
"""

from sys import argv

from ROOT import TColor, TFile  # , TLegend


def main(filename, canvases):
    f = TFile(filename, "READ")
    f.ls()
    cols = ["#e41a1c", "#377eb8", "#4daf4a"]
    cols = [TColor.GetColor(i) for i in cols]
    print("Canvases", canvases)
    canname = canvases[0]
    canname.split("_folder_")[-1]
    can = f.Get(canname)
    lcan = can.GetListOfPrimitives()
    lcan.FindObject("TPave").SetHeader("")
    can.Draw()
    print(can)
    # leg = TLegend(0.7, 0.7, 0.9, 0.9)
    for i in canvases[1:]:
        print("Getting", i)
        c = f.Get(i)
        lp = c.GetListOfPrimitives()
        # lp.ls()
        for j in lp:
            if "TH1" not in j.ClassName():
                continue
            print(j)
            can.cd()
            j.Draw("sameHIST")
            can.Update()
    return can


res = main(argv[1], argv[2:])

input("Press enter to continue")
