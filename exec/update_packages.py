#!/usr/bin/env python3

"""
This script updates local and remote Git repositories, builds aliBuild packages and does cleanup.
Author: Vít Kučera <vit.kucera@cern.ch>

The main Git branch (and the current one if different from the main one) is updated in the following way:
- Pull & rebase the branch from the same branch in the remote origin repository if specified.
- Pull & rebase the branch from the main branch in the main remote repository.
- Force-push the branch into the remote origin repository if specified.
aliBuild packages are built using the respective current branches and the specified build options.
"""

import argparse
import os
import subprocess as sp  # nosec B404
import sys

import yaml  # pylint: disable=import-error

# global variables
debug = False
alibuild_exists = False
alibuild_arch, alibuild_opt, alibuild_dir_alice, alibuild_dir_sw = "", "", "", ""
clean_do, clean_aggressive, clean_purge = 0, 0, 0


def eprint(*args, **kwargs):
    """Print to stderr."""
    print(*args, file=sys.stderr, **kwargs)


def msg_err(message: str):
    """Print an error message."""
    eprint("\x1b[1;31mError: %s\x1b[0m" % message)


def msg_fatal(message: str):
    """Print an error message and exit."""
    msg_err(message)
    sys.exit(1)


def msg_warn(message: str):
    """Print a warning message."""
    eprint("\x1b[1;36mWarning:\x1b[0m %s" % message)


def msg_bold(message: str):
    """Print a boldface message."""
    print("\x1b[1m%s\x1b[0m" % message)


def msg_step(message: str):
    """Print a message at the main step level."""
    print("\x1b[1;32m\n%s\x1b[0m" % message)


def msg_substep(message: str):
    """Print a message at the substep level."""
    print("\x1b[1m\n%s\x1b[0m" % message)


def msg_subsubstep(message: str):
    """Print a message at the subsubstep level."""
    print("\x1b[4m%s\x1b[0m" % message)


def is_allowed(string: str):
    """Check whether string contains only allowed characters."""
    allowed = " $/-=':%_"  # allowed non-alphanumeric characters
    for c in string:
        if not c.isalnum() and c not in allowed:
            return False
    return True


def exec_cmd(cmd: str, msg=None, silent=False, safe=False):
    """Execute a shell command."""
    if debug:
        eprint(cmd)
    # Protect against injected command
    if not safe and not is_allowed(cmd):
        msg_fatal("Command contains forbidden characters!")
    try:
        if silent:
            sp.run(cmd, shell=True, check=True, stdout=sp.DEVNULL, stderr=sp.DEVNULL)  # nosec B602
        else:
            sp.run(cmd, shell=True, check=True)  # nosec B602
    except sp.CalledProcessError:
        msg_fatal(msg if msg else f"executing: {cmd}")


def get_cmd(cmd: str, msg=None, safe=False) -> str:
    """Get output of a shell command."""
    if debug:
        eprint(cmd)
    # Protect against injected command
    if not safe and not is_allowed(cmd):
        msg_fatal("Command contains forbidden characters!")
    try:
        out = sp.check_output(cmd, shell=True, text=True)  # nosec B602
        return out.strip()
    except sp.CalledProcessError:
        msg_fatal(msg if msg else f"executing: {cmd}")
        return ""


def chdir(path: str):
    """Change directory."""
    path_real = get_cmd(f"realpath {path}")
    if not os.path.isdir(path_real):
        msg_fatal(f"{path} does not exist.")
    os.chdir(path_real)


def sizeof_fmt(num, unit="B", base=1000):
    """Express a value in the appropriate order of units."""
    for prefix in ["", "k", "M", "G", "T", "P", "E", "Z"]:
        if abs(num) < base:
            return f"{num:3.1f} {prefix}{unit}"
        num /= base
    return f"{num:.1f} Y{unit}"


