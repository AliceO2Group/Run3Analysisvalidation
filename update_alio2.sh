#!/bin/bash

# This script performs a complete update of an AliPhysics + O2 installation.

# The main Git branch (and the current one if different from the main one) is updated in the following way:
# - Pull & rebase the branch from the same branch in the remote fork repository if specified.
# - Pull & rebase the branch from the main branch in the main remote repository.
# - Force-push the branch into the remote fork repository if specified.
# AliPhysics and O2 are built using the respective current branches and the specified build options.

#################################################################
# User settings:
# paths, names of remotes (check git remote -v), build options.

# Delete unnecessary files.
CLEAN=1
CLEAN_AGGRESSIVE=0 # Delete store and SOURCES except for AliPhysics and O2.

# Delete all builds other than the latest ones (i.e. the ones currently being built in this script).
# WARNING: Do not enable this if you need to keep several builds of the same package (AliPhysics, O2) (e.g. for different branches or commits).
PURGE_BUILDS=0 # NOT WORKING YET!

# Print out an overview of the latest commits of repositories.
PRINT_COMMITS=1

# Main ALICE software directory
ALICE_DIR="$HOME/alice"

# aliBuild
ALIBUILD_ARCH=$(aliBuild architecture) # system architecture
ALIBUILD_OPT="-a $ALIBUILD_ARCH"

# alidist
ALIDIST_UPDATE=1
ALIDIST_DIR="$ALICE_DIR/alidist"
ALIDIST_REMOTE_MAIN="upstream"
ALIDIST_REMOTE_FORK=""
ALIDIST_BRANCH_MAIN="master"

# AliPhysics
ALIPHYSICS_UPDATE=1
ALIPHYSICS_DIR="$ALICE_DIR/AliPhysics"
ALIPHYSICS_REMOTE_MAIN="upstream"
ALIPHYSICS_REMOTE_FORK="origin"
ALIPHYSICS_BRANCH_MAIN="master"
ALIPHYSICS_BUILD_OPT="--defaults user-next-root6"
ALIPHYSICS_BUILD=1

# O2
O2_UPDATE=1
O2_DIR="$ALICE_DIR/O2"
O2_REMOTE_MAIN="upstream"
O2_REMOTE_FORK="origin"
O2_BRANCH_MAIN="dev"
O2_BUILD_OPT="--defaults o2"
O2_BUILD=1

# Run 3 validation
RUN3VALIDATE_UPDATE=1
RUN3VALIDATE_DIR="$(dirname $(realpath $0))"
RUN3VALIDATE_REMOTE_MAIN="upstream"
RUN3VALIDATE_REMOTE_FORK="origin"
RUN3VALIDATE_BRANCH_MAIN="master"
#################################################################

# Report error and exit
function ErrExit { echo -e "\e[1;31mError\e[0m"; exit 1; }

# Message formatting
function MsgStep { echo -e "\n\e[1;32m$@\e[0m"; }
function MsgSubStep { echo -e "\n\e[1m$@\e[0m"; }
function MsgSubSubStep { echo -e "\e[4m$@\e[0m"; }
function MsgWarn { echo -e "\e[1;36m$@\e[0m"; }

function PrintLastCommit {
  BRANCH=$(git rev-parse --abbrev-ref HEAD)
  COMMIT=$(git log -n 1 --pretty='format:%ci %h %s')
  echo "$BRANCH $COMMIT"
}

# Update a given branch and push to the fork repository (if specified).
function UpdateBranch {
  REMOTE_MAIN_="$1" # Main remote (upstream)
  BRANCH_MAIN_="$2" # Main branch
  BRANCH_="$3"      # Current branch to be updated
  REMOTE_FORK_=""   # Fork remote
  [ "$4" ] && REMOTE_FORK_="$4"

  MsgSubStep "- Updating branch $BRANCH_"
  git checkout $BRANCH_ || ErrExit

  # Synchronise with the fork first, just in case there are some commits pushed from another local repository.
  if [ "$REMOTE_FORK_" ]; then
    MsgSubSubStep "-- Updating branch $BRANCH_ from $REMOTE_FORK_/$BRANCH_"
    git pull --rebase $REMOTE_FORK_ $BRANCH_ || ErrExit
  fi

  # Synchronise with upstream/main.
  MsgSubSubStep "-- Updating branch $BRANCH_ from $REMOTE_MAIN_/$BRANCH_MAIN_"
  git pull --rebase $REMOTE_MAIN_ $BRANCH_MAIN_ || ErrExit

  # Push to the fork.
  if [ "$REMOTE_FORK_" ]; then
    MsgSubSubStep "-- Pushing branch $BRANCH_ to $REMOTE_FORK_"
    git push -f $REMOTE_FORK_ $BRANCH_ || ErrExit
  fi
}

