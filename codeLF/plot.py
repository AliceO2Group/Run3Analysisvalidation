#!/usr/bin/python3

from sys import argv

from ROOT import TCanvas, TFile, gPad


def get(fn, dn, hn, V=False):
    f = TFile(fn, "READ")
    if V:
        f.ls()
        print("Getting", dn)
    d = f.Get(dn)
    if V:
        d.ls()
        print("Getting", hn)
    h = d.Get("h" + hn)
    h.SetDirectory(0)
    f.Close()
    return h


# def main(hl = "TOF TPC TRD"):
# def main(hl="TOF TOFBeta TOFLength"):
# def main(fn, dn, hl="ptel beta betael betaelsigma"):
def main(fn, dn, hl="ptel"):
    h = [get(fn=fn, dn=dn, hn=i) for i in hl.split()]
    cl = []
    for i in h:
        cl.append(TCanvas("c" + i.GetName(), i.GetName()))
        gPad.SetLeftMargin(0.15)
        i.Draw()
        gPad.Update()
    print("Enter to continue")
    input()


main(argv[1], argv[2])
