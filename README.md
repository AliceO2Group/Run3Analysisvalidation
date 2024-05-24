# Run 3 validation framework

[![GitHub MegaLinter](https://github.com/AliceO2Group/Run3Analysisvalidation/workflows/MegaLinter/badge.svg?branch=master)](https://github.com/marketplace/actions/megalinter)

## Introduction

The Run 3 validation framework is a tool for an easy execution, testing and validation of the Run 3 analysis code on large local samples.

Its features include

* simple specification of input datasets,
* simple configuration and activation of analysis tasks,
* easy generation of the O<sup>2</sup> command for complex workflow topologies,
* job parallelisation,
* output merging,
* error checking and reporting,
* specification of postprocessing.

It also provides tools for:

* post mortem debugging of failing jobs,
* comparison of ROOT file content,
* visualisation of workflow dependencies,
* maintenance of Git repositories and installations of aliBuild packages.

The original purpose of the Run 3 validation framework was to provide a compact and flexible tool for validation of the
[O<sup>2</sup>(Physics)](https://github.com/AliceO2Group/O2Physics) analysis framework by comparison of its output to its
[AliPhysics](https://github.com/alisw/AliPhysics) counterpart.
The general idea is to run the same analysis using AliPhysics and O<sup>2</sup>(Physics) and produce comparison plots.

However, it can be used without AliPhysics as well to run O<sup>2</sup> analyses locally, similar to running trains on AliHyperloop.
This makes it a convenient framework for local development, testing and debugging of O<sup>2</sup>(Physics) code.

## Overview

The validation framework is a general configurable platform that gives user the full control over what is done.
Its flexibility is enabled by strict separation of its specialised components into a system of Bash scripts.
Configuration is separate from execution code, input configuration is separate from task configuration, execution steps are separate from the main steering code.

* The steering script [`runtest.sh`](exec/runtest.sh) provides control parameters and interface to the machinery for task execution.
* User provides configuration Bash scripts which:
  * modify control parameters,
  * produce modified configuration files,
  * generate step scripts executed by the framework in the validation steps.

### Execution

Execution code can be found in the [`exec`](exec) directory.

**The user should not touch anything in this directory!**

The steering script [`runtest.sh`](exec/runtest.sh) performs the following execution steps:

* Load input specification.
* Load tasks configuration.
* Print out input description.
* Clean before running. (activated by `DOCLEAN=1`)
  * Deletes specified files (produced by previous runs).
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
  * In case you want to use `AO2D.root` files as input directly, you can set `INPUT_IS_O2=1` in your input specification
    and use it in your configuration to deactivate incompatible steps (typically the conversion and AliPhysics tasks).
* Run output postprocessing. (activated by `DOPOSTPROCESS=1`)
  * Executes the postprocessing step script.
  * This step typically compares AliPhysics and O<sup>2</sup> output and produces plots.
* Clean after running. (activated by `DOCLEAN=1`)
  * Deletes specified (temporary) files.
* Done
  * This step is just a visual confirmation that all steps have finished without errors.

All steps are activated by default and some can be disabled individually by setting the respective activation variables to `0` in user's task configuration.

### Configuration

The steering script [`runtest.sh`](exec/runtest.sh) can be executed with the following optional arguments:

```bash
bash [<path>/]runtest.sh [-h] [-i <input-configuration>] [-t <task-configuration>] [-d]
```

`<input-configuration>` Input specification script. See [Input specification](#input-specification).

* Defaults to `config_input.sh` (in the current directory).

`<task-configuration>` Task configuration script. See [Task configuration](#task-configuration).

* Defaults to `config_tasks.sh` (in the current directory).

`-d` Debug mode. Prints out more information about settings and execution.

`-h` Help. Prints out the usage specification above.

#### Input specification

The input specification script is a Bash script that sets input parameters used by the steering script.

**This script defines which data will be processed and how.**

These are the available input parameters and their default values:

* `INPUT_LABEL="nothing"`           Input description
* `INPUT_DIR="$PWD"`                Input directory
* `INPUT_FILES="AliESDs.root"`      Input file pattern
* `INPUT_SYS="pp"`                  Collision system (`"pp"`, `"PbPb"`)
* `INPUT_RUN=2`                     LHC Run (2, 3, 5)
* `INPUT_IS_O2=0`                   Input files are in O<sup>2</sup> format.
* `INPUT_IS_MC=0`                   Input files are MC data.
* `INPUT_PARENT_MASK=""`            Path replacement mask for the input directory of parent files in case of linked derived O<sup>2</sup> input. Set to `";"` if no replacement needed.
* `JSON="dpl-config.json"`          O<sup>2</sup> device configuration

This allows you to define several input datasets and switch between them easily by setting the corresponding value of `INPUT_CASE`.

Other available parameters allow you to specify how many input files to process and how to parallelise the job execution.

#### Task configuration

The task configuration script is a Bash script that modifies the task parameters used by the steering script.

**This script defines which validation steps will run and what they will do.**

* It cleans the directory, deactivates incompatible steps, modifies the JSON file, generates step scripts.
* The body of the script has to provide these mandatory functions:
  * `Clean`                  Performs cleanup before and after running.
  * `AdjustJson`             Modifies the JSON file (e.g. selection cut activation).
  * `MakeScriptAli`          Generates the AliPhysics step script `script_ali.sh`.
  * `MakeScriptO2`           Generates the O<sup>2</sup> step script `script_o2.sh`.
  * `MakeScriptPostprocess`  Generates the postprocessing step script `script_postprocess.sh` (e.g. plotting).
* The `Clean` function takes one argument: `$1=1` for cleaning before running, `$1=2` for cleaning after running.
* The AliPhysics and O<sup>2</sup> step scripts take two arguments: `$1="<input-file>"`, `$2="<JSON-file>"`.
* The postprocessing step script takes two arguments: `$1="<O2-output-file>"`, `$2="<AliPhysics-output-file>"`.

Configuration that should be defined in the task configuration includes:

* Deactivation of the validation steps (`DOCLEAN`, `DOCONVERT`, `DOALI`, `DOO2`, `DOPOSTPROCESS`)
* Customisation of the commands loading the AliPhysics, O2Physics and postprocessing environments (`ENV_ALI`, `ENV_O2`, `ENV_POST`). By default the latest builds of AliPhysics, O2Physics and ROOT are used, respectively.
* Any other parameters related to "what should run and how", e.g. `SAVETREES`, `MAKE_GRAPH`, `USEALIEVCUTS`

#### Workflow specification

The full O<sup>2</sup> command, executed in the O<sup>2</sup> step script to run the activated O<sup>2</sup> workflows, is generated in the `MakeScriptO2` function using a dedicated Python script [`make_command_o2.py`](exec/make_command_o2.py).
This script generates the command using a **YAML database (`workflows.yml`) that specifies workflow options and how workflows depend on each other**.
You can consider a workflow specification in this database to be the equivalent of a wagon definition on AliHyperloop, including the definition of the wagon name, the workflow name, the dependencies and the derived data. The main difference is that the device configuration is stored in the JSON file.

The workflow database has two sections: `options` and `workflows`.
The `options` section defines `global` options, used once at the end of the command, and `local` options, used for every workflow.
The `workflows` section contains the "wagon" definitions.
The available parameters are:

* `executable` Workflow command, if different from the "wagon" name
  * This allows you to define multiple wagons for the same workflow.
* `dependencies` **Direct** dependencies (i.e. other wagons **directly** needed to run this wagon)
  * Allowed formats: string, list of strings
  * Direct dependencies are wagons that produce tables consumed by this wagon. You can figure them out using the [`find_dependencies.py`](https://github.com/AliceO2Group/O2Physics/blob/master/Scripts/find_dependencies.py) script in O2Physics.
* `requires_mc` Boolean parameter to specify whether the workflow can only run on MC
* `options` Command line options. (Currently not supported on AliHyperloop.)
  * Allowed formats: string, list of strings, dictionary with keys `default`, `real`, `mc`
* `tables` Descriptions of output tables to be saved as trees
  * Allowed formats: string, list of strings, dictionary with keys `default`, `real`, `mc`

The `make_command_o2.py` script allows you to generate a topology graph to visualise the dependencies defined in the database, using [Graphviz](https://graphviz.org/).
Generation of the topology graph can be conveniently enabled with `MAKE_GRAPH=1` in the task configuration.

Dummy examples of the configuration files can be found in:

* [`config/config_input_dummy.sh`](config/config_input_dummy.sh),
* [`config/config_tasks_dummy.sh`](config/config_tasks_dummy.sh),
* [`config/workflows_dummy.yml`](config/workflows_dummy.yml).

## Preparation

### Build AliPhysics and O<sup>2</sup>(Physics)

Follow the official [AliPhysics installation](https://alice-doc.github.io/alice-analysis-tutorial/building/) and
[O<sup>2</sup>(Physics) installation](https://aliceo2group.github.io/analysis-framework/docs/gettingstarted/installing.html) instructions.

Make sure the AliPhysics and O<sup>2</sup>Physics environments can be entered using the following respective commands.

```bash
alienv enter AliPhysics/latest
alienv enter O2Physics/latest
```

### Download the validation framework

```bash
git clone --origin upstream https://github.com/AliceO2Group/Run3Analysisvalidation.git
cd Run3Analysisvalidation
```

#### For developers: Fork the repository

* Create your fork repository on GitHub.
* Add it as your remote:

```bash
git remote add origin git@github.com:<your-github-username>/Run3Analysisvalidation.git
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

**Make sure that your Bash environment is clean!
Do not load ROOT, AliPhysics, O<sup>2</sup>, O<sup>2</sup>Physics or any other aliBuild package environment before running the framework!**

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

## Add a new workflow

To add a new workflow in the framework configuration, you need to the following steps.

* Add the workflow in the [task configuration](#task-configuration):
  * Add the activation switch: `DOO2_...=0         # name of the workflow (without o2-analysis)`.
  * Add the application of the switch in the `MakeScriptO2` function: `[ $DOO2_... -eq 1 ] && WORKFLOWS+=" o2-analysis-..."`.
  * If needed, add lines in the `AdjustJson` function to modify the JSON configuration.
* Add the [workflow specification](#workflow-specification) in the workflow database:
  * See the dummy example `o2-analysis-workflow` for the full list of options.
* Add the device configuration in the default JSON file.

## Job debugging

If you run many parallelised jobs and some of them don't finish successfully, you can make use of the debugging script [`debug.sh`](exec/debug.sh) in the [`exec`](exec) directory
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

Enter the [`codeHF`](codeHF) directory and see the [`README`](codeHF/README.md).

## Keep your repositories and installations up to date and clean

With the ongoing fast development, it can easily happen that updating the O<sup>2</sup>Physics part of the validation
also requires updating the O<sup>2</sup> and the AliPhysics installations which then requires updating the alidist recipes as well.
Also when requesting changes in the main repository via a pull request, it is strongly recommended to update one's personal fork repository first,
apply the changes on top the main branch and rebuild the installation to make sure that the new commits can be seamlessly merged into the main repository.

All these maintenance steps can be fully automated using the [`update_packages.py`](exec/update_packages.py) Python script which takes care of keeping your (local and remote) repositories
and installations up to date with the latest development in the respective main branches.
This includes updating alidist, AliPhysics, O<sup>2</sup>(Physics), and this Run 3 validation code repository,
as well as re-building your AliPhysics and O<sup>2</sup>(Physics) installations via aliBuild and deleting obsolete builds.

You can execute the script from any directory on your system using the following syntax:

```bash
python <path to the Run3Analysisvalidation directory>/exec/update_packages.py [-h] [-d] [-l] [-c] database
```

optional arguments:

  `-h`, `--help`   show the help message and exit

  `-d`, `--debug`  print debugging info

  `-l`           print latest commits and exit

  `-c`           print configuration and exit

The positional argument `database` is a YAML database with configuration and options.
The Run3Analysisvalidation repository provides a read-to-use configuration file with a full list of options at [`config/packages.yml`](config/packages.yml).
All you need to do is to make sure that the settings in the database correspond to your local setup and
adjust the activation switches, if needed, to change the list of steps to be executed.
By default, all packages are activated for build and update.

If you are happy with the configuration, you can then start the script and it will take care of the full update of your code and installations for all the activated packages.

If your repository is currently on a feature branch (different from the main branch), both the main branch and your feature branch will be updated from the main branch in the main (upstream) remote repository.
The main and the current branch are each first updated from their respective counterparts in the fork remote repository and then from the upstream main branch.
The updated history of each branch is then force-pushed to the fork repository.
This allows for synchronisation across machines where commits pushed to the fork repository from another machine are first incorporated locally before pushing new commits.
All your personal changes (committed and uncommitted) are preserved via rebasing and stashing.
Check the description of the script behaviour inside the script itself for more details.

If `clean: 1`, obsolete builds are deleted from the `sw` directory at the end.
If `clean_purge: 1`, a deeper purging is done by deleting all builds that are not needed to run the latest AliPhysics and O<sup>2</sup>(Physics) builds.
WARNING: Do not enable the purging if you need to keep several builds of AliPhysics or O<sup>2</sup>(Physics) (e.g. for different branches or commits) or builds of other development packages not specified in your configuration!

If any error occurs during the script execution, the script will report the error and exit immediately.

You can easily extend the script to include any other local Git repository and any other aliBuild development package on your machine that you wish to be updated in the same way.

## Continuous integration tests

Validity and quality of the code in the repository are checked on GitHub by several tools (linters) that support many coding languages.
Linters run automatically for every push or pull request event.
**Please make sure that your code passes all the tests before making a pull request.**

It is possible to check your code locally (before even committing or pushing):

### Space checker

```bash
bash <path to the Run3Analysisvalidation directory>/exec/check_spaces.sh
```

### [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html)

```bash
clang-format -style=file -i <file>
```

### [MegaLinter](https://oxsecurity.github.io/megalinter/latest/mega-linter-runner/)

```bash
npx mega-linter-runner
```
