#!/usr/bin/env python3

"""
Generates full O2 command based on a YAML database of workflows and options.
Author: Vít Kučera <vit.kucera@cern.ch>
"""

import sys
import argparse
import yaml

def eprint(*args, **kwargs):
    '''Print to stderr.'''
    print(*args, file=sys.stderr, **kwargs)

def msg_err(message: str):
    '''Print an error message.'''
    eprint("\x1b[1;31mError: %s\x1b[0m" % message)

def msg_warn(message: str):
    '''Print a warning message.'''
    eprint("\x1b[1;36mWarning:\x1b[0m %s" % message)

def msg_bold(message: str):
    '''Print a boldface message.'''
    eprint("\x1b[1m%s\x1b[0m" % message)

def join_strings(obj):
    '''Return strings concatenated into one.'''
    if isinstance(obj, str):
        return obj
    elif isinstance(obj, list):
        return " ".join(obj)
    else:
        msg_err("Cannot convert %s into a string" % type(obj))
        return None

def join_to_list(obj, list_out: list):
    '''Append string or list another list.'''
    if isinstance(obj, str):
        list_out.append(obj)
    elif isinstance(obj, list):
        list_out+=obj
    else:
        msg_err("Cannot convert %s into a string" % type(obj))
        return None

def healthy_structure(dic_full: dict): # pylint: disable=too-many-return-statements, too-many-branches
    '''Check correct structure of the database.'''
    if not isinstance(dic_full, dict):
        msg_err("No dictionary found.")
        return False

    # Check mandatory database keys.
    good = True
    for key in ["workflows", "options"]:
        if key not in dic_full:
            msg_err("Key \"%s\" not found in the database." % key)
            good = False
    if not good:
        return False

    # Check the options database.
    dic_opt = dic_full["options"]
    if not isinstance(dic_opt, dict):
        msg_err("\"options\" is not a dictionary.")
        return False
    # Check mandatory option keys.
    good = True
    for key in ["global", "local"]:
        if key not in dic_opt:
            msg_err("Key \"%s\" not found in the option database." % key)
            good = False
    if not good:
        return False

    # Check the workflow database.
    dic_wf = dic_full["workflows"]
    if not isinstance(dic_wf, dict):
        msg_err("\"workflows\" is not a dictionary.")
        return False
    # Check mandatory workflow keys.
    for wf in dic_wf:
        dic_wf_single = dic_wf[wf]
        if not isinstance(dic_wf_single, dict):
            msg_err("%s is not a dictionary." % wf)
            return False
        good = True
        for key in ["activate"]:
            if key not in dic_wf_single:
                msg_err("Key \"%s\" not found in workflow %s." % (key, wf))
                good = False
        if not good:
            return False
        if not isinstance(dic_wf_single["activate"], bool):
            msg_err("\"activate\" in workflow %s is not a boolean." % wf)
            return False
    return True

def activate_workflow(wf: str, dic_wf: dict, mc=False, level=0, debug=False):
    '''Activate a workflows and its dependencies.'''
    if debug:
        eprint("  " + level * "  " + wf)
    if wf in dic_wf:
        dic_wf_single = dic_wf[wf]
        # Deactivate workflow if it needs MC and input is not MC.
        if "requires_mc" in dic_wf_single and dic_wf_single["requires_mc"] and not mc:
            msg_warn("Deactivated %s because of non-MC input" % wf)
            # Throw error if this is a dependency.
            if level > 0:
                msg_err("Workflows requiring this dependency would fail!")
                sys.exit(1)
            dic_wf_single["activate"] = False
            return
        # Activate.
        if not dic_wf_single["activate"]:
            dic_wf_single["activate"] = True
        # Activate dependencies recursively.
        if "dependencies" in dic_wf_single:
            list_dep=[]
            join_to_list(dic_wf_single["dependencies"], list_dep)
            for wf_dep in list_dep:
                activate_workflow(wf_dep, dic_wf, mc, level + 1, debug)
    else:
        # Add in the dictionary if not present.
        msg_warn("Adding an unknown workflow %s" % wf)
        dic_wf[wf] = {"activate": True}

