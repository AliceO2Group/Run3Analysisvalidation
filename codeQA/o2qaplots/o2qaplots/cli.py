import o2qaplots.efficiency.efficiency as eff
import o2qaplots.tracking_resolution.ip.ip as ip
import o2qaplots.compare as compare
import o2qaplots.plot as plot
import argparse

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

def cli():
    """Main entrypoint of the program. 
    It redirects the input to the correct task."""

    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(
        dest='command', help='Action to be performed.')

    plot_parser = subparsers.add_parser(
        'plot', description=plot.parser_description)
    plot.add_parser_options(plot_parser)

    compare_parser = subparsers.add_parser(
        'compare', description=compare.parser_description)
    compare.add_parser_options(compare_parser)

    impact_parameter_parser = subparsers.add_parser(
        'ip', description=ip.ImpactParameter.parser_description)
    ip.ImpactParameter.add_parser_options(impact_parameter_parser)

    efficiency_parser = subparsers.add_parser(
        'eff', description=eff.Efficiency.parser_description)
    eff.Efficiency.add_parser_options(efficiency_parser)

    args = parser.parse_args()
    args_to_pop = ['command']

    task_arguments = vars(args).copy()

    for arg in args_to_pop:
        task_arguments.pop(arg)

    if args.command == 'plot':
        plot.plot(args)
    elif args.command == 'compare':
        compare.compare(args)
    elif args.command == 'ip':
        ip.ImpactParameter(**task_arguments).run()
    elif args.command == 'eff':
        eff.Efficiency(**task_arguments).run()
    else:
        raise ValueError("The argument " + str(args.command) + " is invalid.")


if __name__ == '__main__':
    cli()