# Update the main and the current branch and push to the fork repository (if specified).
function UpdateGit {
  DIR="$1"         # Git repository directory
  REMOTE_MAIN="$2" # Main remote (upstream)
  BRANCH_MAIN="$3" # Main branch
  REMOTE_FORK=""   # Fork remote
  [ "$4" ] && REMOTE_FORK="$4"

  # Move to the Git repository and get the name of the current branch.
  cd "$DIR" && BRANCH=$(git rev-parse --abbrev-ref HEAD) || ErrExit
  #echo "Directory: $DIR"
  echo "Current branch: $BRANCH"

  # Skip update when on detached HEAD.
  [ "$BRANCH" == "HEAD" ] && { MsgSubStep "- Skipping update because of detached HEAD"; return; }

  # Stash uncommitted local changes.
  MsgSubStep "- Stashing potential uncommitted local changes"
  NSTASH_OLD=$(git stash list | wc -l) && \
  git stash && \
  NSTASH_NEW=$(git stash list | wc -l) || ErrExit

  # Update the main branch.
  UpdateBranch $REMOTE_MAIN $BRANCH_MAIN $BRANCH_MAIN $REMOTE_FORK || ErrExit

  # Update the current branch.
  [ "$BRANCH" != "$BRANCH_MAIN" ] && { UpdateBranch $REMOTE_MAIN $BRANCH_MAIN $BRANCH $REMOTE_FORK || ErrExit; }

  # Unstash stashed changes if any.
  [ $NSTASH_NEW -ne $NSTASH_OLD ] && { MsgSubStep "- Unstashing uncommitted local changes"; git stash pop || ErrExit; }
}

# alidist
if [ $ALIDIST_UPDATE -eq 1 ]; then
  MsgStep "Updating alidist"
  UpdateGit "$ALIDIST_DIR" $ALIDIST_REMOTE_MAIN $ALIDIST_BRANCH_MAIN $ALIDIST_REMOTE_FORK
fi

# AliPhysics
if [ $ALIPHYSICS_UPDATE -eq 1 ]; then
  MsgStep "Updating AliPhysics"
  UpdateGit "$ALIPHYSICS_DIR" $ALIPHYSICS_REMOTE_MAIN $ALIPHYSICS_BRANCH_MAIN $ALIPHYSICS_REMOTE_FORK
  [ $ALIPHYSICS_BUILD -eq 1 ] && { MsgSubStep "- Building AliPhysics"; cd "$ALICE_DIR" && aliBuild build AliPhysics $ALIPHYSICS_BUILD_OPT $ALIBUILD_OPT || ErrExit; }
fi

# O2
if [ $O2_UPDATE -eq 1 ]; then
  MsgStep "Updating O2"
  UpdateGit "$O2_DIR" $O2_REMOTE_MAIN $O2_BRANCH_MAIN $O2_REMOTE_FORK
  [ $O2_BUILD -eq 1 ] && { MsgSubStep "- Building O2"; cd "$ALICE_DIR" && aliBuild build O2 $O2_BUILD_OPT $ALIBUILD_OPT || ErrExit; }
fi

# Run 3 validation
if [ $RUN3VALIDATE_UPDATE -eq 1 ]; then
  MsgStep "Updating Run3Analysisvalidation"
  UpdateGit "$RUN3VALIDATE_DIR" $RUN3VALIDATE_REMOTE_MAIN $RUN3VALIDATE_BRANCH_MAIN $RUN3VALIDATE_REMOTE_FORK
fi

# Cleanup
  if [ $CLEAN -eq 1 ]; then
  MsgStep "Cleaning builds"

  # Delete all symlinks to builds and recreate the latest ones to allow deleting of all other builds.
  if [ $PURGE_BUILDS -eq 1 ]; then
    MsgWarn "Purging builds"
    # Delete all symlinks to builds.
    MsgSubStep "- Deleting symlinks to builds"
    find "$ALICE_DIR/sw/$ALIBUILD_ARCH/" -mindepth 2 -maxdepth 2 -type l -delete || ErrExit
    find "$ALICE_DIR/sw/BUILD" -mindepth 1 -maxdepth 1 -type l -delete || ErrExit
    # Recreate symlinks to the latest builds.
    MsgSubStep "- Re-building AliPhysics"; cd "$ALICE_DIR" && aliBuild build AliPhysics $ALIPHYSICS_BUILD_OPT $ALIBUILD_OPT > /dev/null 2>&1 || ErrExit
    MsgSubStep "- Re-building O2"; cd "$ALICE_DIR" && aliBuild build O2 $O2_BUILD_OPT $ALIBUILD_OPT > /dev/null 2>&1 || ErrExit
  fi

  aliBuild clean $ALIBUILD_OPT

  # Delete store and SOURCES (except for AliPhysics and O2).
  if [ $CLEAN_AGGRESSIVE -eq 1 ]; then
    MsgWarn "Cleaning aggressively"
    #find "$ALICE_DIR/sw/SOURCES" -mindepth 1 -maxdepth 1 ! -name AliPhysics ! -name O2 -exec rm -rf {} \; || ErrExit
    rm -rf "$ALICE_DIR/sw/TARS/$ALIBUILD_ARCH/store" || ErrExit
    # Delete broken links.
    #find -L "$ALICE_DIR/sw/TARS/$ALIBUILD_ARCH" -type l -delete || ErrExit
  fi
fi

# Print out latest commits.
if [ $PRINT_COMMITS -eq 1 ]; then
  MsgStep "Latest commits"
  echo "alidist: $( cd "$ALIDIST_DIR" && PrintLastCommit )"
  echo "AliPhysics: $( cd "$ALIPHYSICS_DIR" && PrintLastCommit )"
  echo "O2: $( cd "$O2_DIR" && PrintLastCommit )"
  echo "Run 3 validation: $( cd "$RUN3VALIDATE_DIR" && PrintLastCommit )"
fi

MsgStep "Done"

exit 0
