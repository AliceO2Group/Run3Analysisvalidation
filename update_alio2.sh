#!/bin/bash

# This script performs a complete update of an AliPhysics + O2 installation.

# The main Git branch (and the current one if different from the main one) is updated in the following way:
# - Pull & rebase the branch from the same branch in the remote fork repository if specified.
# - Pull & rebase the branch from the main branch in the main remote repository.
# - Force-push the branch into the remote fork repository if specified.
# AliPhysics and O2 are built using the respective current branches and the specified build options.

##################
# User's settings:
# paths, names of remotes (check git remote -v), build options.

ALICE_DIR="$HOME/alice"

ALIDIST_DIR="$ALICE_DIR/alidist"
ALIDIST_REMOTE_MAIN="upstream"
ALIDIST_REMOTE_FORK=""
ALIDIST_BRANCH_MAIN="master"

ALIPHYSICS_DIR="$ALICE_DIR/AliPhysics"
ALIPHYSICS_REMOTE_MAIN="upstream"
ALIPHYSICS_REMOTE_FORK="origin"
ALIPHYSICS_BRANCH_MAIN="master"
ALIPHYSICS_BUILDOPT="--defaults user-next-root6"

O2_DIR="$ALICE_DIR/O2"
O2_REMOTE_MAIN="upstream"
O2_REMOTE_FORK="origin"
O2_BRANCH_MAIN="dev"
O2_BUILDOPT="--defaults o2"
##################

# Error report
ERREXIT="eval echo \"Error\"; exit 1"

# Update a given branch and push to the fork repository (if specified).
function UpdateBranch {
  REMOTE_MAIN_="$1" # Main remote (upstream)
  BRANCH_MAIN_="$2" # Main branch
  BRANCH_="$3"      # Current branch to be updated
  REMOTE_FORK_=""   # Fork remote
  [ "$4" ] && REMOTE_FORK_="$4"

  echo -e "\n- Updating branch $BRANCH_"
  git checkout $BRANCH_ || $ERREXIT

  # Synchronise with the fork first, just in case there are some commits pushed from another local repository.
  if [ "$REMOTE_FORK_" ]; then
    echo "-- Updating branch $BRANCH_ from $REMOTE_FORK_/$BRANCH_"
    git pull --rebase $REMOTE_FORK_ $BRANCH_ || $ERREXIT
  fi

  # Synchronise with upstream/main.
  echo "-- Updating branch $BRANCH_ from $REMOTE_MAIN_/$BRANCH_MAIN_"
  git pull --rebase $REMOTE_MAIN_ $BRANCH_MAIN_ || $ERREXIT

  # Push to the fork.
  if [ "$REMOTE_FORK_" ]; then
    echo "-- Pushing branch $BRANCH_ to $REMOTE_FORK_"
    git push -f $REMOTE_FORK_ $BRANCH_ || $ERREXIT
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
  cd "$DIR" && BRANCH=$(git rev-parse --abbrev-ref HEAD) || $ERREXIT
  echo "Current branch: $BRANCH"

  # Stash uncommitted local changes.
  echo -e "\n- Stashing potential uncommitted local changes"
  NSTASH_OLD=$(git stash list | wc -l) && \
  git stash && \
  NSTASH_NEW=$(git stash list | wc -l) || $ERREXIT

  # Update the main branch.
  UpdateBranch $REMOTE_MAIN $BRANCH_MAIN $BRANCH_MAIN $REMOTE_FORK || $ERREXIT

  # Update the current branch.
  [ "$BRANCH" != "$BRANCH_MAIN" ] && { UpdateBranch $REMOTE_MAIN $BRANCH_MAIN $BRANCH $REMOTE_FORK || $ERREXIT; }

  # Unstash stashed changes if any.
  [ $NSTASH_NEW -ne $NSTASH_OLD ] && { echo -e "\n- Unstashing uncommitted local changes"; git stash pop || $ERREXIT; }
}

# alidist
echo -e "\nUpdating alidist"
UpdateGit "$ALIDIST_DIR" $ALIDIST_REMOTE_MAIN $ALIDIST_BRANCH_MAIN $ALIDIST_REMOTE_FORK

# AliPhysics
echo -e "\nUpdating AliPhysics"
UpdateGit "$ALIPHYSICS_DIR" $ALIPHYSICS_REMOTE_MAIN $ALIPHYSICS_BRANCH_MAIN $ALIPHYSICS_REMOTE_FORK
echo -e "\n- Building AliPhysics"
cd "$ALICE_DIR" && aliBuild build AliPhysics $ALIPHYSICS_BUILDOPT || $ERREXIT

# O2
echo -e "\nUpdating O2"
UpdateGit "$O2_DIR" $O2_REMOTE_MAIN $O2_BRANCH_MAIN $O2_REMOTE_FORK
echo -e "\n- Building O2"
cd "$ALICE_DIR" && aliBuild build O2 $O2_BUILDOPT || $ERREXIT

# Cleanup
echo -e "\nCleaning builds"
aliBuild clean

echo -e "\nDone"

exit 0