def healthy_structure(dic_full: dict):
    """Check correct structure of the database and load global settings."""
    if not isinstance(dic_full, dict):
        msg_err("No dictionary found.")
        return False

    # Check the aliBuild database.
    # default aliBuild settings
    template_alibuild = {
        "dir_alice": "$HOME/alice",
        "architecture": "",
        "options": "",
        "clean": 1,
        "clean_aggressive": 0,
        "clean_purge": 0,
    }
    if "aliBuild" not in dic_full:
        dic_full["aliBuild"] = template_alibuild
        dic_alibuild = dic_full["aliBuild"]
    else:
        dic_alibuild = dic_full["aliBuild"]
        if not isinstance(dic_alibuild, dict):
            msg_err('"aliBuild" is not a dictionary.')
            return False
        for key, val in template_alibuild.items():
            dic_alibuild[key] = dic_alibuild.get(key, val)  # Create a default value if not set.
    global alibuild_arch, alibuild_dir_alice, alibuild_opt, alibuild_dir_sw, clean_do, clean_aggressive, clean_purge
    alibuild_arch = dic_alibuild["architecture"]
    alibuild_dir_alice = dic_alibuild["dir_alice"]
    if not alibuild_dir_alice:
        msg_fatal(f"Invalid path: {alibuild_dir_alice}.")
    alibuild_dir_alice_real = get_cmd(f"realpath {alibuild_dir_alice}")
    if alibuild_exists and not os.path.isdir(alibuild_dir_alice_real):
        msg_fatal(f"{alibuild_dir_alice} does not exist.")
    alibuild_opt = dic_alibuild["options"]
    alibuild_dir_sw = os.environ["ALIBUILD_WORK_DIR"]
    if not alibuild_dir_sw:
        msg_fatal("ALIBUILD_WORK_DIR is not defined.")
    if alibuild_exists and not os.path.isdir(alibuild_dir_sw):
        msg_fatal(f"{alibuild_dir_sw} does not exist.")
    clean_do = dic_alibuild["clean"]
    clean_aggressive = dic_alibuild["clean_aggressive"]
    clean_purge = dic_alibuild["clean_purge"]

    # Check the repository database.
    if "repositories" not in dic_full:
        msg_err('Key "repositories" not found in the database.')
        return False
    dic_repos = dic_full["repositories"]
    if not isinstance(dic_repos, dict):
        msg_err('"repositories" is not a dictionary.')
        return False
    for repo, dic_repo in dic_repos.items():
        if not isinstance(dic_repo, dict):
            msg_err(f"{repo} is not a dictionary.")
            return False
        # default repository settings
        template_repo = {
            "path": f"{alibuild_dir_alice}/{repo}",
            "update": 1,
            "upstream": "upstream",
            "origin": "",
            "branch": "master",
        }
        for key, val in template_repo.items():
            dic_repo[key] = dic_repo.get(key, val)  # Create a default value if not set.
    return True


def package_summary(repo: str, dic_repo: dict):
    """Print a summary of package parameters."""
    print("\nName:", repo)
    print("Path:", dic_repo["path"])
    print("Main branch:", dic_repo["branch"])
    print("Upstream remote:", dic_repo["upstream"])
    print("Origin remote:", dic_repo["origin"])
    print("Update:", dic_repo["update"])
    if "build" in dic_repo:
        print("Build:", dic_repo["build"])
        if "build_opt" in dic_repo:
            print("Build options:", dic_repo["build_opt"])


def get_branch(path=None):
    """Get the name of the current branch in a Git repository."""
    if path:
        chdir(path)
    return get_cmd("git rev-parse --abbrev-ref HEAD", "Failed to get branch")


def get_last_commit(path: str):
    """Get the name of the current branch and the last commit in a Git repository."""
    chdir(path)
    branch = get_branch()
    commit = get_cmd("git log -n 1 --pretty='format:%ci %h %s'", "Failed to get commit")
    return f"{branch} {commit}"


def update_branch(remote_upstream, remote_origin, branch_main, branch_current):
    """Update the current branch in a Git repository."""
    msg_substep(f"- Updating branch {branch_current}")
    exec_cmd(f"git checkout {branch_current}")

    # Synchronise with the origin first, just in case there are some commits pushed from another local repository.
    if remote_origin:
        msg_subsubstep(f"-- Updating branch {branch_current} from {remote_origin}/{branch_current}")
        exec_cmd(f"git pull --rebase {remote_origin} {branch_current}")

    # Synchronise with upstream/main.
    msg_subsubstep(f"-- Updating branch {branch_current} from {remote_upstream}/{branch_main}")
    exec_cmd(f"git pull --rebase {remote_upstream} {branch_main}")

    # Push to the origin.
    if remote_origin:
        msg_subsubstep(f"-- Pushing branch {branch_current} to {remote_origin}")
        exec_cmd(f"git push -f {remote_origin} {branch_current}")


