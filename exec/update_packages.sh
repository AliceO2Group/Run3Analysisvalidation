#!/bin/bash

# This script updates local and remote Git repositories, builds aliBuild packages and does cleanup.

# The main Git branch (and the current one if different from the main one) is updated in the following way:
# - Pull & rebase the branch from the same branch in the remote fork repository if specified.
# - Pull & rebase the branch from the main branch in the main remote repository.
# - Force-push the branch into the remote fork repository if specified.
# aliBuild packages are built using the respective current branches and the specified build options.

####################################################################################################

# Settings

# This repository directory
DIR_REPO="$(realpath $(dirname $0)/..)"

# Configuration file
FILE_CONFIG="$DIR_REPO/config/config_update.sh"

# Main ALICE software directory
ALICE_DIR="$HOME/alice"

# aliBuild
[ "$(which aliBuild)" ] || ErrExit "aliBuild not found"
ALIBUILD_ARCH=$(aliBuild architecture)  # system architecture (as detected by aliBuild)
ALIBUILD_OPT="-a $ALIBUILD_ARCH"
ALIBUILD_DIR_ARCH="$ALICE_DIR/sw/$ALIBUILD_ARCH"  # directory with builds of all packages
ALIBUILD_DIR_BUILD="$ALICE_DIR/sw/BUILD"  # directory with builds of development packages

# List of packages to update/build
LIST_PKG_SPECS=()

# List of development aliBuild packages
LIST_PKG_DEV_SPECS=()

# Delete unnecessary files.
CLEAN=1

# Delete all builds that are not needed to run the latest AliPhysics and O2 builds.
# WARNING: Do not enable this if you need to keep several builds of AliPhysics or O2 (e.g. for different branches or commits) or builds of other development packages!
PURGE_BUILDS=0

# Print out an overview of the latest commits of repositories.
PRINT_COMMITS=1

####################################################################################################

# Message formatting
function MsgStep { echo -e "\n\e[1;32m$@\e[0m"; }
function MsgSubStep { echo -e "\n\e[1m$@\e[0m"; }
function MsgSubSubStep { echo -e "\e[4m$@\e[0m"; }
function MsgWarn { echo -e "\e[1;36m$@\e[0m"; }
function MsgErr { echo -e "\e[1;31m$@\e[0m"; }

# Throw error and exit.
function ErrExit {
  MsgErr "Error: $@"; exit 1;
}

# Print out package summary.
function PackageSummary {
  local -n Specs=$1
  NArgs=${#Specs[@]}
  [ "$NArgs" -lt 1 ] && ErrExit "Bad array."
  [ "$NArgs" -lt 6 ] && ErrExit "Package: ${Specs[0]}: Incomplete list of parameters."
  echo "Package: ${Specs[0]}"
  echo "Update: ${Specs[1]}"
  echo "Repository: ${Specs[2]}"
  echo "Upstream remote: ${Specs[3]}"
  echo "Fork remote: ${Specs[4]}"
  echo "Main branch: ${Specs[5]}"
  [ "$NArgs" -ge 8 ] && {
    echo "Build: ${Specs[7]}"
    echo "Build options: ${Specs[6]}"
  }
  return 0
}

# Print out name of the current branch and the hash of the last commit.
function PrintLastCommit {
  BRANCH=$(git rev-parse --abbrev-ref HEAD)
  COMMIT=$(git log -n 1 --pretty='format:%ci %h %s')
  echo "$BRANCH $COMMIT"
  return 0
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
  return 0
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
  [ "$BRANCH" == "HEAD" ] && { MsgSubStep "- Skipping update because of detached HEAD"; return 0; }

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
  return 0
}

function BuildPackage {
  PkgName="$1"
  PkgBuildOpt="$2"
  [ "$PkgName" ] || ErrExit "Empty package name"
  cd "$ALICE_DIR" && aliBuild build $PkgName $PkgBuildOpt $ALIBUILD_OPT || ErrExit;
}

# Do the full update of a package.
function UpdatePackage {
  # Get package specification parameters.
  local -n Specs=$1
  NArgs=${#Specs[@]}
  [ "$NArgs" -lt 1 ] && ErrExit "Bad array. $Specs"
  Name="${Specs[0]}"
  MsgStep "Updating $Name"
  [ "$NArgs" -lt 6 ] && ErrExit "Incomplete list of parameters."
  DoUpdate=${Specs[1]}
  PathRepo="${Specs[2]}"
  RemoteMain="${Specs[3]}"
  RemoteFork="${Specs[4]}"
  BranchMain="${Specs[5]}"
  DoBuild=0
  BuildOpt=""
  [ "$NArgs" -ge 8 ] && {
    BuildOpt="${Specs[6]}"
    DoBuild=${Specs[7]}
  }
  # Update repository.
  [ $DoUpdate -eq 1 ] && { UpdateGit "$PathRepo" $RemoteMain $BranchMain $RemoteFork; } || { echo "Update deactivated. Skipping"; }
  # Build package.
  [ $DoBuild -eq 1 ] && { MsgSubStep "- Building $Name"; BuildPackage "$Name" "$BuildOpt" || ErrExit; }
  return 0
}

function Help { echo "Usage: bash [<path>/]$(basename $0) [-h] [-c <config>] [-d]"; }

####################################################################################################

########## EXECUTION ##########

# Parse command line options.
while getopts ":hc:d" opt; do
  case ${opt} in
    h)
      Help; exit 0;;
    c)
      FILE_CONFIG="$OPTARG";;
    d)
      DRYRUN=1;;
    \?)
      MsgErr "Invalid option: $OPTARG" 1>&2; Help; exit 1;;
    :)
      MsgErr "Invalid option: $OPTARG requires an argument." 1>&2; Help; exit 1;;
  esac
