#!/usr/bin/env python3

"""
Generates full O2 command based on a YAML database of workflows and options.
Author: Vít Kučera <vit.kucera@cern.ch>
"""

import argparse
import os
import sys
from typing import List

import yaml  # pylint: disable=import-error


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
    eprint("\x1b[1m%s\x1b[0m" % message)


def join_strings(obj) -> str:
    """Return strings concatenated into one."""
    if isinstance(obj, str):
        return obj
    elif isinstance(obj, list):
        return " ".join(obj)
    else:
        msg_fatal("Cannot convert %s into a string" % type(obj))
        return ""


def join_to_list(obj, list_out: list):
    """Append string or list to another list."""
    if isinstance(obj, str):
        list_out.append(obj)
    elif isinstance(obj, list):
        list_out += obj
    else:
        msg_fatal("Cannot convert %s into a string" % type(obj))


def make_table_output(spec: str) -> str:
    """Format the output table descriptor."""
    words = spec.split("/")
    if len(words) > 2:
        return spec
    if len(words) == 1:
        return f"AOD/{spec}/0"
    if len(words) == 2:
        if words[0] in ("AOD", "AOD1", "DYN"):
            return f"{spec}/0"
        if words[1].isdigit():
            return f"AOD/{spec}"
    return spec


def healthy_structure(dic_full: dict):
    """Check correct structure of the database."""
    if not isinstance(dic_full, dict):
        msg_err("No dictionary found.")
        return False

    # Check mandatory database keys.
    good = True
    for key in ["workflows", "options"]:
        if key not in dic_full:
            msg_err('Key "%s" not found in the database.' % key)
            good = False
    if not good:
        return False

    # Check the options database.
    dic_opt = dic_full["options"]
    if not isinstance(dic_opt, dict):
        msg_err('"options" is not a dictionary.')
        return False
    # Check mandatory option keys.
    good = True
    for key in ["global", "local"]:
        if key not in dic_opt:
            msg_err('Key "%s" not found in the option database.' % key)
            good = False
    if not good:
        return False

    # Check the workflow database.
    dic_wf = dic_full["workflows"]
    if not isinstance(dic_wf, dict):
        msg_err('"workflows" is not a dictionary.')
        return False
    # Check workflow keys.
    for wf in dic_wf:
        dic_wf_single = dic_wf[wf]
        if not isinstance(dic_wf_single, dict):
            msg_err("%s is not a dictionary." % wf)
            return False
        if "activate" in dic_wf_single and not isinstance(dic_wf_single["activate"], bool):
            msg_err('"activate" in workflow %s is not a boolean.' % wf)
            return False
    return True


def activate_workflow(wf: str, dic_wf: dict, mc=False, level=0, debug=False):
    """Activate a workflows and its dependencies."""
    if debug:
        eprint((level + 1) * "  " + wf)
    if wf in dic_wf:
        dic_wf_single = dic_wf[wf]
        # Deactivate workflow if it needs MC and input is not MC.
        if "requires_mc" in dic_wf_single and dic_wf_single["requires_mc"] and not mc:
            msg_warn("Deactivated %s because of non-MC input" % wf)
            # Throw error if this is a dependency.
            if level > 0:
                msg_fatal("Workflows requiring this dependency would fail!")
            dic_wf_single["activate"] = False
            return
        # Activate.
        if "activate" not in dic_wf_single or not dic_wf_single["activate"]:
            dic_wf_single["activate"] = True
        # Activate dependencies recursively.
        if "dependencies" in dic_wf_single:
            list_dep: List[str] = []
            join_to_list(dic_wf_single["dependencies"], list_dep)
            for wf_dep in list_dep:
                activate_workflow(wf_dep, dic_wf, mc, level + 1, debug)
    else:
        # Add in the dictionary if not present.
        msg_warn("Adding an unknown workflow %s" % wf)
        dic_wf[wf] = {"activate": True}


