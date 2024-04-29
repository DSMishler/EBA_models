import os
import sys

if len(sys.argv) < 2:
    print("usage: python3 convert_named_dot.py <dot file>")
    exit(0)

fname = sys.argv[1]

def run_with_fname(fname):
    if fname[-4:] == ".dot":
        fname = fname[:-4]
    os.system(f"dot -Tsvg {fname}.dot > {fname}.svg")
    os.system(f"convert {fname}.svg {fname}.png")

if fname == "all":
    for fname in os.listdir():
        if fname[-4:] == ".dot":
            run_with_fname(fname)
else:
    run_with_fname(fname)
