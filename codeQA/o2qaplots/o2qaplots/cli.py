import argparse

import ROOT
from o2qaplots.efficiency.efficiency import Efficiency
from o2qaplots.plot1d import Plot1D, Plot2D
from o2qaplots.tracking_resolution.ip.ip import ImpactParameter

ROOT.PyConfig.IgnoreCommandLineOptions = True


def cli():
    """Main entrypoint of the program.
    It redirects the input to the correct task."""

    help_general = "Action to be performed."

    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", help=help_general)

    tasks = [Efficiency, ImpactParameter, Plot1D, Plot2D]

    for t in tasks:
        t.add_to_subparsers(subparsers)

    args = parser.parse_args()
    args_to_pop = ["command"]

    task_arguments = vars(args).copy()

    for arg in args_to_pop:
        task_arguments.pop(arg)

    task_to_run = None

    for task in tasks:
        if args.command == task.parser_command:
            task_to_run = task

    if task is None:
        raise ValueError("Task not defined.")

    task_to_run(**task_arguments).run()


if __name__ == "__main__":
    cli()
