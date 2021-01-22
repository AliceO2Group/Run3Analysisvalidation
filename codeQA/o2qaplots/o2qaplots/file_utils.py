import pathlib


def check_file_exists(file):
    """Checks if file exists.

    Raises:
        FileNotFoundError is file does not exist.
    """
    file_path = pathlib.Path(file)
    if not file_path.is_file():
        raise FileNotFoundError("It was not possible find the file: " + file)


def discover_root_objects(file, type_check):
    """Discovers the histograms saved in a file with multiple TDirectories.

    Args:
        file: the file to be inspected.
        type_check: a function to be called in the object to determine if they should
            be selected.

    Returns
        histograms: a list with HistogramInfo for each histogram.
    """
    histograms = list()

    _find_objects_in_path(None, histograms, file, type_check)

    return histograms


def _find_objects_in_path(path, results, file, type_check):

    if path is None:
        key_list = file.GetListOfKeys()
    else:
        try:
            key_list = file.Get(path).GetListOfKeys()
        except AttributeError:
            return

    for key in key_list:
        key_path = f"{path}/{key.GetName()}"

        if path is None:
            key_path = key.GetName()

        if type_check(key.GetClassName()):
            results.append(key_path)
        else:
            _find_objects_in_path(key_path, results, file, type_check)