def main():
    """Main function"""
    parser = argparse.ArgumentParser(
        description="Generates full O2 command based on a YAML " "database of workflows and options."
    )
    parser.add_argument("database", help="database with workflows and options")
    parser.add_argument("-w", "--workflows", type=str, help="explicitly requested workflows")
    parser.add_argument("--mc", action="store_true", help="Monte Carlo mode")
    parser.add_argument("-t", "--tables", action="store_true", help="save table into trees")
    parser.add_argument("-g", "--graph", action="store_true", help="make topology graph")
    parser.add_argument("-d", "--debug", action="store_true", help="print debugging info")
    parser.add_argument("-p", "--perf", action="store_true", help="produce performance profiling stats")
    args = parser.parse_args()
    path_file_database = args.database
    debug = args.debug
    workflows_add = args.workflows.split() if args.workflows else ""
    mc_mode = args.mc
    save_tables = args.tables
    make_graph = args.graph
    perf = args.perf

    # Open database input file.
    if debug:
        eprint("Input database: " + path_file_database)
    try:
        with open(path_file_database, "r") as file_database:
            dic_in = yaml.safe_load(file_database)
    except IOError:
        msg_fatal("Failed to open file " + path_file_database)

    # Check valid structure of the input database.
    if not healthy_structure(dic_in):
        msg_fatal("Bad structure!")

    if mc_mode:
        msg_warn("MC mode is on.")
    if save_tables:
        msg_warn("Tables will be saved in trees.")
    if perf:
        msg_warn(
            "Performance profiling stats will be saved in perf.data files.\n"
            "  Convert them with: perf script --demangle -i perf.data --no-inline |"
            " c++filt -r -t  > profile.linux-perf.txt\n"
            "  and upload the output to https://www.speedscope.app/."
        )

    # Get workflow-independent options.
    dic_opt = dic_in["options"]
    # options that appear only once
    opt_global = join_strings(dic_opt["global"])
    # options that appear for each workflow
    opt_local = join_strings(dic_opt["local"])

    # Get the workflow database.
    dic_wf = dic_in["workflows"]

    # Get list of primary workflows to run.
    # already activated in the database
    list_wf_activated = [wf for wf in dic_wf if "activate" in dic_wf[wf] and dic_wf[wf]["activate"]]
    if debug and list_wf_activated:
        eprint("\nWorkflows activated in the database:")
        eprint("\n".join("  " + wf for wf in list_wf_activated))
    # requested on command line
    if workflows_add:
        if debug:
            eprint("\nWorkflows specified on command line:")
            eprint("\n".join("  " + wf for wf in workflows_add))
        list_wf_activated += workflows_add
    # Remove duplicities.
    list_wf_activated = list(dict.fromkeys(list_wf_activated))
    if debug:
        eprint("\nPrimary workflows to run:")
        eprint("\n".join("  " + wf for wf in list_wf_activated))

    # Activate all needed workflows recursively.
    if debug:
        eprint("\nActivating workflows")
    for wf in list_wf_activated:
        activate_workflow(wf, dic_wf, mc_mode, 0, debug)

    # Get the list of tables and add the option to the local options.
    if save_tables:
        tables = []  # list of all tables of activated workflows
        for wf, dic_wf_single in dic_wf.items():
            if "activate" not in dic_wf_single or not dic_wf_single["activate"]:
                continue
            if "tables" not in dic_wf_single:
                continue
            tab_wf = dic_wf_single["tables"]
            if isinstance(tab_wf, (str, list)):
                join_to_list(tab_wf, tables)
            elif isinstance(tab_wf, dict):
                if "default" in tab_wf:
                    join_to_list(tab_wf["default"], tables)
                if not mc_mode and "real" in tab_wf:
                    join_to_list(tab_wf["real"], tables)
                if mc_mode and "mc" in tab_wf:
                    join_to_list(tab_wf["mc"], tables)
            else:
                msg_fatal('"tables" in %s must be str, list or dict, is %s' % (wf, type(tab_wf)))
        string_tables = ",".join(make_table_output(t) for t in tables)
        if string_tables:
            opt_local += " --aod-writer-keep " + string_tables

    # Compose the full command with all options.
    command = ""
    eprint("\nActivated workflows:")
    for wf, dic_wf_single in dic_wf.items():
        if "activate" not in dic_wf_single or not dic_wf_single["activate"]:
            continue
        msg_bold("  " + wf)
        # Determine the workflow executable.
        if "executable" in dic_wf_single:
            exec_wf = dic_wf_single["executable"]
            if not isinstance(exec_wf, str):
                msg_fatal('"executable" in %s must be str, is %s' % (wf, type(exec_wf)))
            string_wf = exec_wf
        else:
            string_wf = wf
        # Detect duplicate workflows.
        if string_wf + " " in command:
            msg_warn("Workflow %s is already present." % string_wf)
        # Process options.
        if "options" in dic_wf_single:
            opt_wf = dic_wf_single["options"]
            if isinstance(opt_wf, (str, list)):
                string_wf += " " + join_strings(opt_wf)
            elif isinstance(opt_wf, dict):
                if "default" in opt_wf:
                    string_wf += " " + join_strings(opt_wf["default"])
                if not mc_mode and "real" in opt_wf:
                    string_wf += " " + join_strings(opt_wf["real"])
                if mc_mode and "mc" in opt_wf:
                    string_wf += " " + join_strings(opt_wf["mc"])
            else:
                msg_fatal('"options" in %s must be str, list or dict, is %s' % (wf, type(opt_wf)))
        if opt_local:
            string_wf += " " + opt_local
        command += "| \\\n" + string_wf + " "
    if not command:
        msg_fatal("Nothing to do!")
    # Remove the leading "| \\\n" and the trailing " ".
    command = command[4:-1]
    # Append performance profiling options.
    if perf:
        opt_perf = "perf record -F 99 -g --call-graph dwarf --user-callchains"
        command = command.replace(string_wf, f"{opt_perf} {string_wf}")
    # Append global options.
    if opt_global:
        command += " " + opt_global

    # Print out the command.
    print(command)

    # Produce topology graph.
    if make_graph:
        basename, _ = os.path.splitext(path_file_database)
        ext_graph = "pdf"
        path_file_dot = basename + ".gv"
        path_file_graph = basename + "." + ext_graph
        eprint("Making diagram in: %s" % path_file_dot)
        dot = "digraph {\n"
        dot += "  edge [dir=back] // inverted arrow direction\n"
        dot += "  rankdir=BT // bottom to top drawing\n"
        dot += "  ranksep=2 // vertical node separation\n"
        dot += '  node [shape=box, style="filled,rounded", fillcolor=papayawhip, fontname=Courier, fontsize=20]\n'
        for wf, dic_wf_single in dic_wf.items():
            if "activate" not in dic_wf_single or not dic_wf_single["activate"]:
                continue
            # Hyphens are not allowed in node names.
            node_wf = wf.replace("-", "_")
            # Replace hyphens with line breaks to save horizontal space.
            label_wf = wf.replace("o2-analysis-", "")
            label_wf = label_wf.replace("-", "\\n")
            dot += '  %s [label="%s"]\n' % (node_wf, label_wf)
            if "dependencies" in dic_wf_single:
                nodes_dep = join_strings(dic_wf_single["dependencies"]).replace("-", "_")
                dot += "  %s -> {%s}\n" % (node_wf, nodes_dep)
        dot += "}\n"
        try:
            with open(path_file_dot, "w") as file_dot:
                file_dot.write(dot)
        except IOError:
            msg_fatal("Failed to open file " + path_file_dot)
        eprint("Produce graph with Graphviz: dot -T%s %s -o %s" % (ext_graph, path_file_dot, path_file_graph))


if __name__ == "__main__":
    main()
