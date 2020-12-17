import argparse
import os
import os.path
import pathlib

import ROOT

ROOT.TH1.AddDirectory(False)

from o2qaplots.config import JsonConfig
from o2qaplots.file_utils import discover_histograms, ROOTObjInfo
import o2qaplots.config as cfg

parser_description = 'This option will plots all the histograms (1D - TH1 and 2D- TH2) from a ROOT file into PDFs. ' \
                     'The histograms in the file are discovered automatically. For the TH2, automatic ' \
                     'follow the same structure as they have in the ROOT files. It is useful to produce the QA presentation.'

_base_folder = os.path.dirname(os.path.abspath(__file__))
default_config_file = _base_folder + '/config/qa_plot_default.json'


def get_object(input_file: str, obj_info: ROOTObjInfo):
    """ Reads an histogram located in input_file. It must be in a ROOT.TDirectory called category_folder and be called
    histogram_name.

    Args:
        input_file: the name of the ROOT file with the histogram.
        obj_info: ROOTObjInfo with the information to find the file.
    Returns:
        The histogram pointed. The type of the object depends on the chosen backend.
    """
    _set_root_global_style()

    file = ROOT.TFile(input_file)

    sub_folders = obj_info.path
    histogram_name = obj_info.name

    try:
        folder = file
        for sub_folder in sub_folders:
            if sub_folder is not None:
                folder = folder.Get(sub_folder)

        obj = folder.Get(histogram_name).Clone()
    except AttributeError:
        error_message = f'It is not possible to read the input in the path: {obj_info.path + [obj_info.name]} ' \
                        f'in the file {input_file}.'
        raise ValueError(error_message) from None

    return obj


def read_histograms(file, histogram_info_list):
    for f in file:
        yield {histos: {h: get_object(f, h) for h in histos} for histos in histogram_info_list}


def _validate_size(histograms, attribute):
    """Checks if histograms and attributes have the same size."""
    if attribute is not None:
        if len(histograms) != len(attribute):
            raise ValueError("The size of the lists do not match the number of histograms.")


def plot_1d(histograms_to_plot, normalize=False, plot_errors=True, labels=None, colors=None,
            draw_option='', plot_ratio=False, plot_config=None, legend=None):
    """Plot a list of histograms to a ROOT.Canvas.

    Args:
        histograms_to_plot: the histograms to be plotted.
        draw_option: this will be passed to the Draw method of the histogram.
        normalize: whether the histograms should be normalized or not.
        labels: The labels to be used in the legend.
        colors: The colors for each histogram.
        plot_errors: if true, the uncertainties in x and y will be plotted. Otherwise the plot will use a only a line.
        plot_ratio: is True and len(histograms_to_plot) is 2, it will plot the ratio between the two histograms.
        plot_config: PlotConfig object with the plot configuration

    Returns:
        A list with  ROOT.Canvas or matplotlib.Axes with the histograms drawn.
    """

    if len(histograms_to_plot) == 0:
        return []

    if plot_config is None:
        plot_config = cfg.PlotConfig()

    _validate_size(histograms_to_plot, labels)
    _validate_size(histograms_to_plot, colors)

    if plot_ratio and len(histograms_to_plot) != 2:
        raise ValueError("Ratio plots can only be used if two histograms are passed.")

    _set_root_global_style()
    _normalize(histograms_to_plot, normalize)
    _set_labels(histograms_to_plot, labels)
    _set_colors(histograms_to_plot, colors)

    common_draw_opt = _get_draw_option(histograms_to_plot, draw_option, colors, plot_errors)

    canvas = ROOT.TCanvas()
    _set_log(canvas, plot_config)
    _set_axis_range(histograms_to_plot, plot_config)
    _plot_graph(histograms_to_plot, common_draw_opt, draw_option, canvas, plot_ratio)

    if legend is None:
        _build_legend(canvas, labels)
    else:
        legend.Draw()

    canvas.Update()

    return canvas