done

# Load configuration.
source "$FILE_CONFIG" || ErrExit "Failed to load configuration from $FILE_CONFIG."

# Dry run: Print out configuration and exit.
if [ $DRYRUN -eq 1 ]; then
  MsgStep "Configuration:"
  echo "Configuration file: $FILE_CONFIG"
  echo "Main ALICE software directory: $ALICE_DIR"
  echo "System architecture: $ALIBUILD_ARCH"
  echo "Cleanup: $CLEAN"
  echo "Purging: $PURGE_BUILDS"
  echo "Print commits: $PRINT_COMMITS"
  MsgSubStep "Package settings:"
  for pkg in "${LIST_PKG_SPECS[@]}"; do
    arr="$pkg[@]"
    spec=("${!arr}")
    echo ""
    PackageSummary spec
  done
  MsgSubStep "Development packages:"
  for pkg in "${LIST_PKG_DEV_SPECS[@]}"; do
    arr="$pkg[@]"
    spec=("${!arr}")
    echo "${spec[0]}"
  done
  exit 0
fi

# Update all packages in the list.
for pkg in "${LIST_PKG_SPECS[@]}"; do
  arr="$pkg[@]"
  spec=("${!arr}")
  UpdatePackage spec || ErrExit
done

# Cleanup
if [ $CLEAN -eq 1 ]; then
  MsgStep "Cleaning aliBuild files"

  # Get the directory size before cleaning.
  SIZE_BEFORE=$(du -s $ALICE_DIR | cut -f1)

  # Delete all symlinks to builds and recreate the latest ones to allow deleting of all other builds.
  if [ $PURGE_BUILDS -eq 1 ]; then
    MsgSubStep "- Purging builds"
    # Check existence of the build directories.
    MsgSubSubStep "-- Checking existence of the build directories"
    [[ -d "$ALIBUILD_DIR_ARCH" && -d "$ALIBUILD_DIR_BUILD" ]] || ErrExit
    # Get paths to the latest builds of development packages. (They need to be recreated manually because aliBuild creates them only when the package needs to be rebuilt.)
    MsgSubSubStep "-- Getting paths to latest builds of development packages"
    arrPathBuild=()
    arrPathArch=()
    for pkg in "${LIST_PKG_DEV_SPECS[@]}" ; do
      arr="$pkg[@]"
      spec=("${!arr}")
      Name="${spec[0]}"
      path="$(realpath $ALIBUILD_DIR_BUILD/${Name}-latest)" && [ -d "$path" ] && arrPathBuild+=("$path") || ErrExit
      echo $path
      path="$(realpath $ALIBUILD_DIR_ARCH/${Name}/latest)" && [ -d "$path" ] && arrPathArch+=("$path") || ErrExit
      echo $path
    done
    # Delete symlinks to all builds.
    MsgSubSubStep "-- Deleting symlinks to all builds"
    find "$ALIBUILD_DIR_ARCH" -mindepth 2 -maxdepth 2 -type l -delete || ErrExit
    find "$ALIBUILD_DIR_BUILD" -mindepth 1 -maxdepth 1 -type l -delete || ErrExit
    # Recreate symlinks to the latest builds of development packages and their dependencies.
    for ((i = 0; i < ${#LIST_PKG_DEV_SPECS[@]}; ++i)); do
      pkg="${LIST_PKG_DEV_SPECS[i]}"
      arr="$pkg[@]"
      spec=("${!arr}")
      Name="${spec[0]}"
      BuildOpt="${spec[6]}"
      MsgSubSubStep "-- Re-building $Name to recreate symlinks of dependencies"; cd "$ALICE_DIR" && BuildPackage "$Name" "$BuildOpt" > /dev/null 2>&1 || ErrExit
      MsgSubSubStep "-- Recreating symlinks to the latest builds of $Name"
      echo "${arrPathBuild[i]}"
      echo "${arrPathArch[i]}"
      ln -snf "$(basename "${arrPathBuild[i]}")" "$(dirname "${arrPathBuild[i]}")/${Name}-latest" || ErrExit
      ln -snf "$(basename "${arrPathArch[i]}")" "$(dirname "${arrPathArch[i]}")/latest" || ErrExit
    done
  fi

  # Delete obsolete builds.
  MsgSubStep "- Deleting obsolete builds"
  cd "$ALICE_DIR" && aliBuild clean $ALIBUILD_OPT

  # Get the directory size after cleaning.
  SIZE_AFTER=$(du -s $ALICE_DIR | cut -f1)
  # Report size difference.
  SIZE_DIFF=$(( SIZE_BEFORE - SIZE_AFTER ))
  [ "$(which numfmt)" ] && SIZE_DIFF=$(numfmt --to=si $SIZE_DIFF) # Convert the number of bytes to a human-readable format.
  echo "Freed up ${SIZE_DIFF}B of disk space."
fi

# Print out latest commits.
if [ $PRINT_COMMITS -eq 1 ]; then
  MsgStep "Latest commits"
  for pkg in "${LIST_PKG_SPECS[@]}"; do
    arr="$pkg[@]"
    spec=("${!arr}")
    echo "${spec[0]}: $( cd "${spec[2]}" && PrintLastCommit || ErrExit )"
  done
fi

MsgStep "Done"

exit 0
