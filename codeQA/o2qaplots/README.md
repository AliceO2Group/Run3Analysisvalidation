# Plots for the O2QA

## Install the package in your system

One of the easiest way to work with qa plotting macros is to install them as a package.
You can install it by navigating into this folder installing it using pip (or pip3 depending on your system):

    pip install -e . 

In order to run the QA plotting, you can check the available commands by running

    o2qa --help

You can also get more information for each option, such as

    o2qa plot --help

## Run the scripts without installing

You can run the script directly. This will require you to run python in the correct directory
(the same one that has the setup.py and this README file). Go to the folder containing this file
and run:

    python o2qaplots/cli.py --help.

The same instructions as for `Install the package in your system` apply here to check the available commands.

## Run in a (docker) container

Having problems with python versions? Something just does not work? Addicted to docker?
Or does it sparks joy on you to see your analysis running in an isolated and reproducible setup?
You can use the `Dockerfiles` and `docker-compose` configuration to help you run the analysis.
