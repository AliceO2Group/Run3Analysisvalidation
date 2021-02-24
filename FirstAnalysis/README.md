# O2 First analysis framework

## Table of contents

* [Introduction](#introduction)
* [Overview](#overview)
  * [Execution](#execution)

## Introduction

The main purpose of this folder is to provide analysis tools to perform analysis of O2 analysis task outputs.
It includes for the moment a set of QA plotting macros to inspect AnalysisResults.root of MC and data samples. 
It also includes some preliminary functionalities for performing efficiency corrections of HF analyses. It is currently being used
also for performing Run5 analysis. 

## Available datasets
* The steering script `runtest.sh` provides control parameters and interface to the machinery for task execution.
* User provides configuration bash scripts which:
  * modify control parameters,
