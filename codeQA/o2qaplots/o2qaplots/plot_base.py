"""
To create your own plotting macro, do the following:

- Derive your plotting class from PlottingTask.

- Define configurables that this task will depend upon in the
  PlottingTask.configurables class attribute.
  They should be declared using the type Configurable. The task configurables
  will follow the same logic of the argparse.ArgumentParser.add_argument method,
  except that they require an additional "name" parameter at the start.

  When creating an instance of the class, the value read from the command line
  argument will be available for this parameter at self.<parameter_name>.

- Define your input files that will be read from each ROOT file. Add a class
  attribute with type TaskInputObj and it will automatically read it for each
  file. So at run time, this will always point to the declared ROOT object
  for the current file.

- Create your own PlottingTask.process function. This will be applied to each
  file and should return a dict with the output that to be saved/plotted.

"""
import argparse
import inspect
import os
import typing

import o2qaplots.config as cfg
import o2qaplots.plot as plot
import ROOT
from o2qaplots.file_utils import check_file_exists

default_json = (
    f"{os.path.dirname(os.path.abspath(__file__))}/config/qa_plot_default.json"
)


class Configurable:
    """An argument to be added to a parser from argparse.
    This argument will become a data member of your plotting task and you will
    be able to access it."""

    def __init__(self, *args, **kwargs):
        """Constructor that mimics argparse.ArgumentParser.add_argument.
        Args:
            *args: configurables to be passed to
                argparse.ArgumentParser.add_argument
            **kwargs: kwargs to be passed to
                argparse.ArgumentParser.add_argument
        """
        self.args = args
        self.kwargs = kwargs

    def add_argument(self, parser):
        """Adds the content of this argument to a argparse.ArgumentParser"""
        parser.add_argument(*self.args, **self.kwargs)

    def __repr__(self):
        return f"<{self.__class__.__name__}(args={self.args}, kwargs={self.kwargs})>"


class InputConfigurable(Configurable):
    """Special case of configurable that can modify the path of the input
    objects."""

    pass


class ROOTObj:
    """Stores the representation of a input ROOT object that can be read
    from a file.

    Attributes:
        path: a list with the path in the ROOT file. This is NOT the path to
            the file, but rather the TDirectories.
        name: the name of the object to be read from the file.
    """

    def __init__(self, path: str):
        obj_path = path.split("/")
        self.path = obj_path[:-1]
        self.name = obj_path[-1]

    @property
    def full_path(self):
        return "/".join(self.path) + "/" + self.name

    def get(self, input_file):
        file = ROOT.TFile(input_file)
        return file.Get(self.full_path)

    def with_input(self, input_argument=None):
        """In case your task has input configurables that can change the name of
        your structure of the ROOT folders, this function will generate a new
        object taking into account the input argument. The values will be
        passed to the first directory of the object.

        Args:
            input_argument: a string or list with the input configurables.

        Returns:
            an instance of this class taking into account the input_arguments.
        """
        if input_argument is None:
            return self

        if not isinstance(input_argument, str):
            input_argument = "-".join(input_argument)

        path = self.path[:]

        if path:
            path[0] += "-" + input_argument
        else:
            path.append(input_argument)

        return self.__class__("/".join(path + [self.name]))

    def add_to_path(self, additional_path):
        """Retuns a new object with the addition of additional_path at the
        beggining of the path"""

        return self.__class__(f"{additional_path}/{self.full_path}")

    def __hash__(self):
        return hash(self.full_path)

    def __eq__(self, other):
        return self.full_path == other.full_path

    def __repr__(self):
        return f"<{self.__class__.__name__}({self.full_path})>"


class TaskInput(ROOTObj):
    """Stores the representation of an input object.
    You should define your inputs using this class."""

    pass


def find_class_instances(class_, class_to_find) -> typing.List[str]:
    """Finds all the data members of class_ that are from class_to_find.

    Args:
        class_: the class which will be inspected.
        class_to_find: the class that the data members should match.

    Returns:
        A list with the data members with type class_to_find.
    """
    return [
        name
        for name, _ in inspect.getmembers(
            class_, lambda x: isinstance(x, class_to_find)
        )
    ]


