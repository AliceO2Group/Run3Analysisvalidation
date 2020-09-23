#!/bin/bash

# This script performs a complete update of an AliPhysics + O2 installation.

# The Git repositories are updated in the following way:
# - Pull & rebase the main branch from the main remote repository.
# - Rebase the current branch (if different from the main one) from the main branch in the main remote repository.
# - Force push both branches to the remote fork repository if specified.
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

# Update the main and the current branch and push to the fork repository (if specified).
function UpdateGit {
  DIR="$1"
  REMOTE_MAIN="$2"
  BRANCH_MAIN="$3"
  REMOTE_FORK=""
  if [ "$4" ]; then REMOTE_FORK="$4"; fi

  cd "$DIR" && \
  BRANCH=$(git rev-parse --abbrev-ref HEAD) || $ERREXIT
  echo "Current branch: $BRANCH"

  # Update the main branch.
  echo "Updating branch $BRANCH_MAIN from $REMOTE_MAIN"
  git checkout $BRANCH_MAIN && \
  git pull --rebase $REMOTE_MAIN $BRANCH_MAIN || $ERREXIT
  if [ "$REMOTE_FORK" ]; then
    echo "Pushing branch $BRANCH_MAIN to $REMOTE_FORK"
    git push -f $REMOTE_FORK $BRANCH_MAIN || $ERREXIT
  fi

  # Update the current branch.
  if [ "$BRANCH" != "$BRANCH_MAIN" ]; then
    echo "Updating branch $BRANCH from $REMOTE_MAIN"
    git checkout $BRANCH && \
    git fetch $REMOTE_MAIN && \
    git rebase $REMOTE_MAIN/$BRANCH_MAIN || $ERREXIT
    if [ "$REMOTE_FORK" ]; then
      echo "Pushing branch $BRANCH to $REMOTE_FORK"
      git push -f $REMOTE_FORK $BRANCH || $ERREXIT
    fi
  fi
}

# alidist
echo -e "\nUpdating alidist"
UpdateGit "$ALIDIST_DIR" $ALIDIST_REMOTE_MAIN $ALIDIST_BRANCH_MAIN $ALIDIST_REMOTE_FORK

# AliPhysics
echo -e "\nUpdating AliPhysics"
UpdateGit "$ALIPHYSICS_DIR" $ALIPHYSICS_REMOTE_MAIN $ALIPHYSICS_BRANCH_MAIN $ALIPHYSICS_REMOTE_FORK
cd "$ALICE_DIR" && \
aliBuild build AliPhysics $ALIPHYSICS_BUILDOPT || $ERREXIT

# O2
echo -e "\nUpdating O2"
UpdateGit "$O2_DIR" $O2_REMOTE_MAIN $O2_BRANCH_MAIN $O2_REMOTE_FORK
cd "$ALICE_DIR" && \
aliBuild build O2 $O2_BUILDOPT || $ERREXIT

# Cleanup
echo -e "\nCleaning builds"
aliBuild clean

echo -e "\nDone"

exit 0
