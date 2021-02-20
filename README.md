# Run 3 validation framework

[![GitHub Super-Linter](https://github.com/AliceO2Group/Run3Analysisvalidation/workflows/Super-Linter/badge.svg)](https://github.com/marketplace/actions/super-linter)
[![GitHub Clang Format Linter](https://github.com/AliceO2Group/Run3Analysisvalidation/workflows/Clang%20Format%20Linter/badge.svg)](https://github.com/marketplace/actions/clang-format-lint)

## Table of contents

* [Introduction](#introduction)
* [Overview](#overview)
  * [Execution](#execution)
  * [Configuration](#configuration)
* [Preparation](#preparation)
  * [Build AliPhysics and O<sup>2</sup>](#build-aliphysics-and-o-sup-2--sup-)
  * [Download the comparison software](#download-the-comparison-software)
  * [Install parallelisation software](#install-parallelisation-software)
* [Run the framework](#run-the-framework)
* [Job debugging](#job-debugging)
* [Heavy-flavour analyses](#heavy-flavour-analyses)
* [Keep your repositories and installations up to date](#keep-your-repositories-and-installations-up-to-date)
* [Continuous integration tests](#continuous-integration-tests)
  * [C++](#c)
  * [Python](#python)

## Introduction

The main purpose of the Run 3 validation framework is to provide a compact and flexible tool for validation of the
[O<sup>2</sup>](https://github.com/AliceO2Group/AliceO2) analysis framework by comparison of its output to its
[AliPhysics](https://github.com/alisw/AliPhysics) counterpart.
The general idea is to run the same analysis using AliPhysics and O<sup>2</sup> and produce comparison plots.

## Overview

The validation framework is a general configurable platform that gives user the full control over what is done.
Its flexibility is enabled by strict separation of its specialised components into a system of bash scripts.
Configuration is separate from execution code, input configuration is separate from task configuration, execution steps are separate from the main steering code.

* The steering script `runtest.sh` provides control parameters and interface to the machinery for task execution.
* User provides configuration bash scripts which:
  * modify control parameters,
  * produce modified configuration files,
  * generate step scripts executed by the framework in the validation steps.

### Execution

Execution code can be found in the `exec` directory.

The steering script `runtest.sh` performs the following execution steps:
* Load input specification.
* Load tasks configuration.
* Print out input description.
* Clean before running. (activated by `DOCLEAN=1`)
  * Deletes specified files.
* Generate list of input files.
* Modify the JSON file.
* Convert `AliESDs.root` to `AO2D.root`. (activated by `DOCONVERT=1`)
  * Executes the AliPhysics conversion macro in parallel jobs.
  * Specified input `AliESDs.root` files are converted into `AO2D.root` files in the `output_conversion` directory.
* Run AliPhysics tasks. (activated by `DOALI=1`)
  * Executes the AliPhysics step script in parallel jobs.
  * Produces the `AnalysisResults_ALI.root` file, resulting from merging output files in the `output_ali` directory.
* Run O<sup>2</sup> tasks. (activated by `DOO2=1`)
  * Executes the O<sup>2</sup> step script in parallel jobs.
  * Produces the `AnalysisResults_O2.root` file, resulting from merging output files in the `output_o2` directory.
  * If `SAVETREES=1`, tables are saved as trees in the `AnalysisResults_trees_O2.root` file.
  * Parameters of individual tasks are picked up from the JSON configuration file (`dpl-config.json` by default).
  * By default, the list of input files includes files produced by the conversion step.
  * In case you want to use `AO2D.root` files as input directly, you can set `ISINPUTO2=1` in your input specification
    and use it in your configuration to deactivate incompatible steps (typically the conversion and AliPhysics tasks).
* Run output postprocessing. (activated by `DOPOSTPROCESS=1`)
  * Executes the postprocessing step script.
  * This step typically compares AliPhysics and O<sup>2</sup> output and produces plots.
* Clean after running. (activated by `DOCLEAN=1`)
  * Deletes specified files.
* Done
  * This step is just a visual confirmation that all steps have finished without errors.

All steps are activated by default and some can be disabled individually by setting the respective activation variables to `0` in user's task configuration.

### Configuration

The steering script `runtest.sh` can be executed with the following optional arguments:

```bash
bash [<path>/]runtest.sh [-h] [-i <input config>] [-t <task config>] [-d]
```

`-h` Prints out the usage specification above.

`-d` (Debug mode) Prints out more information about settings and execution.

`<input config>` Input specification
* Bash script that modifies input parameters.
* This script defines which data will be processed.
* Defaults to `config_input.sh` (in the current directory).

`<task config>` Task configuration
* Bash script that cleans the directory, deactivates steps, modifies the JSON file, generates step scripts.
* This script defines what the validation steps will do.
* Defaults to `config_tasks.sh` (in the current directory).
* Provides these mandatory functions:
  * `Clean` Performs cleanup before and after running.
  * `AdjustJson`             Modifies the JSON file. (e.g. selection cut activation)
  * `MakeScriptAli`          Generates the AliPhysics step script.
  * `MakeScriptO2`           Generates the O<sup>2</sup> step script.
  * `MakeScriptPostprocess`  Generates the postprocessing step script. (e.g. plotting)
* The `Clean` function takes one argument: `$1=1` before running, `$1=2` after running.
* The AliPhysics and O<sup>2</sup> step scripts take two arguments: `$1="<input file>"`, `$2="<JSON file>"`.
* The postprocessing step script takes two arguments: `$1="<O2 output file>"`, `$2="<AliPhysics output file>"`.

Implementation of these configuration scripts is fully up to the user.

Dummy examples can be found in: `config/config_input_dummy.sh`, `config/config_tasks_dummy.sh`.

## Preparation

### Build AliPhysics and O<sup>2</sup>

Follow [the official installation instructions](https://alice-doc.github.io/alice-analysis-tutorial/building/) to build
AliPhysics and O<sup>2</sup> on your machine.

Make sure the AliPhysics and O<sup>2</sup> environments can be entered using the following respective commands.

```bash
alienv enter AliPhysics/latest
alienv enter O2/latest
```

### Download the comparison software

```bash
git clone https://github.com/AliceO2Group/Run3Analysisvalidation.git
cd Run3Analysisvalidation
```

### Install parallelisation software

The execution of validation steps is parallelised using the [GNU Parallel](https://www.gnu.org/software/parallel/) tool.
You need to have it installed on your machine to run the code in parallel jobs.
You can install GNU Parallel on Debian/Ubuntu-based systems with:

```bash
sudo apt install parallel
```

## Run the framework

Now you are ready to run the validation code.

**Make sure that your bash environment is clean!
Do not load ROOT, AliPhysics, O<sup>2</sup> or any other aliBuild package environment before running the framework!**

Enter any directory and execute the steering script `runtest.sh`.
(You can create a symlink for convenience.)
All the processing will take place in the current directory.

JSON file is expected in the current directory unless specified otherwise.

Variable `DIR_TASKS` stores the path to the task configuration script directory.
It can be used inside that script to refer to other files in the same directory, (e.g. cleaning script, ROOT macros).

Use the debug command line option `-d` to see more details in the terminal.

If everything went fine, the script will exit with the message `Done` and you should have got all the output files in the current directory.

If any step fails, the script will display an error message and you should look into the respective log file to investigate the problem.

If the main log file of a validation step mentions "parallel: This job failed:", inspect the respective log file in the directory of the corresponding job.

## Job debugging

If you run many parallelised jobs and some of them don't finish successfully, you can make use of the debugging script `debug.sh` in the `exec` directory
which can help you figure out what went wrong, where and why.

You can execute the script from the current working directory using the following syntax (options can be combined):
```bash
bash [<path>/]debug.sh [-h] [-t TYPE] [-b [-u]] [-f] [-w] [-e]
```

`-h` Print out the usage help.

`TYPE` Job type: `conversion`, `ali`, `o2` (`o2` by default)

`-b`  Show bad jobs (without output file or successful end).

`-u`  Mark unfinished jobs (running, hanging, aborted). (Requires `-b`.)

`-f`  Show input files of bad jobs.

`-w`  Show warnings (for all jobs).

`-e`  Show errors (for all jobs).

## Heavy-flavour analyses

Enter the `codeHF` directory and see the `README`.

## Keep your repositories and installations up to date

With the ongoing fast development, it can easily happen that updating the O<sup>2</sup> part of the validation
also requires updating the AliPhysics installation which then requires updating the alidist recipes as well.
Also when requesting changes in the main repository via a pull request, it is strongly recommended to update one's personal fork repository first,
apply the changes on top the main branch and rebuild the installation to make sure that the new commits can be seamlessly merged into the main repository.

All these maintenance steps can be fully automated using the `update_packages.sh` bash script which takes care of keeping your (local and remote) repositories
and installations up to date with the latest development in the respective main branches.
This includes updating alidist, AliPhysics, O<sup>2</sup>, and this Run 3 validation code repository, as well as re-building your AliPhysics and O<sup>2</sup> installations via aliBuild and deleting obsolete builds.

All you need to do is to make sure that the strings in the `config/config_update.sh` script correspond to your local setup and
adjust the activation switches, if needed, to change the list of steps to be executed.
By default, O<sup>2</sup> and Run3Analysisvalidation are activated for build and update.
Settings for alidist and AliPhysics are also included but with deactivated build and update.

You can execute the script from any directory on your system using the following syntax:
```bash
bash <path to the Run3Analysisvalidation directory>/exec/update_packages.sh [-h] [-c <config>] [-d]
```

`-h` (Help) Prints out the usage specification above.

`<config>` Configuration script including the package specification
* Defaults to `config/config_update.sh` (in the Run3Analysisvalidation repository).

`-d` (Dry run) Displays configuration without doing anything.

If you are happy with the configuration, you can then start the script and it will take care of the full update of your code and installations for all the activated packages.

If your repository is currently on a feature branch (different from the main branch), both the main branch and your feature branch will be updated from the main branch in the main (upstream) remote repository.
The main and the current branch are each first updated from their respective counterparts in the fork remote repository and then from the upstream main branch.
The updated history of each branch is then force-pushed to the fork repository.
This allows for synchronisation across machines where commits pushed to the fork repository from another machine are first incorporated locally before pushing new commits.
All your personal changes (committed and uncommitted) are preserved via rebasing and stashing.
Check the description of the script behaviour inside the script itself for more details.

If `CLEAN=1`, obsolete builds are deleted from the `sw` directory at the end.
If `PURGE_BUILDS=1`, a deeper purging is done by deleting all builds that are not needed to run the latest AliPhysics and O<sup>2</sup> builds.
WARNING: Do not enable the purging if you need to keep several builds of AliPhysics or O<sup>2</sup> (e.g. for different branches or commits) or builds of other development packages not specified in your configuration!

If any error occurs during the script execution, the script will report the error and exit immediately.

You can easily extend the script to include any other local Git repository and any other aliBuild development package on your machine that you wish to be updated in the same way.

## Continuous integration tests

Validity and quality of the code in the repository are checked on Github by several tools (linters) that support many coding languages.
Linters run automatically for every push or pull request.
**Please make sure that your code passes all the tests before making a pull request.**

Here are some tips how to check your code locally with the linters that usually complain the most.

### C++

* [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) - automatic reformatting of C++ sources files according to configurable style guides

```bash
clang-format -style=file -i <file>
```

### Python

* [Black](https://github.com/psf/black) - “uncompromising Python code formatter”
* [flake8](https://gitlab.com/pycqa/flake8) - “tool that glues together pep8, pyflakes, mccabe, and third-party plugins to check the style and quality of some python code”
* [isort](https://github.com/PyCQA/isort) - “utility to sort imports alphabetically, and automatically separated into sections and by type”

```bash
black . && flake8 . && isort .
```
