#!/usr/bin/env python3

"""
Comparison script.
Comparing different files with same structure and same histogram names.
To run your comparison between AnalysisResults1.root AnalysisResults2.root you can use:
./compare.py AnalysisResults1.root AnalysisResults2.root -b
"""

import argparse
from sys import exit

from ROOT import (  # pylint: disable=import-error
    TH1,
    TH2,
    TH3,
    THnSparse,
    # RooUnfoldResponse,
    TAxis,
    TCanvas,
    TColor,
    TFile,
    TLegend,
    gPad,
    gROOT,
)

# import itertools

def msg_err(message : str):
    """ Print error message """
    print(f"Error: {message}")


def msg_fatal(message : str):
    """ Print error message and exit """
    print(f"Fatal: {message}")
    exit(1)


def are_valid(*objects) -> bool:
    """Check whether objects exist"""
    result = True
    for i, o in enumerate(objects):
        if not o:
            msg_err(f"Bad object {i}")
            result = False
    return result


def are_same_axes(axis1, axis2) -> bool:
    """ Tell whether two axes are same. """
    if not are_valid(axis1, axis2):
        msg_fatal("Bad input objects")
        return False
    # Check classes
    for i, o in enumerate((axis1, axis2)):
        if not isinstance(o, TAxis):
            msg_fatal(f"Object {i} is not an axis")
            return False
    # Check number of bins
    n_bins1, n_bins2 = axis1.GetNbins(), axis2.GetNbins()
    if n_bins1 != n_bins2:
        return False
    # Check bin arrays
    array1 = [axis1.GetBinLowEdge(i + 1) for i in range(n_bins1 + 1)]
    array2 = [axis2.GetBinLowEdge(i + 1) for i in range(n_bins2 + 1)]
    if array1 != array2:
        return False
    return True


def get_object_type(obj) -> int:
    """Return histogram degree"""
    # for num, tp in zip((5, 4, 3, 2, 1), (RooUnfoldResponse, THnSparse, TH3, TH2, TH1)):
    for num, tp in zip((4, 3, 2, 1), (THnSparse, TH3, TH2, TH1)):
        if isinstance(obj, tp):
            return num
    return 0


def are_same_histograms(his1 : TH1, his2 : TH1) -> bool:
    """ Tell whether two histograms are same. """
    if not are_valid(his1, his2):
        msg_fatal("Bad input objects")
        return False
    # Compare number of entries
    if his1.GetEntries() != his2.GetEntries():
        print(f"Different number of entries {his1.GetEntries()} vs {his2.GetEntries()}")
        return False;
    # Compare axes
    for ax1, ax2 in zip((his1.GetXaxis(), his1.GetYaxis(), his1.GetZaxis()),
                        (his2.GetXaxis(), his2.GetYaxis(), his2.GetZaxis())):
        if not are_same_axes(ax1, ax2):
            print("Different axes")
            return False
    # Compare bin counts and errors (include under/overflow bins)
    for bin_z in range(his1.GetNbinsZ() + 2):
        for bin_y in range(his1.GetNbinsY() + 2):
            for bin_x in range(his1.GetNbinsX() + 2):
                bin = his1.GetBin(bin_x, bin_y, bin_z)
                if his1.GetBinContent(bin) != his2.GetBinContent(bin) or \
                    his1.GetBinError(bin) != his2.GetBinError(bin):
                    print(f"Different bin {bin} content: {his1.GetBinContent(bin)} ± {his1.GetBinError(bin)} vs "
                          "{his2.GetBinContent(bin)} ± {his2.GetBinError(bin)}")
                    return False
    return True