def update_git(dic_repo: dict):
    """Update a Git repository."""
    remote_upstream = dic_repo["upstream"]
    remote_origin = dic_repo["origin"]
    branch_main = dic_repo["branch"]

    # Move to the Git repository and get the name of the current branch.
    chdir(dic_repo["path"])
    branch_current = get_branch()
    print(f"Current branch: {branch_current}")

    # Skip update when on detached HEAD.
    if branch_current == "HEAD":
        msg_substep("- Skipping update because of detached HEAD")
        return

    # Stash uncommitted local changes.
    msg_substep("- Stashing potential uncommitted local changes")
    n_stash_old = get_cmd("git stash list | wc -l", safe=True)
    exec_cmd("git stash")
    n_stash_new = get_cmd("git stash list | wc -l", safe=True)

    # Update the main and the current branch. (Remove duplicates.)
    # TODO: multiple branches
    for branch in dict.fromkeys((branch_main, branch_current)):
        update_branch(remote_upstream, remote_origin, branch_main, branch)

    # Unstash stashed changes if any.
    if n_stash_new != n_stash_old:
        msg_substep("- Unstashing uncommitted local changes")
        exec_cmd("git stash pop")


def build_package(pkg: str, dic_pkg: dict, silent=False):
    """Build a package with aliBuild."""
    if not silent:
        msg_substep(f"Building {pkg}")
    chdir(alibuild_dir_alice)
    opt_build_pkg = dic_pkg.get("build_opt", "")
    opt_arch = f"-a {alibuild_arch}"
    exec_cmd(
        f"aliBuild build {pkg} {' '.join((opt_arch, alibuild_opt, opt_build_pkg))}",
        silent=silent,
    )


def update_package(repo: str, dic_repo: dict):
    """Update a Git repository and build the package with aliBuild."""
    msg_step(f"Updating {repo}")
    # print(dic_repo)
    # Update repository.
    if dic_repo.get("update", False):
        update_git(dic_repo)
    else:
        print("Update deactivated. Skipping")
    # Build package.
    if dic_repo.get("build", False):
        if alibuild_exists:
            build_package(repo, dic_repo)
        else:
            msg_warn("Skipping build because of absent aliBuild.")


