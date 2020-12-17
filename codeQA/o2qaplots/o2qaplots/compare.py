import argparse


import o2qaplots.plot_base as pb
from o2qaplots.config import JsonConfig
from o2qaplots.plot import discover_histograms, plot_1d, plot_profile, get_object, save_canvas, default_config_file

parser_description = 'Compare the results of two files'


class ComparePlotsTask(pb.PlottingTask):
    parser_description = 'Compare the results of N files.'

    def process(self):
        pass


def compare_histograms(files, output_dir, normalize, label_legend, ratio,
                       plot_config_file=default_config_file, colors=None,
                       histograms_info=None,
                       make_profile=True):
    json_config = JsonConfig(plot_config_file)

    if histograms_info is None:
        histograms_info = discover_histograms(files[0])

    histograms = [{h: get_object(f, h) for h in histograms_info} for f in files]

    histograms_1d_keys = [x for x in histograms[0].keys() if x.root_class.startswith('TH1')]

    plot_hist_1d = {info: plot_1d([h[info] for h in histograms], normalize=normalize, labels=label_legend,
                                  colors=colors, plot_errors=False, plot_ratio=False,
                                  plot_config=json_config.get(info.name))
                    for info in histograms_1d_keys}

    for info, canvas in plot_hist_1d.items():
        save_canvas(info, canvas, output_dir)

    if make_profile:
        histograms_2d_keys = [x for x in histograms[0].keys() if x.root_class.startswith('TH2')]

        plot_hist_2d_profile = {info: plot_profile([h[info] for h in histograms], axis='x',
                                                   labels=label_legend, colors=colors, plot_errors=True,
                                                   plot_ratio=False,
                                                   draw_option='APE',
                                                   plot_config=json_config.get(info.name))
                                for info in histograms_2d_keys}

        for info, canvas in plot_hist_2d_profile.items():
            save_canvas(info, canvas, output_dir, '_profile')


def compare(args, histograms_info=None):
    compare_histograms(args.files, args.output_objects, args.normalize, args.labels, not args.no_ratio,
                       histograms_info=histograms_info)


def add_parser_options(parser):
    parser.add_argument('files', type=str, nargs="+",
                        help='Location of the analysis results file to be plotted')

    parser.add_argument('-l', '--labels', type=str, nargs="+",
                        help='Labels to put on the plots', default=None)

    parser.add_argument('--output', '-o', help='Location to save_canvas the produced files', default="qa_output")
    parser.add_argument('--normalize', '-n', help='Normalize by the integral.', action='store_true', default=False)
    parser.add_argument('--no_ratio', '-nr', help='Do not plot the ratio plot.', action='store_true', default=False)


if __name__ == '__main__':
    parser_main = argparse.ArgumentParser(description=parser_description)
    add_parser_options(parser_main)
    args_main = parser_main.parse_args()

    compare(args_main)