def _normalize(histograms_to_plot, normalize):
    if normalize:
        for h in histograms_to_plot:
            if h.Integral() > 0:
                h.GetYaxis().SetTitle('Relative Frequency')
                h.Scale(1. / h.Integral())


def _set_axis_range(histograms_to_plot, plot_config):
    if plot_config.y_axis.view_range is not None:
        for h1 in histograms_to_plot:
            h1.GetYaxis().SetRangeUser(*plot_config.y_axis.view_range)
    else:
        if len(histograms_to_plot) > 1:
            max_value, min_value = _get_histogram_ranges(histograms_to_plot)
            for h1 in histograms_to_plot:
                h1.GetYaxis().SetRangeUser(min_value, 1.1 * max_value)
    if plot_config.x_axis.view_range is not None:
        for h1 in histograms_to_plot:
            h1.GetXaxis().SetRangeUser(*plot_config.x_axis.view_range)


def _set_labels(histograms_to_plot, labels):
    if labels is not None:
        for h1, label in zip(histograms_to_plot, labels):
            h1.SetTitle(label)


def _set_colors(histograms_to_plot, colors):
    if colors is not None:
        for h1, color in zip(histograms_to_plot, colors):
            h1.SetLineColor(color)
            h1.SetMarkerColor(color)


def _get_draw_option(histograms_to_plot, draw_option, colors, plot_errors):
    common_draw_opt = ""
    if 'TGraph' in str(type(histograms_to_plot[0])) and not draw_option:
        common_draw_opt += "AP "
    if colors is None:
        common_draw_opt += "PLC PMC"
    if not plot_errors and 'TH1' in str(type(histograms_to_plot[0])):
        common_draw_opt += 'HIST'
    return common_draw_opt


def _plot_graph(histograms_to_plot, common_draw_opt, draw_option, canvas, plot_ratio):
    canvas.cd()
    if plot_ratio:
        ratio_plot = ROOT.TRatioPlot(histograms_to_plot[0], histograms_to_plot[1])
        ratio_plot.Draw()
    else:
        histograms_to_plot[0].Draw(draw_option + common_draw_opt)

        for h in histograms_to_plot[1:]:
            h.Draw(draw_option + " SAME " + common_draw_opt)


def _set_log(canvas, plot_config):
    if plot_config.x_axis.log:
        canvas.SetLogx()
    if plot_config.y_axis.log:
        canvas.SetLogy()


def _build_legend(canvas, labels):
    if labels is not None:
        legend = canvas.BuildLegend()
        legend.SetLineWidth(0)
        legend.SetBorderSize(0)
        legend.SetFillStyle(0)


def plot_2d(histogram, draw_option='colz1'):
    """Plot a list of histograms to a canvas. """
    import ROOT
    canvas = ROOT.TCanvas()
    canvas.cd()
    histogram.Draw(draw_option)
    return canvas


def profile_histogram(histogram, axis, option="", update_title=True):
    """Make a profile (taking mean of each bin) of histogram in de designated axis."""
    _set_root_global_style()

    if axis.lower() == 'x':
        profile1 = histogram.ProfileX("_pfx", 1, -1, option)
        if update_title:
            profile1.GetYaxis().SetTitle('< ' + histogram.GetYaxis().GetTitle() + ' >')
    else:
        profile1 = histogram.ProfileY("_pfy", 1, -1, option)
        if update_title:
            profile1.GetXaxis().SetTitle('< ' + histogram.GetXaxis().GetTitle() + ' >')
    profile = profile1

    return profile


def plot_profile(histograms, draw_option='', axis='x', option="", **kwargs):
    """Plot a profile histogram, taking the average of each bin"""
    profiles = [profile_histogram(h, axis, option) for h in histograms]

    return plot_1d(profiles, draw_option=draw_option, **kwargs)


