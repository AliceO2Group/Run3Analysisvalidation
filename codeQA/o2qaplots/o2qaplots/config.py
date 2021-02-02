import json
import typing
from collections import defaultdict


class AxisConfig:
    """Configures an histogram axis representation.

    Attributes:
        view_range: the axes range that will be visible in the plot
        log: whether the axis should be shown in a log scale.

    Args:
        view_range: the axes range that will be visible in the plot
        log: whether the axis should be shown in a log scale.
    """

    def __init__(self, view_range: typing.List[float] = None, log: bool = False):
        self.view_range = view_range
        self.log = log

    def __repr__(self):
        return (
            f"<{self.__class__.__name__} view_range={self.view_range} log={self.log}>"
        )


class PlotConfig:
    """Configures an individual plot.

    Attributes:
        x_axis: AxisConfig with the configuration for the x axis
        y_axis: AxisConfig with the configuration for the y axis

    Args:
        x_axis: an AxisConfig with for the axis, or a dict to be passed to the
            constructor of AxisConfig
        y_axis:  an AxisConfig with for the axis, or a dict to be passed to the
            constructor of AxisConfig

    """

    def __init__(self, x_axis=AxisConfig(), y_axis=AxisConfig()):
        if not isinstance(x_axis, AxisConfig):
            x_axis = AxisConfig(**x_axis)

        if not isinstance(y_axis, AxisConfig):
            y_axis = AxisConfig(**y_axis)

        self.x_axis = x_axis
        self.y_axis = y_axis

    def __repr__(self):
        return (
            f"<{self.__class__.__name__} "
            f"x_axis={repr(self.x_axis)} y_axis={repr(self.y_axis)}>"
        )


class JsonConfig(defaultdict):
    """ "Class used to read and store the JSON configuration files.
    It reads the configuration from the JSON file and it stores it in as a dictionary.
    The values of the dictionaryare automatically converted into an PlotConfig.

    In case the configuration is not set for a particular object,
    this configuration will return a PlotConfig with its default value.
    """

    def __init__(self, json_file_name=None):
        if json_file_name is not None:
            with open(json_file_name) as json_file:
                values = json.load(json_file)
                super().__init__(
                    lambda: PlotConfig(),
                    {k: PlotConfig(**v) for k, v in values.items()},
                )
        else:
            super(JsonConfig, self).__init__()
