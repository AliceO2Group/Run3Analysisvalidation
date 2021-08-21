# Prepared HF tasks and tests

## HF decay reconstruction tasks
The prepared HF example allows you to run reconstruction of 2-prong secondary vertices and a simple analysis of D<sup>0</sup> mesons. The entire data processing procedure in O<sup>2</sup> involves several steps:
1. Pre-selection of secondary tracks
   * Performed by the [HFTrackIndexSkimsCreator](https://github.com/AliceO2Group/O2Physics/blob/master/Tasks/PWGHF/HFTrackIndexSkimsCreator.cxx) class
   in the `o2-analysis-hf-track-index-skims-creator` binary.
   * Includes track selection, candidate preselection, and secondary vertex reconstruction.
1. Reconstruction of 2-prong decay candidates
   * Performed by the [HFCandidateCreator2Prong](https://github.com/AliceO2Group/O2Physics/blob/master/Tasks/PWGHF/HFCandidateCreator2Prong.cxx) class
   in the `o2-analysis-hf-candidate-creator-2prong` binary.
   * Reconstructs the secondary vertices again and creates 2-prong decay candidates.
   * Performs MC matching of the reconstructed candidates and of the generated particles.
1. Final selection of candidates
   * Performed by the [HFD0CandidateSelector](https://github.com/AliceO2Group/O2Physics/blob/master/Tasks/PWGHF/HFD0CandidateSelector.cxx) class
   in the `o2-analysis-hf-d0-candidate-selector` binary.
   * Appplies cuts on the parameters of the decay topology and track PID cuts.
1. D<sup>0</sup> analysis task
   * Performed by the [TaskD0](https://github.com/AliceO2Group/O2Physics/blob/master/Tasks/PWGHF/taskD0.cxx) class
   in the `o2-analysis-hf-task-d0` binary.
   * Fills histograms with kinematic properties of selected candidates (and matched particles).

Check [this presentation](https://indico.cern.ch/event/932917/contributions/3920363/attachments/2065207/3465791/20200629_PWGHF_report.pdf) to learn more about the design of the underlying data processing scheme that includes skimming, decay reconstruction and analysis.

## Validation

See the `README` in the main repository directory to get familiar with the validation framework.

The default example in the HF validation code performs the D<sup>0</sup> reconstruction analysis using AliPhysics (Run 1+2) and O<sup>2</sup> (Run 3) and produces comparison plots for various decay parameter distributions.

The validation steps are defined in the task configuration script `config_tasks.sh`.

### Select your data

In order to run the analysis with AliPhysics, you need `AliESDs.root` files and specify their location by setting the variables `INPUT_DIR` and `INPUT_FILES` in your input specification script.
To process Monte Carlo data, you also need the corresponding `galice.root` and `Kinematics.root` files.

The default HF input specification script `config_input.sh` contains some predefined input cases with hard-coded paths.
By default `INPUT_CASE=2` is selected.
To run the code with the same input files locally on your machine, download them from `/alice/sim/2018/LHC18a4a2_cent/282099/001/` on the Grid.

### Run the example

Enter the `codeHF` directory and execute the steering script:

```bash
bash runtest.sh
```

Running all the steps with `INPUT_CASE=2` takes about 40 seconds in total.

The postprocessing step produces several plots `comparison_histos_(...).pdf`, `MC_(...).pdf`.

To confirm that the output of the default settings looks as expected, compare the produced plots with their reference counterparts `(...)_ref.pdf`.

The complete list of commit hashes used to produce the reference plots can be found in `versions_ref.txt`.
