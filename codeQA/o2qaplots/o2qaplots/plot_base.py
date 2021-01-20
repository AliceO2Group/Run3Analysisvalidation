"""
To create your own plotting macro, do the following:

- Derive your plotting class from PlottingTask.

- Define configurables that this task will depend upon in the PlottingTask.configurables class attribute.
  They should be declared using the type Configurable.
  The task configurables will follow the same logic of the argparse.ArgumentParser.add_argument method, except that they
  require an additional "name" parameter at the start.
  When creating an instance of the class, the value read from the command line argument will be available
  for this parameter at self.<parameter_name>.

- Define your input files that will be read from each ROOT file. Add a class attribute with type TaskInputObj and it
  will automatically read it for each file. So at run time, this will always point to the declared ROOT object
  for the current file.

- Create your own PlottingTask.process function. This will be applied to each file and should return a dict with the
  output that to be saved/plotted.

"""
import argparse
import pathlib
import typing

import ROOT

import o2qaplots.config as cfg
import o2qaplots.plot as plot


class Configurable:
    """An argument to be added to a parser from argparse. This argument will become a data member of your plotting task
    and you will be able to access it with """

    def __init__(self, name, *args, **kwargs):
        """ Constructor that mimics argparse.ArgumentParser.add_argument. The first value passed should be the name of
        the parameter and the following args and kwargs will be forwarded to argparse.ArgumentParser.add_argument.
        Args:
            name: this will become the name of an attribute that the you can read in your process function.
            *args: configurables to be passed to argparse.ArgumentParser.add_argument
            **kwargs: kwargs to be passed to argparse.ArgumentParser.add_argument
        """
        self.name = name
        self.args = args
        self.kwargs = kwargs

    def add_argument(self, parser):
        """Adds the content of this argument to a argparse.ArgumentParser"""
        parser.add_argument(*self.args, **self.kwargs)


class ROOTObj:
    """Stores the representation of a input ROOT object that can be read from a file.

    Attributes:
        path: a list with the path in the ROOT file. This is NOT the path to the file, but rather the TDirectories.
        name: the name of the object to be read from the file.
    """

    def __init__(self, path: typing.List[str], name: str):
        self.path = path
        self.name = name

    def get(self, input_file):
        file = ROOT.TFile(input_file)
        return file.Get('/'.join(self.path) + '/' + self.name)

    def include_task_argument(self, input_argument=None):
        """In case your task has input configurables that can change the name of your structure of the ROOT folders,
        this function will generate a new ROOTObj taking into account the input argument. The values will be passed
        to the first directory of the ROOTObj.

        Args:
            input_argument: a string or list with the input configurables.

        Returns:
            an instance of this class taking into account the input_arguments.
        """
        if input_argument is None:
            return self

        if isinstance(input_argument, list):
            input_argument = '-'.join(input_argument)

        path = self.path[:]
        path[0] += '-' + input_argument

        return self.__class__(path, self.name)

    def __hash__(self):
        return hash('/'.join(self.path) + '/' + self.name)


class TaskInputObj(ROOTObj):
    """Stores the representation of an input object. You should define your inputs using this class"""
    pass


def _check_file_exists(file):
    """Checks if file exists.

    Raises:
        FileNotFoundError is file does not exist.
    """
    file_path = pathlib.Path(file)
    if not file_path.is_file():
        raise FileNotFoundError("It was not possible find the file: " + file)


