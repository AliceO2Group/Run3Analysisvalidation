#!/usr/bin/env python3

from ROOT import TFile, TCanvas, TLegend, gPad, TColor, TH1, TLatex
import argparse
import itertools


def compare(objs, add_leg_title=True):
    print("Comparing")
    cols = ['#e41a1c', '#377eb8', '#4daf4a']
    colors = {}
    drawn = {}
    for i in objs:
        print("Entry", len(colors), i)
        colors[i] = TColor.GetColor(cols[len(colors)])
    # Drawing objects
    for i in objs:
        for j in objs[i]:
            obj = objs[i][j]
            opt = ""
            if drawn.setdefault(j, None) is None:
                drawn[j] = [TCanvas(j, j)]
            else:
                opt += "same"
                drawn[j][0].cd()
            print("Drawing", obj, "with opt", opt, "on canvas", gPad.GetName())
            obj.SetLineColor(colors[i])
            obj.SetBit(TH1.kNoTitle)
            obj.SetBit(TH1.kNoStats)
            obj.SetTitle(i)
            drawn[j].append(obj.DrawNormalized(opt))
    for i in drawn:
        d = drawn[i]
        can = d[0]
        can.cd()
        gPad.SetLogy()
        leg = TLegend(.8, .8, .99, .99, can.GetName())
        d.append(leg)
        for j in can.GetListOfPrimitives():
            leg.AddEntry(j)
        leg.Draw()
    return drawn


def main(files, th1=True, th2=False, th3=False):
    f = [TFile(i) for i in files]
    h = {}

    def extract(directory):
            def accept_obj(entry):
                if not th1 and "TH1" in entry.ClassName():
                    return False
                if not th2 and "TH2" in entry.ClassName():
                    return False
                if not th3 and "TH3" in entry.ClassName():
                    return False
                return True
            o = []
            print("Dir", directory)
            for i in directory.GetListOfKeys():
                obj = directory.Get(i.GetName())
                if not accept_obj(obj):
                    continue
                if "TDirectory" in obj.ClassName():
                    for j in obj.GetListOfKeys():
                        if not accept_obj(obj.Get(j.GetName())):
                            continue
                        o.append(f"{directory.GetName()}/{i.GetName()}/{j.GetName()}")
                    continue
                o.append(f"{directory.GetName()}/{i.GetName()}")
            return o

    for i in f:
        fn = i.GetName()
        fn = fn.replace(".root", "")
        fn = fn.replace("AnalysisResults_O2_Run5_", "")
        h[fn] = {}
        l = i.GetListOfKeys()
        for j in l:
            # h[fn] = list(itertools.chain(*extract(i.Get(j.GetName()))))
            o = extract(i.Get(j.GetName()))
            for k in o:
                h[fn][k] = i.Get(k) 
    drawn = compare(h)
    first = True
    for i in drawn:
        obj = drawn[i][0]
        print(i)
        if first:
            first_obj = obj
            obj.SaveAs("Comparison.pdf[")
        obj.SaveAs("Comparison.pdf")
        first = False
    first_obj.SaveAs("Comparison.pdf]")
    fout = TFile("Comparison.root", "RECREATE")
    for i in drawn:
        obj = drawn[i][0]
        obj.Write()
    fout.Close()


if __name__ == "__main__":
    pass
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("files", type=str, nargs="+",
                        help="Input files")
    parser.add_argument("-v", action="store_true", help="Verbose mode")
    parser.add_argument("-b", action="store_true", help="Background mode")
    args = parser.parse_args()

    main(files=args.files)
