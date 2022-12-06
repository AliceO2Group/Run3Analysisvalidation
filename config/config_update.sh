#!/bin/bash
# shellcheck disable=SC2034 # Ignore unused parameters.

# Configuration of update_packages.sh

####################################################################################################

# Settings

# Delete unnecessary files.
CLEAN=1
CLEAN_AGGRESSIVE=0

# Delete all builds that are not needed to run the latest builds of specified development packages.
# WARNING: This feature requires all development packages to be specified and added in the list LIST_PKG_DEV_SPECS! Builds of missing packages will be deleted!
# WARNING: This feature executes "aliBuild build" for all development packages. If a package needs to be rebuilt, it will be rebuilt!
# WARNING: Do not enable this if you need to keep several builds per development package (e.g. for different branches or commits)!
PURGE_BUILDS=0

# Print out an overview of the latest commits of repositories.
PRINT_COMMITS=1

# Main ALICE software directory
ALICE_DIR="$HOME/alice"

# aliBuild
#ALIBUILD_ARCH=$(aliBuild architecture) # system architecture
#ALIBUILD_OPT="-a $ALIBUILD_ARCH"

####################################################################################################

# Package specification:
# paths, names of remotes (check git remote -v), build options.

# alidist
ALIDIST_NAME="alidist"
ALIDIST_UPDATE=0
ALIDIST_DIR="$ALICE_DIR/alidist"
ALIDIST_REMOTE_MAIN="upstream"
ALIDIST_REMOTE_FORK=""
ALIDIST_BRANCH_MAIN="master"
ALIDIST_SPECS=("$ALIDIST_NAME" "$ALIDIST_UPDATE" "$ALIDIST_DIR" "$ALIDIST_REMOTE_MAIN" "$ALIDIST_REMOTE_FORK" "$ALIDIST_BRANCH_MAIN")

# AliPhysics
ALIPHYSICS_NAME="AliPhysics"
ALIPHYSICS_UPDATE=0
ALIPHYSICS_DIR="$ALICE_DIR/AliPhysics"
ALIPHYSICS_REMOTE_MAIN="upstream"
ALIPHYSICS_REMOTE_FORK="origin"
ALIPHYSICS_BRANCH_MAIN="master"
ALIPHYSICS_BUILD_OPT="--defaults o2"
ALIPHYSICS_BUILD=0
ALIPHYSICS_SPECS=("$ALIPHYSICS_NAME" "$ALIPHYSICS_UPDATE" "$ALIPHYSICS_DIR" "$ALIPHYSICS_REMOTE_MAIN" "$ALIPHYSICS_REMOTE_FORK" "$ALIPHYSICS_BRANCH_MAIN" "$ALIPHYSICS_BUILD_OPT" "$ALIPHYSICS_BUILD")

# O2
O2_NAME="O2"
O2_UPDATE=1
O2_DIR="$ALICE_DIR/O2"
O2_REMOTE_MAIN="upstream"
O2_REMOTE_FORK="origin"
O2_BRANCH_MAIN="dev"
O2_BUILD_OPT="--defaults o2"
O2_BUILD=1
O2_SPECS=("$O2_NAME" "$O2_UPDATE" "$O2_DIR" "$O2_REMOTE_MAIN" "$O2_REMOTE_FORK" "$O2_BRANCH_MAIN" "$O2_BUILD_OPT" "$O2_BUILD")

# O2Physics
O2PHYSICS_NAME="O2Physics"
O2PHYSICS_UPDATE=1
O2PHYSICS_DIR="$ALICE_DIR/O2Physics"
O2PHYSICS_REMOTE_MAIN="upstream"
O2PHYSICS_REMOTE_FORK="origin"
O2PHYSICS_BRANCH_MAIN="master"
O2PHYSICS_BUILD_OPT="--defaults o2"
O2PHYSICS_BUILD=1
O2PHYSICS_SPECS=("$O2PHYSICS_NAME" "$O2PHYSICS_UPDATE" "$O2PHYSICS_DIR" "$O2PHYSICS_REMOTE_MAIN" "$O2PHYSICS_REMOTE_FORK" "$O2PHYSICS_BRANCH_MAIN" "$O2PHYSICS_BUILD_OPT" "$O2PHYSICS_BUILD")

# DelphesO2
DELPHESO2_NAME="DelphesO2"
DELPHESO2_UPDATE=0
DELPHESO2_DIR="$ALICE_DIR/DelphesO2"
DELPHESO2_REMOTE_MAIN="upstream"
DELPHESO2_REMOTE_FORK=""
DELPHESO2_BRANCH_MAIN="master"
DELPHESO2_BUILD_OPT="--defaults o2"
DELPHESO2_BUILD=0
DELPHESO2_SPECS=("$DELPHESO2_NAME" "$DELPHESO2_UPDATE" "$DELPHESO2_DIR" "$DELPHESO2_REMOTE_MAIN" "$DELPHESO2_REMOTE_FORK" "$DELPHESO2_BRANCH_MAIN" "$DELPHESO2_BUILD_OPT" "$DELPHESO2_BUILD")

# O2DPG
O2DPG_NAME="O2DPG"
O2DPG_UPDATE=0
O2DPG_DIR="$ALICE_DIR/O2DPG"
O2DPG_REMOTE_MAIN="upstream"
O2DPG_REMOTE_FORK=""
O2DPG_BRANCH_MAIN="master"
O2DPG_BUILD_OPT="--defaults o2"
O2DPG_BUILD=0
O2DPG_SPECS=("$O2DPG_NAME" "$O2DPG_UPDATE" "$O2DPG_DIR" "$O2DPG_REMOTE_MAIN" "$O2DPG_REMOTE_FORK" "$O2DPG_BRANCH_MAIN" "$O2DPG_BUILD_OPT" "$O2DPG_BUILD")

# Run 3 validation
RUN3VALIDATE_NAME="Run 3 validation"
RUN3VALIDATE_UPDATE=1
RUN3VALIDATE_DIR="$DIR_REPO"
RUN3VALIDATE_REMOTE_MAIN="upstream"
RUN3VALIDATE_REMOTE_FORK="origin"
RUN3VALIDATE_BRANCH_MAIN="master"
RUN3VALIDATE_SPECS=("$RUN3VALIDATE_NAME" "$RUN3VALIDATE_UPDATE" "$RUN3VALIDATE_DIR" "$RUN3VALIDATE_REMOTE_MAIN" "$RUN3VALIDATE_REMOTE_FORK" "$RUN3VALIDATE_BRANCH_MAIN")

####################################################################################################

# List of packages to update/build (Put alidist first!)
LIST_PKG_SPECS=(
"ALIDIST_SPECS"
"ALIPHYSICS_SPECS"
"O2_SPECS"
"O2PHYSICS_SPECS"
#"DELPHESO2_SPECS"
#"O2DPG_SPECS"
"RUN3VALIDATE_SPECS"
)

# List of development aliBuild packages
LIST_PKG_DEV_SPECS=(
"ALIPHYSICS_SPECS"
"O2_SPECS"
"O2PHYSICS_SPECS"
#"DELPHESO2_SPECS"
#"O2DPG_SPECS"
)

####################################################################################################
