import ROOT

import o2qaplots.efficiency.efficiency as eff

ROOT.TH1.AddDirectory(False)


def set_all_bin_contents(hist: ROOT.TH3D, value: float):
    for x in range(1, hist.GetNbinsX()):
        for y in range(1, hist.GetNbinsY()):
            for z in range(1, hist.GetNbinsZ()):
                hist.SetBinContent(x, y, z, value)


def print_all_bin_content(hist):
    for x in range(1, hist.GetNbinsX()):
        for y in range(1, hist.GetNbinsY()):
            for z in range(1, hist.GetNbinsZ()):
                print(hist.GetBinContent(x, y, z))


def make_3d_with_fixed_value(hist=None, n_fill=1.):
    if hist is None:
        hist = ROOT.TH3D("base_hist", "", 10, 0, 10, 6, -3, 3, 10, 0, 2 * ROOT.TMath.Pi())

    set_all_bin_contents(hist, n_fill)

    return hist


def test_calculate_efficiency(generated=make_3d_with_fixed_value(n_fill=600 * 1000.),
                              reconstructed=make_3d_with_fixed_value(n_fill=0.7 * 600 * 1000.)):

    efficiency = eff.calculate_efficiency(reconstructed, generated).CreateGraph()

    for i in range(0, efficiency.GetN()):
        assert efficiency.GetPointY(i) == 0.7