class PlottingTask:
    """Base class to perform the plotting. It does contains basic functions to
    read the files and utilities.
    You should derive your class from it and override the methods.

    Attributes:
        parser_description: string with the description which will be
            shown when this plotting macro is run from the command line.
        arguments: list with the configurables for the process.
    """

    parser_description: str = "Put here the parser description"
    parser_command = "here_goes_the_parser_command"

    files = Configurable("files", type=str, nargs="+", help="Analysis files.")

    labels = Configurable("--labels", "-l", type=str, nargs="+", help="Legend labels")

    output = Configurable("--output", "-o", help="Output folder", default="qa_output")

    config = Configurable(
        "--config", "-c", help="JSON configuration", default=default_json
    )

    suffix = Configurable(
        "--suffix", "-s", type=str, help="Suffix to the output", default=""
    )

    save_output = True

    plotting_function = plot.plot_1d
    plotting_kwargs = dict()

    output_file = "LocalTaskResults.root"

    def __init__(self, **kwargs):
        """Constructor for the process. Do not modify it when inheriting,
        it will be automatically generated based on the configurables specified
        in the class definition."""
        cls = self.__class__

        # Initialize all configurables and input arguments
        for arg in cls.configurables():
            try:  # Read from class constructor
                setattr(self, arg, kwargs[arg])
            except KeyError:
                try:  # Set from default value defined in class
                    default_value = getattr(self, arg).kwargs["default"]
                    setattr(self, arg, default_value)
                except KeyError:  # if nothing is provided, fallbakcs to None
                    setattr(self, arg, None)

        self.input_arguments = None
        if cls.input_configurables():
            self.input_arguments = [
                getattr(self, arg) for arg in cls.input_configurables()
            ]

        self.json_config = cfg.JsonConfig(self.config)
        self.output_objects = []
        self.file = None

    @classmethod
    def input(cls):
        """Returns a list with the name of the inputs."""
        return find_class_instances(cls, TaskInput)

    @classmethod
    def input_configurables(cls):
        """Returns a list of the class members that are input arguments."""
        return find_class_instances(cls, InputConfigurable)

    @classmethod
    def configurables(cls):
        """Returns a list of the class members that are configurables."""
        return find_class_instances(cls, Configurable)

    def get_input_from_file(self, file):
        """Gets the input from a file.

        Args:
            file: the file that will be used to read the input objects.

        Returns:
            input_objs: dictionary with ROOTObj: <Object> for the inputs of this task.

        """
        cls = self.__class__

        input_objs = {
            attr: getattr(cls, attr).with_input(self.input_arguments).get(file)
            for attr in cls.input()
        }

        return input_objs

    def _check_consistency(self):
        """Check if the input of the user is valid.  The following checks are performed:

        - Files declared exists.
        - If labels were passed, if they have the same length as the number of files.

        Raises:
            ValueError: if the number of labels is different from the number of files.
            FileNotFoundError: if any of the files in self.files do not exist.

        """
        for f in self.files:  # pylint: disable=not-an-iterable
            check_file_exists(f)

        if self.labels is not None:
            if len(self.labels) != len(self.files):
                raise ValueError(
                    f"The length of labels ({len(self.labels)}) is "
                    f"different from the length of files ({len(self.files)}"
                )

    def _set_input_for_current_file(self):
        """Reads the input objects from file and returns them."""
        for attr, obj in self.get_input_from_file(self.file).items():
            setattr(self, attr, obj)

    def _get_output_objects_info(self):
        """Return a list-like with the histograms that have to be saved."""
        if len(self.output_objects) == 0:
            return []

        return self.output_objects[0].keys()

    def _get_results_from_all_files(self, output_ojt):
        """Returns a list will a particular output object"""
        return [output[output_ojt] for output in self.output_objects]

    def process(self):
        """Process the objects of input_objects. This should be the input for a
        single file.
        It should return the output as a dict in the form {HistogramInfo: histogram}.
        """
        pass

    def run(self):
        """Process the task."""
        self._check_consistency()

        self.process_files()

        self.save_figures()

        if self.save_output:
            self.save_root_output()

    def process_files(self):
        for f in self.files:  # pylint: disable=not-an-iterable
            self.file = f
            self._set_input_for_current_file()
            self.output_objects.append(self.process())

    def save_figures(self):
        """Save the output figures to PDF files."""
        cls = self.__class__
        plotted_canvas = []

        for result in self._get_output_objects_info():
            result_objects_list = self._get_results_from_all_files(result)

            canvas = cls.plotting_function(
                result_objects_list,
                labels=self.labels,
                plot_config=self.json_config.get(result.name),
                **self.plotting_kwargs,
            )

            plotted_canvas.append(canvas)

            plot.save_canvas(
                result.with_input(self.input_arguments),
                canvas,
                self.output,
                self.suffix,
            )
        return plotted_canvas

    def save_root_output(self):
        root_output_file = ROOT.TFile(f"{self.output}/{self.output_file}", "RECREATE")
        root_output_file.cd()

        labels = self.labels

        if labels is None:
            if len(set(self.files)) == len(self.files):
                labels = self.files
            else:
                labels = [str(i) for i in range(len(self.files))]

        for output, label in zip(self.output_objects, labels):
            for root_info, opt_obj in output.items():
                opt_obj.Write(root_info.add_to_path(label).full_path)

        root_output_file.Close()

    @classmethod
    def add_parser_options(cls, parser):
        """Add the configurables from this task to parser."""
        for arg in cls.configurables():
            getattr(cls, arg).add_argument(parser)

    @classmethod
    def add_to_subparsers(cls, subparsers):
        sub = subparsers.add_parser(
            cls.parser_command, description=cls.parser_description
        )
        cls.add_parser_options(sub)


def macro(task_class):
    """Instance to run as the main entrypoint of a program or/and scripting.
    Call this function with to make a script macro.

    Args:
        task_class: the class, derived from PlottingTask, which will be used to
            run this script.
    """
    parser_main = argparse.ArgumentParser(description=task_class.parser_description)
    task_class.add_parser_options(parser_main)
    task_class(**vars(parser_main.parse_args())).run()
