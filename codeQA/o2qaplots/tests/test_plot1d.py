import os

import o2qaplots.plot1d as plot1d
import ROOT


def test_discover_root_objects():
    file_test = ROOT.TFile("test_file.root", "RECREATE")

    ROOT.TH1D().Write("um")
    ROOT.TH1D().Write("dois/tres")
    ROOT.TH1D().Write("quatro/cinco/seis")
    ROOT.TH1D().Write("quatro/cinco/sete/oito")

    file_test.Close()

    plots = plot1d.discover_root_objects(
        ROOT.TFile("test_file.root"), lambda x: "TH1D" in x
    )

    assert set(plots) == {
        "um",
        "dois/tres",
        "quatro/cinco/seis",
        "quatro/cinco/sete/oito",
    }

    os.remove("test_file.root")