class PlottingTask:
    """ Base class to perform the plotting. It does contains basic functions to read the files and utilities.
    You should derive your class from it and override the methods.

    Attributes:
        parser_description: string with the description which will be shown when this plotting macro is run from the
            command line.
        base_arguments: list with the common configurables for any process. Each argument should have the type C
            onfigurable.
        arguments: list with the configurables for the process.

    """
    parser_description: str = None

    base_configurables = [
        Configurable('files', 'files', type=str, nargs='+', help='Location of the analysis results file to be plotted'),
        Configurable('labels', '-l', '--labels', type=str, nargs="+", help='Labels to put on the plots', default=None),
        Configurable('output', '--output', '-o', help='Location to save_canvas the produced files',
                     default="qa_output"),
        Configurable('config', '--config', '-c', help='Location of the JSON config file with the plot configurations',
                     default=plot.default_config_file),
        Configurable('suffix', '--suffix', '-s', type=str, help='Suffix to add to the saved files', default='')
    ]

    input_argument = None

    configurables = []

    plotting_function = plot.plot_1d
    plotting_kwargs = dict()

    @classmethod
    def input(cls):
        """Returns a list with the name of the inputs."""
        return [key for key, value in vars(cls).items() if isinstance(value, TaskInputObj)]

    def get_input_from_file(self, file):
        """Gets the input from a file.

        Args:
            file: the file that will be used to read the input objects.

        Returns:
            input_objs: dictionary with ROOTObj: <Object> for the inputs of this task.

        """
        cls = self.__class__

        input_objs = {attr: cls.__dict__[attr].include_task_argument(self.input_argument).get(file)
                      for attr in cls.input()}

        return input_objs

    @classmethod
    def input_argument_list(cls):
        if cls.input_argument is None:
            return []
        return [cls.input_argument]

    def __init__(self, **kwargs):
        """Constructor for the process. Do not modify it when inheriting, it will be automatically generated based on
        the configurables specified in the class definition. """

        for arg in self.base_configurables + self.configurables + self.input_argument_list():
            self.__setattr__(arg.name, kwargs[arg.name])

        if self.__class__.input_argument is not None:
            self.input_argument = self.__getattribute__(self.__class__.input_argument.name)

        self.json_config = cfg.JsonConfig(self.config)
        self.output_objects = []
        self.file = None

    def _check_consistency(self):
        """Check if the input of the user is valid.  The following checks are performed:

        - Files declared exists.
        - If labels were passed, if they have the same length as the number of files.

        Raises:
            ValueError: if the number of labels is different from the number of files.
            FileNotFoundError: if any of the files in self.files do not exist.

        """
        for f in self.files:
            _check_file_exists(f)

        if self.labels is not None:
            if len(self.labels) != len(self.files):
                raise ValueError(f"The length of labels ({len(self.labels)}) is "
                                 f"different from the length of files ({len(self.files)}")

    def _set_input_for_current_file(self):
        """Reads the input objects from file and returns them."""
        for attr, obj in self.get_input_from_file(self.file).items():
            self.__setattr__(attr, obj)

    def _get_output_objects_info(self):
        """Return a list-like with the histograms that have to be saved."""
        if len(self.output_objects) == 0:
            return []

        return self.output_objects[0].keys()

    def _get_results_from_all_files(self, output_ojt):
        """Returns a list will a particular output object """
        return [output[output_ojt] for output in self.output_objects]

    def process(self):
        """Process the objects of input_objects. This should be the input for a single file.
        It should return the output as a dict in the form {HistogramInfo: histogram}."""
        pass

    def run(self):
        self._check_consistency()

        for file in self.files:
            self.file = file
            self._set_input_for_current_file()
            self.output_objects.append(self.process())

        output_list = self._get_output_objects_info()

        for result in output_list:
            result_objects_list = self._get_results_from_all_files(result)

            canvas = self.__class__.plotting_function(result_objects_list, labels=self.labels,
                                                      plot_config=self.json_config.get(result.name),
                                                      **self.plotting_kwargs)

            plot.save_canvas(result.include_task_argument(self.input_argument),
                             canvas, self.output, self.suffix)

    @classmethod
    def add_parser_options(cls, parser):
        for task_arg in cls.base_configurables + cls.configurables + cls.input_argument_list():
            task_arg.add_argument(parser)


def macro(task_class):
    """Instance to run as the main entrypoint of a program or/and scripting. Call this function with to make a script
    macro.

    Args:
        task_class: the class, derived from PlottingTask, which will be used to run this script.
    """
    parser_main = argparse.ArgumentParser(description=task_class.parser_description)
    task_class.add_parser_options(parser_main)
    task_class(**vars(parser_main.parse_args())).run()
