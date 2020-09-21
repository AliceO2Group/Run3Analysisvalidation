# Prepared HF tasks and tests

## HF decay reconstruction task
The prepared HF example allows you to run reconstruction of 2-prong and 3-prong secondary vertices and a simple analysis of D<sup>0</sup> mesons. The entire data processing procedure in O<sup>2</sup> involves three steps:
1. Pre-selection of secondary tracks
   * Performed by the [HFTrackIndexSkimsCreator](https://github.com/AliceO2Group/AliceO2/blob/dev/Analysis/Tasks/PWGHF/HFTrackIndexSkimsCreator.cxx) class in the `o2-analysis-hf-track-index-skims-creator` binary.
   * Includes track selection and secondary vertex reconstruction.
1. Reconstruction of 2-prong decay candidates
   * Performed by the [HFCandidateCreator2Prong](https://github.com/AliceO2Group/AliceO2/blob/dev/Analysis/Tasks/PWGHF/HFCandidateCreator2Prong.cxx) class in the `o2-analysis-hf-candidate-creator-2prong` binary.
   * Reconstructs the secondary vertices again and creates 2-prong decay candidates.
1. D<sup>0</sup> analysis task
   * Performed by the [TaskD0](https://github.com/AliceO2Group/AliceO2/blob/dev/Analysis/Tasks/PWGHF/taskD0.cxx) class in the `o2-analysis-hf-task-d0` binary.

Check [this presentation](https://indico.cern.ch/event/932917/contributions/3920363/attachments/2065207/3465791/20200629_PWGHF_report.pdf) to learn more about the design of the underlying data processing scheme that includes skimming, decay reconstruction and analysis.

## Run 3 code validation

The results and the intermediate data processing steps can be cross-checked with their counterparts in AliPhysics.
To perform this Run 3 code validation, you need to run a dedicated code that is not part of the O<sup>2</sup> framework.
It runs the same analysis using AliPhysics and O<sup>2</sup> and produces comparison plots.

### Download the comparison software

You can download the validation code:
``` bash
git clone https://github.com/AliceO2Group/Run3Analysisvalidation.git
cd Run3Analysisvalidation/codeHF
```

### Select your data

In order to run the analysis with AliPhysics, you need `AliESDs.root` files and specify their location by setting the variables `INPUTDIR` and `STRING` in the `runtest.sh` steering script in the `codeHF` directory.

The steering script contains some hard-coded paths for different cases.
By default `CASE=4` is enabled.
To run the code with the same input file locally on your machine, download it from `/alice/sim/2018/LHC18a4a2_cent/282099/001/AliESDs.root` on the Grid.

### Run the example

Now you are ready to run the validation code.
It performs the D<sup>0</sup> reconstruction analysis using AliPhysics (Run 1+2) and O<sup>2</sup> (Run 3) and produces comparison plots for various decay parameter distributions.

Execute the steering script from the `codeHF` directory:

``` bash
bash runtest.sh
```

These steps are performed by default:

1. Convert input data from the Run 1+2 format into the Run 3 format.
   *  Specified input `AliESDs.root` files are converted into `AO2D.root` files.
   *  Enabled by `DOCONVERT=1`.
1. Run the analysis tasks with AliPhysics.
   *  Produces a `Vertices2prong-ITS1.root` file.
   *  Enabled by `DORUN1=1`.
1. Run the analysis tasks with O<sup>2</sup>.
   *  Produces an `AnalysisResults.root` file.
   *  Enabled by `DORUN3=1`.
   *  Parameters of individual tasks are picked up from the JSON configuration file `dpl-config_std.json`.
   *  List of input files is defined by the variable `O2INPUT` in `runtest.sh`. By default, it is the list of files produced by the conversion stage.
1. Compare AliPhysics and O<sup>2</sup> output.
   *  Produces comparison plots `comparison_histos.pdf`, `comparison_ratios.pdf`.
   *  Enabled by `DOCOMPARE=1`.

Individual steps can be disabled by setting the respective variables to `0`.
Running all the steps with `CASE=4` takes about 23 seconds in total.
If everything went fine, the script will exit with the message `Done`.
You should have got all the output files in the `codeHF` directory.
To confirm that the output of the default settings looks as expected, compare the produced plots with the reference `comparison_histos_ref.pdf`, `comparison_ratios_ref.pdf`.
If any step fails, the script will exit with the message `Error` and you should look into the respective log file to investigate the problem.
