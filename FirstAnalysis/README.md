# Run 3 First analysis framework

## Table of contents

* [Introduction](#introduction)
* [Overview](#overview)
  * [Execution](#execution)

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
