#!/usr/bin/env python3

"""
Comparison script.
Comparing different files with same structure and same histogram names.
To run your comparison between AnalysisResults1.root AnalysisResults2.root you can use:
./compare.py AnalysisResults1.root AnalysisResults2.root -b
"""

import argparse

from ROOT import (  # pylint: disable=import-error
    TH1,
    TCanvas,
    TColor,
    TFile,
    TLegend,
    gPad,
    gROOT,
)

# import itertools


def compare(dict_obj, add_leg_title=True, normalize=True):
    print("Comparing")
    list_colors = ["#e41a1c", "#377eb8", "#4daf4a"]
    list_markers = [21, 20, 34]
    dict_colors = {}
    dict_markers = {}
    dict_list_canvas = {}
    for key_file in dict_obj:
        print("Entry", len(dict_colors), key_file)
        dict_colors[key_file] = TColor.GetColor(list_colors[len(dict_colors)])
        dict_markers[key_file] = list_markers[len(dict_markers)]
    # Drawing objects
    is_first_file = True
    key_file_first = ""
    for key_file in dict_obj:
        if is_first_file:
            key_file_first = key_file
        for key_obj in dict_obj[key_file]:
            obj = dict_obj[key_file][key_obj]
            # FIXME
            if "TDirectory" in obj.ClassName():
                continue
            opt = "LP"
            if dict_list_canvas.setdefault(key_obj, None) is None:
                dict_list_canvas[key_obj] = [TCanvas(key_obj, key_obj), TCanvas(f"{key_obj}_ratio", f"{key_obj}_ratio")]
            else:
                opt += "same"
            dict_list_canvas[key_obj][0].cd()
            print(f'Drawing {obj.GetName()} with opt "{opt}" on canvas {gPad.GetName()}')
            obj.SetLineColor(dict_colors[key_file])
            obj.SetMarkerStyle(dict_markers[key_file])
            obj.SetMarkerColor(dict_colors[key_file])
            obj.SetBit(TH1.kNoTitle)
            obj.SetBit(TH1.kNoStats)
            obj.SetTitle(key_file)
            if normalize:
                dict_list_canvas[key_obj].append(obj.DrawNormalized(opt))
            else:
                dict_list_canvas[key_obj].append(obj.DrawClone(opt))
            # Ratio
            if not is_first_file:
                dict_list_canvas[key_obj][1].cd()
                print(f'Drawing {obj.GetName()} with opt "{opt}" on canvas {gPad.GetName()}')
                # line_1 = TLine(obj.GetXaxis().GetXmin(), 1, obj.GetXaxis().GetXmax(), 1)
                obj_ratio = obj.Clone(f"{obj.GetName()}_ratio")
                obj_ratio.Divide(dict_obj[key_file_first][key_obj])
                dict_list_canvas[key_obj].append(obj_ratio.DrawClone(opt))
                # dict_list_canvas[key_obj].append(line_1.Draw())
        is_first_file = False
    for key_obj in dict_list_canvas:
        list_canvas = dict_list_canvas[key_obj]
        can = list_canvas[0]
        can.cd()
        # gPad.SetLogy()
        leg = TLegend(0.1, 0.9, 0.9, 0.99, can.GetName())
        leg.SetNColumns(2)
        list_canvas.append(leg)
        for prim in can.GetListOfPrimitives():
            leg.AddEntry(prim)
        leg.Draw()
        # Ratio
        can_ratio = list_canvas[1]
        can_ratio.cd()
        # gPad.SetLogy()
        leg_ratio = TLegend(0.1, 0.9, 0.9, 0.99, can_ratio.GetName())
        leg_ratio.SetNColumns(2)
        list_canvas.append(leg_ratio)
        for prim in can_ratio.GetListOfPrimitives():
            leg_ratio.AddEntry(prim)
        leg_ratio.Draw()
    return dict_list_canvas


def main(files, th1=True, th2=False, th3=False):
    gROOT.SetBatch(True)
    list_files = [TFile(i) for i in files]
    dict_obj = {}

    def extract(directory):
        def accept_obj(entry):
            if not th1 and "TH1" in entry.ClassName():
                return False
            if not th2 and "TH2" in entry.ClassName():
                return False
            if not th3 and "TH3" in entry.ClassName():
                return False
            return True

        list_names = []
        print(f"Directory {directory.GetName()}")
        for key in directory.GetListOfKeys():
            obj = directory.Get(key.GetName())
            if not accept_obj(obj):
                continue
            if "TDirectory" in obj.ClassName():
                for key_sub in obj.GetListOfKeys():
                    if not accept_obj(obj.Get(key_sub.GetName())):
                        continue
                    list_names.append(f"{directory.GetName()}/{key.GetName()}/{key_sub.GetName()}")
                continue
            list_names.append(f"{directory.GetName()}/{key.GetName()}")
        return list_names

    for file in list_files:
        name_file = file.GetName()
        name_file = name_file.replace(".root", "")
        name_file = name_file.replace("AnalysisResults_O2_Run5_", "")
        name_file = name_file.split("/")[-1]
        dict_obj[name_file] = {}
        list_keys = file.GetListOfKeys()
        for key in list_keys:
            # h[fn] = list(itertools.chain(*extract(i.Get(j.GetName()))))
            list_obj_names = extract(file.Get(key.GetName()))
            for name_obj in list_obj_names:
                dict_obj[name_file][name_obj] = file.Get(name_obj)
    dict_list_canvas = compare(dict_obj, normalize=False)
    first = True
    for key_obj in dict_list_canvas:
        can = dict_list_canvas[key_obj][0]
        can_rat = dict_list_canvas[key_obj][1]
        print(key_obj)
        if first:
            can_first = can
            can_first.SaveAs("Comparison.pdf[")
            first = False
        can.SaveAs("Comparison.pdf")
        can_rat.SaveAs("Comparison.pdf")
    can_first.SaveAs("Comparison.pdf]")
    # file_out = TFile("Comparison.root", "RECREATE")
    # for key_obj in dict_list_canvas:
    #     can = dict_list_canvas[key_obj][0]
    #     print("Writing", can.GetName())
    #     can.Write(can.GetName().replace("/", "_folder_"))
    # file_out.Close()


if __name__ == "__main__":
    pass
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("files", type=str, nargs="+", help="Input files")
    parser.add_argument("-v", action="store_true", help="Verbose mode")
    parser.add_argument("-b", action="store_true", help="Background mode")
    args = parser.parse_args()

    main(files=args.files)
