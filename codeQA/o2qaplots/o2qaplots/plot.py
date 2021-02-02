import os
import os.path
import pathlib

import ROOT
from o2qaplots.config import PlotConfig

ROOT.TH1.AddDirectory(False)


def _validate_size(histograms, attribute):
    """Checks if histograms and attributes have the same size."""
    if attribute is not None:
        if len(histograms) != len(attribute):
            raise ValueError(
                "The size of the lists do not match the number of histograms."
            )


def plot_1d(
    histograms_to_plot,
    normalize=False,
    plot_errors=True,
    labels=None,
    colors=None,
    draw_option="",
    plot_ratio=False,
    plot_config=None,
    legend=None,
):
    """Plot a list of histograms to a ROOT.Canvas.

    Args:
        histograms_to_plot: the histograms to be plotted.
        draw_option: this will be passed to the Draw method of the histogram.
        normalize: whether the histograms should be normalized or not.
        labels: The labels to be used in the legend.
        colors: The colors for each histogram.
        plot_errors: if true, the uncertainties in x and y will be plotted.
            Otherwise the plot will use a only a line.
        plot_ratio: is True and len(histograms_to_plot) is 2, it will plot the ratio
            between the two histograms.
        plot_config: PlotConfig object with the plot configuration

    Returns:
        A list with  ROOT.Canvas or matplotlib.Axes with the histograms drawn.
    """

    if len(histograms_to_plot) == 0:
        return []

    if plot_config is None:
        plot_config = PlotConfig()

    _validate_size(histograms_to_plot, labels)
    _validate_size(histograms_to_plot, colors)

    if plot_ratio and len(histograms_to_plot) != 2:
        raise ValueError("Ratio plots can only be used if two histograms are passed.")

    _set_root_global_style()
    _normalize(histograms_to_plot, normalize)
    _set_labels(histograms_to_plot, labels)
    _set_colors(histograms_to_plot, colors)

    common_draw_opt = _get_draw_option(
        histograms_to_plot, draw_option, colors, plot_errors
    )
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
                h.GetYaxis().SetTitle("Relative Frequency")
                h.Scale(1.0 / h.Integral())


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
    if "TH1" not in str(type(histograms_to_plot[0])):
        common_draw_opt += "PE "
    if colors is None:
        common_draw_opt += "PLC PMC"
    if not plot_errors and "TH1" in str(type(histograms_to_plot[0])):
        common_draw_opt += "HIST"
    return common_draw_opt


def _plot_graph(histograms_to_plot, common_draw_opt, draw_option, canvas, plot_ratio):
    canvas.cd()

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


def plot_2d(histogram, draw_option="colz1", labels=None, plot_config=None):
    """Plot a list of histograms to a canvas. """
    canvas = ROOT.TCanvas()
    canvas.cd()
    histogram.Draw(draw_option)

    return canvas


def profile_histogram(histogram, axis, option="", update_title=True):
    """Make a profile (taking mean of each bin) of histogram in de designated axis."""
    _set_root_global_style()

    if axis.lower() == "x":
        profile1 = histogram.ProfileX("_pfx", 1, -1, option)
        if update_title:
            profile1.GetYaxis().SetTitle("< " + histogram.GetYaxis().GetTitle() + " >")
    else:
        profile1 = histogram.ProfileY("_pfy", 1, -1, option)
        if update_title:
            profile1.GetXaxis().SetTitle("< " + histogram.GetXaxis().GetTitle() + " >")
    profile = profile1

    return profile


def plot_profile(histograms, draw_option="", axis="x", option="", **kwargs):
    """Plot a profile histogram, taking the average of each bin"""
    profiles = [profile_histogram(h, axis, option) for h in histograms]

    return plot_1d(profiles, draw_option=draw_option, **kwargs)


def save_canvas(info, canvas_or_ax, base_output_dir, suffix=""):
    """Save a ROOT.TCanvas or a matplotplib Axes into an base_histogram file."""

    output_file = f"{base_output_dir}/{info.full_path}{suffix}.pdf"

    output_dir = os.path.dirname(output_file)
    os.makedirs(output_dir, exist_ok=True)

    try:
        canvas_or_ax.SaveAs(output_file)
    except AttributeError:
        canvas_or_ax.get_figure().savefig(output_file, bbox_inches="tight")

    _check_file_exists(output_file)


def _check_file_exists(file):
    """Checks if file exists.

    Raises:
        FileNotFoundError is file does not exist.
    """
    file_path = pathlib.Path(file)
    if not file_path.is_file():
        raise FileNotFoundError("It was not possible to save_canvas the file: " + file)


def _set_root_global_style():
    """Set the global style for the plots"""

    ROOT.gStyle.SetOptStat(0)
    ROOT.gROOT.SetStyle("ATLAS")
    ROOT.gStyle.SetMarkerStyle(ROOT.kFullSquare)
    ROOT.gROOT.ForceStyle()


def _get_histogram_ranges(histograms_to_plot):
    max_value = max([h.GetMaximum() for h in histograms_to_plot])
    min_value = min([h.GetMinimum() for h in histograms_to_plot] + [0])
    return max_value, min_value
