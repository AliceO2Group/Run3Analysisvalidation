import json

import pytest

from o2qaplots.config import AxisConfig, PlotConfig


@pytest.fixture
def json_file():
    return """{
        "numberOfTracks": {
            "x_axis": {
                "view_range": [0, 200],
                "log": false
            },
            "y_axis": {
                "view_range": [0.0, 1.0],
                "log": true
            }
        },
        "pt": {
            "y_axis": {
                "log": true
            }
        },
        "ptResolutionVsPt": {
            "x_axis": {
                "log": true
            }
        }
    }
    """


@pytest.fixture
def dict_example(json_file):
    return json.loads(json_file)


def test_axis_config(dict_example):
    x_axis = AxisConfig(**dict_example["numberOfTracks"]["x_axis"])

    assert x_axis.log is False
    assert x_axis.view_range == [0, 200]


def test_plot_config(dict_example):
    n_tracks = PlotConfig(**dict_example["numberOfTracks"])

    assert n_tracks.x_axis.log is False
    assert n_tracks.x_axis.view_range == [0, 200]

    assert n_tracks.y_axis.log is True
    assert n_tracks.y_axis.view_range == [0., 1.]
