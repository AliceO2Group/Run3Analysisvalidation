# Prepared HF tasks and tests

## HF decay reconstruction tasks
The prepared HF example allows you to run reconstruction of 2-prong and 3-prong secondary vertices and a simple analysis of D<sup>0</sup> mesons. The entire data processing procedure in O<sup>2</sup> involves three steps:
1. Pre-selection of secondary tracks
   * Performed by the [HFTrackIndexSkimsCreator](https://github.com/AliceO2Group/AliceO2/blob/dev/Analysis/Tasks/PWGHF/HFTrackIndexSkimsCreator.cxx) class
   in the `o2-analysis-hf-track-index-skims-creator` binary.
   * Includes track selection, candidate preselection, and secondary vertex reconstruction.
1. Reconstruction of 2-prong and 3-prong decay candidates
   * Performed by the [HFCandidateCreator2Prong](https://github.com/AliceO2Group/AliceO2/blob/dev/Analysis/Tasks/PWGHF/HFCandidateCreator2Prong.cxx)
   and [HFCandidateCreator3Prong](https://github.com/AliceO2Group/AliceO2/blob/dev/Analysis/Tasks/PWGHF/HFCandidateCreator3Prong.cxx) classes in the
   `o2-analysis-hf-candidate-creator-2prong` and `o2-analysis-hf-candidate-creator-3prong` binaries.
   * Reconstructs the secondary vertices again and creates 2-prong and 3-prong decay candidates.
1. D<sup>0</sup> and D<sup>+</sup> analysis tasks
   * Performed by the [TaskD0](https://github.com/AliceO2Group/AliceO2/blob/dev/Analysis/Tasks/PWGHF/taskD0.cxx)
   and [TaskDPlus](https://github.com/AliceO2Group/AliceO2/blob/dev/Analysis/Tasks/PWGHF/taskDPlus.cxx) classes in the
   `o2-analysis-hf-task-d0` and `o2-analysis-hf-task-dplus` binaries.

Check [this presentation](https://indico.cern.ch/event/932917/contributions/3920363/attachments/2065207/3465791/20200629_PWGHF_report.pdf) to learn more about the design of the underlying data processing scheme that includes skimming, decay reconstruction and analysis.

## Validation

The HF validation code performs the D<sup>0</sup> and D<sup>+</sup> reconstruction analysis using AliPhysics (Run 1+2) and O<sup>2</sup> (Run 3) and produces comparison plots for various decay parameter distributions.

The validation steps are defined in the task configuration script `config_tasks.sh`.

### Select your data

In order to run the analysis with AliPhysics, you need `AliESDs.root` files and specify their location by setting the variables `INPUT_DIR` and `INPUT_FILES` in your input specification script.
To process Monte Carlo data, you also need the corresponding `galice.root` and `Kinematics.root` files.

The default HF input specification script `config_input.sh` contains some predefined input cases with hard-coded paths.
By default `INPUT_CASE=4` is selected.
To run the code with the same input files locally on your machine, download them from `/alice/sim/2018/LHC18a4a2_cent/282099/001/` on the Grid.

### Run the example

Enter the `codeHF` directory and execute the steering script:

```bash
bash runtest.sh
```

Running all the steps with `INPUT_CASE=4` takes about 40 seconds in total.

The postprocessing step produces comparison plots `comparison_histos_2prong.pdf`, `comparison_ratios_2prong.pdf`, `comparison_histos_3prong.pdf`, `comparison_ratios_3prong.pdf`.

To confirm that the output of the default settings looks as expected, compare the produced plots with their reference counterparts
`comparison_histos_2prong_ref.pdf`, `comparison_ratios_2prong_ref.pdf`, `comparison_histos_3prong_ref.pdf`, `comparison_ratios_3prong_ref.pdf`.
