import o2qaplots.plot_base as plot_base


def test_argument_reading():
    class PtSpectra(plot_base.PlottingTask):
        pt_range = plot_base.Configurable(
            "--pt_range",
            "-pt",
            default=[0.0, 10.0],
            nargs=2,
            action="append",
            type=float,
            help="Cut in pt_range[0] < pt <= pt_range[1].",
        )

        particle = plot_base.InputConfigurable(
            "-p",
            "--particle",
            help="particle to be processed",
            type=str,
            choices=["electron", "pion", "kaon", "muon", "proton"],
            default="pion",
        )

    pt_task_defaults = PtSpectra(files=["pt.root"])
    print(pt_task_defaults.pt_range)
    assert pt_task_defaults.pt_range == [0.0, 10.0]
    assert pt_task_defaults.particle == "pion"

    pt_task_arguments = PtSpectra(files=["pt.root"], pt_range=[5, 20], particle="d0")

    assert pt_task_arguments.pt_range == [5.0, 20.0]
    assert pt_task_arguments.particle == "d0"


def test_root_obj_include_task_argument():
    """Tests if plot_base.ROOTObj.with_input does the correct routing when
    including the input arguments."""

    no_folder = plot_base.ROOTObj("some-object")
    assert no_folder.with_input() is no_folder

    no_folder_str_arg = no_folder.with_input("pions")
    assert no_folder_str_arg == plot_base.ROOTObj("pions/some-object")

    no_folder_list_arg = no_folder.with_input(["pions", "low_pt"])
    assert no_folder_list_arg == plot_base.ROOTObj("pions-low_pt/some-object")

    one_folder = plot_base.ROOTObj("folder/object")

    one_folder_str_arg = one_folder.with_input("pions")
    assert one_folder_str_arg == plot_base.ROOTObj("folder-pions/object")

    one_folder_list_arg = one_folder.with_input(["pions", "low_pt"])
    assert one_folder_list_arg == plot_base.ROOTObj("folder-pions-low_pt/object")


def test_find_class_instances():
    """ Construct simple cases to test find_class_instances."""

    class Input:
        pass

    class Output:
        pass

    class Task:
        pt = Input()
        eta = Input()

        pt_histogram = Output()
        eta_histogram = Output()

    assert set(plot_base.find_class_instances(Task, Input)) == set(["pt", "eta"])
    assert set(plot_base.find_class_instances(Task, Output)) == set(
        ["pt_histogram", "eta_histogram"]
    )
    # Add more tests?