def are_same_thnspare(his1 : THnSparse, his2 : THnSparse) -> bool:
    """ Tell whether two THnSparse objects are same. """
    if not are_valid(his1, his2):
        msg_fatal("Bad input objects")
        return False
    # Compare number of dimensions
    if his1.GetNdimensions() != his2.GetNdimensions():
        return False
    # Compare number of entries
    if his1.GetEntries() != his2.GetEntries():
        return False
    # Compare number of filled bins
    if his1.GetNbins() != his2.GetNbins():
        return False
    # Compare axes
    for iAx in range(his1.GetNdimensions()):
        if not are_same_axes(his1.GetAxis(iAx), his2.GetAxis(iAx)):
            return False
    # Compare bin content
    for iBin in range(his1.GetNbins()):
        if his1.GetBinContent(iBin) != his2.GetBinContent(iBin) or his1.GetBinError(iBin) != his2.GetBinError(iBin):
            return False
    return True


# def are_same_response(his1 : RooUnfoldResponse, his2 : RooUnfoldResponse) -> bool:
#     """ Tell whether two RooUnfoldResponse objects are same. """
#     if not are_valid(his1, his2):
#         msg_fatal("Bad input objects")
#         return False
#     # Compare number of dimensions
#     if his1.GetDimensionMeasured() != his2.GetDimensionMeasured() or \
#         his1.GetDimensionTruth() != his2.GetDimensionTruth():
#         return False
#     # Compare number of bins
#     if his1.GetNbinsMeasured() != his2.GetNbinsMeasured() or his1.GetNbinsTruth() != his2.GetNbinsTruth():
#         return False
#     # Compare axes and bin content
#     if not are_same_histograms(his1.Hfakes(), his2.Hfakes()):
#         return False
#     if not are_same_histograms(his1.Hmeasured(), his2.Hmeasured()):
#         return False
#     if not are_same_histograms(his1.Htruth(), his2.Htruth()):
#         return False
#     if not are_same_histograms(his1.Hresponse(), his2.Hresponse()):
#         return False
#     return True


def are_same_objects(obj1, obj2) -> bool:
    """ Tell whether two histogram-like objects are same. """
    if not are_valid(obj1, obj2):
        msg_fatal("Bad input objects")
        return False
    # Compare types
    if type(obj1) is not type(obj2):
        print(f"Different types {type(obj1)} {type(obj2)}")
        return False
    # Get ROOT types
    list_type = [-1, -2]
    for i, o in enumerate((obj1, obj2)):
        list_type[i] = get_object_type(o)
    # Compare ROOT types (is it not covered by type(obj)?)
    if list_type[0] != list_type[1]:
        print(f"Different types {list_type[0]} {list_type[1]}")
        return False
    type_obj = list_type[0]
    # Compare supported ROOT objects
    if type_obj == 0:
        msg_fatal(f"Objects have an unsupported type {type(obj1)}.")
        return False
    # elif type_obj == 5:
    #     return are_same_response(obj1, obj2)
    elif type_obj == 4:
        return are_same_thnspare(obj1, obj2)
    return are_same_histograms(obj1, obj2)


def compare(dict_obj, add_leg_title=True, normalize=True):
    print("Comparing")
    list_colors = ["#e41a1c", "#377eb8", "#4daf4a"]
    list_markers = [21, 20, 34]
    dict_colors = {}
    dict_markers = {}
    dict_list_canvas = {}

    # Explicit comparison
    list_files = list(dict_obj.keys())
    name_file_0 = list_files[0]
    name_file_1 = list_files[1]
    for key_obj in dict_obj[name_file_0]:
        obj_0 = dict_obj[name_file_0][key_obj]
        obj_1 = dict_obj[name_file_1][key_obj]
        name_his = obj_0.GetName()
        if are_same_objects(obj_0, obj_1):
            print(f"Objects {name_his} are same {obj_0.GetEntries()}")
        else:
            print(f"Objects {name_his} are different")

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
            # print(f'Drawing {obj.GetName()} with opt "{opt}" on canvas {gPad.GetName()}')
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
                # print(f'Drawing {obj.GetName()} with opt "{opt}" on canvas {gPad.GetName()}')
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
