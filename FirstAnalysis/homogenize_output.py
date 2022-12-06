#!/usr/bin/env python3

"""
Script to split the directories of a file into several ones with the same structure, useful for ML processing
"""

import argparse
import os
import time
from multiprocessing import Pool

from ROOT import TFile

g_verbose = False
g_out_path = False
g_base_dir = False


def split_file(input_name):
    processing_time = time.time()
    print(" > Processing file", input_name)
    if g_verbose:
        print("Homogenizing file", f"'{input_name}'", "for ML processing")
    f = TFile(input_name, "READ")
    if g_verbose:
        f.ls()
    lk = f.GetListOfKeys()
    files_created = 0
    for i in lk:
        obj = f.Get(i.GetName())
        if obj.ClassName() == "TDirectoryFile":
            fout = input_name.replace(".root", f"_sub{files_created}.root")
            fout = os.path.join(g_out_path, fout.split("/")[-1])
            if os.path.isfile(fout):
                raise RuntimeError("File", fout, "already there!")
            fout = TFile(fout, "RECREATE")
            if not fout.IsOpen():
                raise RuntimeError("File", fout, "is not open!")
            if g_verbose:
                print("Creating omogenized file to", fout)
            files_created += 1
            fout.mkdir(g_base_dir + "0")
            fout.cd(g_base_dir + "0")
            for j in obj.GetListOfKeys():
                if g_verbose:
                    print("Writing", j.ClassName(), j)
                t = obj.Get(j.GetName())
                if t.ClassName() == "TTree":
                    t.CloneTree().Write()
                else:
                    t.Clone().Write()
            if g_verbose:
                fout.ls()
            fout.Close()
    print(
        " < Processed file",
        input_name,
        "split into",
        files_created,
        "files, in",
        time.time() - processing_time,
        "seconds",
    )


def main(input_files, verbose=True, base_dir="TF_", out_path="", jobs=20):
    global g_verbose
    g_verbose = verbose
    global g_out_path
    g_out_path = out_path
    global g_base_dir
    g_base_dir = base_dir

    print("Omogenizing", len(input_files), "files")
    processing_time = time.time()
    with Pool(jobs) as p:
        p.map(split_file, input_files)
    print("Done, in", time.time() - processing_time, "seconds")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Omogenizer for ML processing")
    parser.add_argument("input_files", type=str, nargs="+", help="Input files")
    parser.add_argument(
        "--base_dir",
        type=str,
        default="TF_",
        help="Name of the base directory, usually `TF_` or `DF_`",
    )
    parser.add_argument(
        "--out_dir", "-o", type=str, default="./tmp/", help="Name of the output path"
    )
    parser.add_argument(
        "--jobs", "-j", type=int, default=5, help="Number of parallel jobs"
    )
    parser.add_argument("-v", action="store_true", help="Verbose mode")
    args = parser.parse_args()
    main(args.input_files, verbose=args.v, base_dir=args.base_dir, jobs=args.jobs)
