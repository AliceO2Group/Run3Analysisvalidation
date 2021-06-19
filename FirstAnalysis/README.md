# O2 first analysis utilities

## Introduction
The main purpose of this folder is to provide analysis tools to perform analysis of O2 analysis task outputs.
It includes for the moment a set of QA plotting macros to inspect `AnalysisResults.root` of MC and data samples.
It also includes some preliminary functionalities for performing efficiency corrections of HF analyses.
It is currently being used also for performing Run5 analysis.

## Instructions for contributors

* Write documentation comments to make the code easy to understand.
* Every file should contain the following details at the top:
  * what the code does,
  * which input it needs (e.g. output of which O2 workflow),
  * how to run it (in case simply executing the script is not enough),
  * names of authors, contributors, maintainers (to contact in case of problems or questions).
* Follow [PEP 8 naming conventions](https://www.python.org/dev/peps/pep-0008/#naming-conventions) in Python code.
* Do not use hard-coded paths! A default path should point to a file directly produced by the framework (e.g. `../codeHF/AnalysisResults_O2.root`).

## Available samples ALICE3
* D2H KrKr production (X events):
  * AOD O2 tables: `/data/Run5data/EMBEDDING_KrKr_CCBARLc_scenario3/TEST`
  * Analysis result: `/data/Run5data_samples/EMBEDDING_KrKr_CCBARLc_scenario3_20210224/AnalysisResults_O2.root`

## Plotting
The analyses validation is supposed to use the [HFPlot tool](https://github.com/benedikt-voelkel/HFPlot) for plotting purposes in the future.
It is basically a *pythonic* wrapper around `ROOT` with some automated features.
As a first use case it is implemented in [distrib_studies.py](distrib_studies.py).
A more extensive README can be found [here](https://github.com/benedikt-voelkel/HFPlot) and there are also some examples provided.