def main():
    '''Main function'''
    parser = argparse.ArgumentParser(description="Generates full O2 command based on a YAML " \
                                                 "database of workflows and options.")
    parser.add_argument("input", help="database with workflows")
    parser.add_argument("-o", dest="output", help="output file")
    parser.add_argument("-g", "--graph", action="store_true", help="Make topology graph.")
    parser.add_argument("-d", "--debug", action="store_true", help="Print debugging info.")
    parser.add_argument("-w", "--workflows", type=str, help="explicit workflows")
    parser.add_argument("--mc", action="store_true", help="Monte Carlo mode")
    parser.add_argument("-t", "--tables", action="store_true", help="Save table into trees")
    args = parser.parse_args()
    file_input = args.input
    debug = args.debug
    workflows_add = args.workflows.split() if args.workflows else ""
    mc_mode = args.mc
    save_tables = args.tables

    if debug:
        eprint("Input: " + file_input)
    if mc_mode:
        msg_warn("MC mode is on")
    if save_tables:
        msg_warn("Tables will be saved in trees.")

    # Open database input file.
    try:
        with open(args.input, 'r') as file_in:
            dic_in = yaml.safe_load(file_in)
    except IOError:
        msg_err("Failed to open file " + args.input)
        sys.exit(1)

    # Check valid structure of the input database.
    if not healthy_structure(dic_in):
        msg_err("Bad structure.")
        sys.exit(1)

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
    list_wf_activated = [wf for wf in dic_wf if dic_wf[wf]["activate"]]
    if debug and list_wf_activated:
        eprint("\nWorkflows activated in the database:")
        eprint("\n".join("  " + wf for wf in list_wf_activated))
    # requested at command line
    if workflows_add:
        if debug:
            eprint("\nWorkflows specified at command line:")
            eprint("\n".join("  " + wf for wf in workflows_add))
        list_wf_activated+=workflows_add
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
        tables=[] # list of all tables of activated workflows
        for wf, dic_wf_single in dic_wf.items():
            if not dic_wf_single["activate"]:
                continue
            if "tables" not in dic_wf_single:
                continue
            tab_wf = dic_wf_single["tables"]
            if isinstance(tab_wf, (str, list)):
                join_to_list(tab_wf, tables)
            elif isinstance(tab_wf, dict):
                if "default" in tab_wf:
                    join_to_list(tab_wf["default"], tables)
                if mc_mode and "mc" in tab_wf:
                    join_to_list(tab_wf["mc"], tables)
            else:
                msg_err("\"tables\" in %s must be str, list or dict, is %s" % (wf, type(tab_wf)))
                sys.exit(1)
        str_before="AOD/"
        str_after="/0"
        string_tables = ",".join(str_before + t + str_after for t in tables)
        if string_tables:
            opt_local+=(" --aod-writer-keep " + string_tables)

    # Compose the full command with all options.
    command = ""
    eprint("\nActivated workflows:")
    for wf, dic_wf_single in dic_wf.items():
        if not dic_wf_single["activate"]:
            continue
        msg_bold("  " + wf)
        string_wf = wf
        # Process options
        if "options" in dic_wf_single:
            opt_wf = dic_wf_single["options"]
            if isinstance(opt_wf, (str, list)):
                string_wf+=(" " + join_strings(opt_wf))
            elif isinstance(opt_wf, dict):
                if "default" in opt_wf:
                    string_wf+=(" " + join_strings(opt_wf["default"]))
                if mc_mode and "mc" in opt_wf:
                    string_wf+=(" " + join_strings(opt_wf["mc"]))
            else:
                msg_err("\"options\" in %s must be str, list or dict, is %s" % (wf, type(opt_wf)))
                sys.exit(1)
        if opt_local:
            string_wf+=(" " + opt_local)
        command+=(" | " + string_wf)
    if not command:
        msg_err("Nothing to do!")
        sys.exit(1)
    # Remove the leading " | ".
    command = command[3:]
    # Append global options.
    if opt_global:
        command+=(" " + opt_global)

    # Print out the command.
    print(command)

main()