def main():
    """Main function"""
    parser = argparse.ArgumentParser(
        description="This script updates local and remote Git repositories, builds aliBuild packages and does cleanup."
    )
    parser.add_argument("database", help="database with package configuration and options")
    parser.add_argument("-d", "--debug", action="store_true", help="print debugging info")
    parser.add_argument("-l", action="store_true", help="print latest commits and exit")
    parser.add_argument("-c", action="store_true", help="print configuration and exit")
    args = parser.parse_args()
    path_file_database = args.database
    global debug
    debug = args.debug
    show_config = args.c
    show_commits = args.l

    # Open database input file.
    if show_config:
        print(f"Input database: {path_file_database}")
    try:
        with open(path_file_database, "r") as file_database:
            dic_in = yaml.safe_load(file_database)
    except IOError:
        msg_fatal(f"Failed to open file {path_file_database}")

    # Check valid structure of the input database and load global settings.
    if not healthy_structure(dic_in):
        msg_fatal("Bad structure!")

    dic_repos = dic_in["repositories"]

    # Check aliBuild
    global alibuild_exists
    try:
        get_cmd("which aliBuild", "aliBuild not found")
        alibuild_exists = True
    except SystemExit:
        msg_warn("aliBuild commands will be skipped.")

    global alibuild_arch
    if not alibuild_arch and alibuild_exists:
        alibuild_arch = get_cmd("aliBuild architecture", "Failed to get architecture")

    # Dry run: Print out configuration and exit.
    if show_config:
        msg_step("Configuration")
        if alibuild_exists:
            print(get_cmd("aliBuild version", "Failed to get aliBuild version"))
        print(f"Architecture: {alibuild_arch}")
        print(f"aliBuild work dir: {alibuild_dir_sw}")
        print(f"aliBuild build dir: {alibuild_dir_alice}")
        print(f"aliBuild options: {alibuild_opt}")
        print("Clean builds:", "yes" if clean_do else "no")
        print("Clean agressively:", "yes" if clean_aggressive else "no")
        print("Purge builds:", "yes" if clean_purge else "no")
        msg_substep("Package settings")
        for repo, dic_repo in dic_repos.items():
            package_summary(repo, dic_repo)
        return

    if show_commits:
        msg_step("Latest commits")
        for repo, dic_repo in dic_repos.items():
            print(repo, get_last_commit(dic_repo["path"]))
        return

    # Update all packages.
    for repo, dic_repo in dic_repos.items():
        update_package(repo, dic_repo)

    # Cleanup
    if clean_do and alibuild_exists:
        msg_step("Cleaning aliBuild files")
        alibuild_dir_arch = f"{alibuild_dir_sw}/{alibuild_arch}"
        alibuild_dir_build = f"{alibuild_dir_sw}/BUILD"

        # Get the directory size before cleaning.
        msg_substep(f"- Estimating size of {alibuild_dir_sw}")
        size_before = int(get_cmd(f"du -sb {alibuild_dir_sw}").split()[0])

        # Delete all symlinks to builds and recreate the latest ones to allow deleting of all other builds.
        if clean_purge:
            msg_substep("- Purging builds")
            msg_warn("This action will run 'aliBuild build' for each development package.")
            # Check existence of the build directories.
            msg_subsubstep("-- Checking existence of the build directories")
            for dir in (alibuild_dir_arch, alibuild_dir_build):
                if not os.path.isdir(dir):
                    msg_fatal(f"Directory {dir} does not exist.")
            # Delete symlinks to all builds.
            msg_subsubstep("-- Deleting symlinks to all builds")
            exec_cmd(
                f"find {alibuild_dir_arch} -mindepth 2 -maxdepth 2 -type l -delete",
                f"Failed to delete symlinks in {alibuild_dir_arch}.",
            )
            exec_cmd(
                f"find {alibuild_dir_build} -mindepth 1 -maxdepth 1 -type l -delete",
                f"Failed to delete symlinks in {alibuild_dir_build}.",
            )
            # Recreate symlinks to the latest builds of development packages and their dependencies.
            for repo, dic_repo in dic_repos.items():
                if "build" in dic_repo:  # only development packages
                    msg_subsubstep(f"-- Re-building {repo} to recreate symlinks")
                    build_package(repo, dic_repo, silent=True)  # suppress output

        # Delete obsolete builds.
        msg_substep("- Deleting obsolete builds")
        if clean_aggressive:
            msg_warn("Using aggressive cleanup")
        opt_clean = "--aggressive-cleanup" if clean_aggressive else ""
        opt_arch = f"-a {alibuild_arch}"
        chdir(alibuild_dir_alice)
        exec_cmd(f"aliBuild clean {' '.join((opt_arch, alibuild_opt, opt_clean))}")

        # Recreate symlinks to the source of development packages in sw/SOURCES.
        if clean_aggressive:
            os.makedirs(f"{alibuild_dir_sw}/SOURCES")
            for repo, dic_repo in dic_repos.items():
                if "build" in dic_repo:  # only development packages
                    msg_subsubstep(f"-- Recreating symlinks in SOURCES to {repo}")
                    path_link = f"{alibuild_dir_sw}/SOURCES/{repo}/{dic_repo['branch']}"
                    os.makedirs(path_link)
                    os.symlink(get_cmd(f"realpath {dic_repo['path']}"), f"{path_link}/0")

        # Get the directory size after cleaning.
        msg_substep(f"- Estimating size of {alibuild_dir_sw}")
        size_after = int(get_cmd(f"du -sb {alibuild_dir_sw}").split()[0])
        # Report size difference.
        size_diff = size_before - size_after
        # Convert the number of bytes to a human-readable format.
        size_diff = sizeof_fmt(size_diff)
        size_after = sizeof_fmt(size_after)
        # try:
        #     size_diff = get_cmd(f"numfmt --to=si --round=nearest -- {size_diff}")
        #     size_after = get_cmd(f"numfmt --to=si --round=nearest -- {size_after}")
        # except:
        #     pass
        print(f"\nFreed up {size_diff} of disk space.")
        print(f"Directory {alibuild_dir_sw} takes {size_after}.")

    # Print out latest commits.
    msg_step("Latest commits")
    for repo, dic_repo in dic_repos.items():
        print(repo, get_last_commit(dic_repo["path"]))

    msg_step("Done")


main()
