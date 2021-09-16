# ALICE3 HF performance studies

## Table of contents

* [Introduction](#introduction)
* [Overview](#overview)
  * [InputTheory](#InputTheory)
  * [InputExp](#InputExp)
  * [HowTo](#HowTo)

## Introduction

The purpose of this repository is to collect the macros and utilities to study the performances of the ALICE3 detector with heavy-flavoured hadrons, including charmed and beauty hadrons, HF jets and quarkonia. For these studies, the ALICE3 detector setup is simulated using the [DelphesO2](https://github.com/AliceO2Group/DelphesO2) package, while the heavy-flavour reconstruction is performed using the [O<sup>2</sup>](https://github.com/AliceO2Group/AliceO2) analysis framework.

## Overview

The repository now contains the following subfolders:

### InputTheory
This folder contains p<sub>T</sub>-dependent predictions for charmed and multi-charm baryons, which are used to estimate the expected signal yields of each hadron. In particular:
  * predictions in csv format from the coalescence calculations based on arXiv.1907.12786 in PbPb collisions at 2.76 TeV. These inputs can be converted in ROOT histograms by using the following [script](https://github.com/AliceUpgrades/ALICE3_HFperformance/blob/main/analysis/read_predictions_ptdep_stat_cholee_2_pbpb2p76_absy0p5.py), as discussed later.
  * predictions for the Λ<sub>c</sub> baryons in pp collisions from Pythia 8.

### InputExp
This folder contains the experimental inputs for performing significance estimations for the different hadrons in both pp and AA collisions. 
The name of the folder indicates whether these studies were performed with aliphysics or with the O<sup>2</sup> simulation and analysis framework. Each folder typically contains:
  * a histogram with the background content within 3 sigma from the peak mean normalized by the number of events extracted in MB Monte Carlo simulations
  * a histogram with the efficiencies for each channel. 

### analysis
In this folder, we collect all the analysis macros for performing the final analyses and plots. In particular:
  * The [script](https://github.com/AliceUpgrades/ALICE3_HFperformance/blob/main/analysis/read_predictions_ptdep_stat_cholee_2_pbpb2p76_absy0p5.py) already introduced above to convert the cvs predictions from arXiv.1907.12786 to ROOT histograms (the ROOT files are not pushed into the repository, but one needs to generate them when needed). 
  * A [script](https://github.com/AliceUpgrades/ALICE3_HFperformance/blob/main/analysis/expected_yields.py) to perform the comparison of the expected yields of charmed and multi-charmed hadrons in AA collisions. Few remarks:
    * For the moment, we consider for each theoretical calculation the same p<sub>T</sub>-shape. We indeed consider the normalized p<sub>T</sub>-spectrum from arXiv.1907.12786 and we just multiply it for the expected dN/dy provided by each theory group (collected [here](https://github.com/AliceUpgrades/ALICE3_HFperformance/blob/main/analysis/databases/theory_yields.yaml)).
    * Make sure you run the script descrive in the bullet above in advance in order to generate the ROOT histograms needed for the comparison.  
  * A [script](https://github.com/AliceUpgrades/ALICE3_HFperformance/blob/main/analysis/significance.py) that performs the significance estimation as a function of p<sub>T</sub> (currently working only for Λ<sub>c</sub> baryons in pp collisions). The parameters needed are read from the following two databases, [significance.yaml](https://github.com/AliceUpgrades/ALICE3_HFperformance/blob/main/analysis/databases/significance.yaml) and [general.yaml](https://github.com/AliceUpgrades/ALICE3_HFperformance/blob/main/analysis/databases/general.yaml).

## HowTo

All the scripts can be run as simple python scripts from the folder "analysis" . E.g.:
```python
python read_predictions_ptdep_stat_cholee_2_pbpb2p76_absy0p5.py
```
```python
python expected_yields.py
```
```python
python significance.py
```