def save_canvas(info: ROOTObjInfo, canvas_or_ax, base_output_dir, suffix=''):
    """Save a ROOT.TCanvas or a matplotplib Axes into an base_histogram file."""

    output_file = base_output_dir + '/' + '/'.join(info.path) + '/' + info.name + suffix + '.pdf'

    output_dir = os.path.dirname(output_file)
    os.makedirs(output_dir, exist_ok=True)

    try:
        canvas_or_ax.SaveAs(output_file)
    except AttributeError:
        canvas_or_ax.get_figure().savefig(output_file, bbox_inches='tight')

    _check_file_exists(output_file)


def _check_file_exists(file):
    """Checks if file exists.

    Raises:
        FileNotFoundError is file does not exist.
    """
    file_path = pathlib.Path(file)
    if not file_path.is_file():
        raise FileNotFoundError("It was not possible to save_canvas the file: " + file)


def plot_histograms(file_name, output_dir, normalize,
                    plot_config_file=default_config_file,
                    automatic_profile_th2=True):
    json_config = JsonConfig(plot_config_file)
    histograms_info = discover_histograms(file_name)

    histograms = {h: get_object(file_name, h) for h in histograms_info}
    histograms_1d_keys = [x for x in histograms.keys() if x.root_class.startswith('TH1')]
    histograms_2d_keys = [x for x in histograms.keys() if x.root_class.startswith('TH2')]

    plot_hist_1d = {info: plot_1d([histograms[info]], normalize, False, plot_config=json_config.get(info.name))
                    for info in histograms_1d_keys}

    for info, canvas in plot_hist_1d.items():
        save_canvas(info, canvas, output_dir)

    plot_hist_2d = {info: plot_2d(histograms[info]) for info in histograms_2d_keys}

    for info, canvas in plot_hist_2d.items():
        save_canvas(info, canvas, output_dir)

    if automatic_profile_th2:
        plot_hist_2d_profile_x = {info: plot_profile(histograms[info], axis='x', plot_config=json_config.get(info.name))
                                  for info in histograms_2d_keys}
        plot_hist_2d_profile_y = {info: plot_profile(histograms[info], axis='y', plot_config=json_config.get(info.name))
                                  for info in histograms_2d_keys}

        for info, canvas in plot_hist_2d_profile_x.items():
            save_canvas(info, canvas, output_dir, '_profileX')

        for info, canvas in plot_hist_2d_profile_y.items():
            save_canvas(info, canvas, output_dir, '_profileY')


def add_parser_options(parser):
    parser.add_argument('file', help='Location of the analysis results file to be plotted')

    parser.add_argument('--output', '-o',
                        help='Location to save_canvas the produced files',
                        default="qa_output")

    parser.add_argument('--normalize', '-n', help='Normalize histograms by the integral.',
                        action='store_true',
                        default=False)

    parser.add_argument('--plot_config_file', '-c', help='Normalize histograms by the integral.',
                        default=default_config_file)


def plot(args=None):
    """Entrypoint function to parse the arguments and make plots of single datasets. """

    if args is None:
        main_parser = argparse.ArgumentParser(description=parser_description)
        add_parser_options(main_parser)
        args = main_parser.parse_args()

    plot_histograms(args.file, args.output, args.normalize)


if __name__ == '__main__':
    plot()


def _set_root_global_style():
    """Set the global style for the plots"""

    ROOT.gStyle.SetOptStat(0)
    ROOT.gROOT.SetStyle('ATLAS')
    ROOT.gStyle.SetMarkerStyle(ROOT.kFullSquare)
    ROOT.gROOT.ForceStyle()


def _get_histogram_ranges(histograms_to_plot):
    max_value = max([h.GetMaximum() for h in histograms_to_plot])
    min_value = min([h.GetMinimum() for h in histograms_to_plot] + [0])
    return max_value, min_value


def _normalize_histograms(histograms_to_plot):
    for h in histograms_to_plot:
        if h.Integral() > 0:
            h.GetYaxis().SetTitle('Relative Frequency')
            h.Scale(1. / h.Integral())
