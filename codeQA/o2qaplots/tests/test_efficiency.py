import o2qaplots.efficiency.efficiency as eff
import ROOT

ROOT.TH1.AddDirectory(False)


def set_all_bin_contents(hist: ROOT.TH3D, value: float):
    for x in range(0, hist.GetNbinsX() + 2):
        for y in range(0, hist.GetNbinsY() + 2):
            for z in range(0, hist.GetNbinsZ() + 2):
                hist.SetBinContent(x, y, z, value)


def print_all_bin_content(hist):
    for x in range(1, hist.GetNbinsX()):
        for y in range(1, hist.GetNbinsY()):
            for z in range(1, hist.GetNbinsZ()):
                print(hist.GetBinContent(x, y, z))


def make_3d_with_fixed_value(hist=None, n_fill=1.0):
    if hist is None:
        hist = ROOT.TH3D(
            "base_hist", "", 10, 0, 10, 6, -3, 3, 10, 0, 2 * ROOT.TMath.Pi()
        )

    set_all_bin_contents(hist, n_fill)

    return hist


def test_calculate_efficiency(
    generated=make_3d_with_fixed_value(n_fill=600 * 1000.0),
    reconstructed=make_3d_with_fixed_value(n_fill=0.7 * 600 * 1000.0),
):
    """Creates a generated and reconstructed histogram with 0.7 efficiency
    and calculates it."""

    efficiency = eff.calculate_efficiency(reconstructed, generated)

    for i in range(1, efficiency.GetXaxis().GetNbins() + 1):
        assert efficiency.GetBinContent(i) == 0.7
