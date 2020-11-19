# Run 3 validation framework

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
     and use it in your configuration to deactivate incompatible steps (typically the conversion, AliPhysics tasks, and comparison).
 * Run output postprocessing. (activated by `DOPOSTPROCESS=1`)
   * Executes the postprocessing step script.
   * This step typically compares AliPhysics and O<sup>2</sup> output and produces plots.
 * Clean after running. (activated by `DOCLEAN=1`)
   * Deletes specified files.
 * Done
   * This step is just a visual confirmation that all steps have finished without errors.

All steps are activated by default and some can be disabled indiviually by setting the respective activation variables to `0` in user's task configuration.

### Configuration

The steering script `runtest.sh` can be executed with the following optional arguments:

```bash
bash [<path>/]runtest.sh [-h] [-i <input config>] [-t <task config>]
```

`h` Prints out the usage specification above.

`<input config>` Input specification
 * Bash script that modifies input parameters.
 * This script defines which data will be processed.
 * Defaults to `config_input.sh` (in the current directory).

`<task config>` Task configuration
 * Bash script that cleans the directory, deactivates step, modifies JSON, generates step scripts.
 * This script defines what the validation steps will do.
 * Defaults to `config_tasks.sh` (in the current directory).
 * Provides these mandatory functions:
   * `Clean` Performs cleanup before and after running.
   * `AdjustJson`             Modifies the JSON file. (e.g. selection cut activation)
   * `MakeScriptAli`          Generates the AliPhysics step script.
   * `MakeScriptO2`           Generates the O<sup>2</sup> step script.
   * `MakeScriptPostprocess`  Generates the postprocessing step script. (e.g plotting)
 * The `Clean` function takes one argument: `$1=1` before running, `$1=2` after running.
 * The AliPhysics and O<sup>2</sup> step scripts take two arguments: `$1="<input file>"`, `$2="<JSON file>"`.
 * The postprocessing step script takes two arguments: `$1="<O2 output file>"`, `$2="<AliPhysics output file>"`.

Implementation of these configuration scripts is fully up to the user.

Dummy examples can be found in: `config/config_input_dummy.sh`, `config/config_tasks_dummy.sh`.

## Preparation

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

Enter any directory and execute the steering script `runtest.sh`.
(You can create a symlink for convenience.)
All the processing will take place in the current directory.

JSON file is expected in the current directory unless specified otherwise.

Variable `DIR_TASKS` stores the path to the task configuration script directory.
It can be used inside that script to refer to other files in the same directory, (e.g. cleaning script, ROOT macros).

Set `DEBUG=1` to see more details in the terminal.

If everything went fine, the script will exit with the message `Done` and you should have got all the output files in the current directory.

If any step fails, the script will display an error message and you should look into the respective log file to investigate the problem.

If the main log file of a validation step mentions "parallel: This job failed:", inspect the respective log file in the directory of the corresponding job.

# Heavy-flavour analyses

Enter the `codeHF` directory and see the `README`.

# Keep your repositories and installations up to date

With the ongoing fast development, it can easily happen that updating the O<sup>2</sup> part of the validation
also requires updating the AliPhysics installation which then requires updating the alidist recipes as well.
Also when requesting changes in the main repository via a pull request, it is strongly recommended to update one's personal fork repository first,
apply the changes on top the main branch and rebuild the installation to make sure that the new commits can be seamlessly merged into the main repository.

All these maintenance steps can be fully automated using the `update_packages.sh` bash script which takes care of keeping your (local and remote) repositories
and installations up to date with the latest development in the respective main branches.
This includes updating alidist, AliPhysics, O<sup>2</sup>, and this Run 3 validation code repository, as well as re-building your AliPhysics and O<sup>2</sup> installations via aliBuild and deleting obsolete builds.

All you need to do is to make sure that the strings in the `User settings` block of the script correspond to your local setup and
adjust the activation switches, if needed, to change the list of steps to be executed.

You can then start the full update of your code and installations by running the script from any directory on your system:
```bash
bash <path to the Run3Analysisvalidation directory>/exec/update_packages.sh
```

If your repository is currently on a feature branch (different from the main branch), both the main branch and your feature branch will be updated from the main branch.
All your personal changes (committed and uncommitted) are preserved via rebasing and stashing.
Check the description of the script behaviour inside the script itself for more details.

If `CLEAN=1`, obsolete builds are deleted from the `sw` directory at the end.
If `PURGE_BUILDS=1`, a deeper purging is done by deleting all builds that are not needed to run the latest AliPhysics and O2 builds.
WARNING: Do not enable the purging if you need to keep several builds of AliPhysics or O2 (e.g. for different branches or commits) or builds of other development packages!

You can easily extend the script to include any other local Git repository on your machine that you wish to be updated in the same way.
