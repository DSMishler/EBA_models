import os
import sys


def state_to_gv(state_slice):
    nvertices = len(state_slice)
    # we shouldn't need vertex numbers, right?
    # Just the vertex names will be fine, but we're going to make the matrix
    # anyway
    name_to_num = {}
    for (i, key) in enumerate(state_slice):
        name_to_num[key] = i
    which_edges = [[0 for i in range(nvertices)] for j in range(nvertices)]

    # Build adjacency matix
    for node_name in state_slice:
        node = state_slice[node_name]
        for neighbor in node["neighbors"]:
            which_edges[name_to_num[node_name]][name_to_num[neighbor]] = 1

    # Graph heading
    gv_str = ""
    gv_str += "// network test graph\n"
    gv_str += "digraph\n"
    gv_str += "{\n"

    # Add vertices to the graph
    for v in state_slice.keys():
        vnum = name_to_num[v]
        gv_str += f"\t{vnum} ["
        # now add special names or anything that is needed or added
        #
        gv_str += "]\n"

    # Add edges to graph
    for i in range(nvertices):
        for j in range(i+1, nvertices):
            if which_edges[i][j] + which_edges[j][i] == 1: # exclusive or
                print("warning: unimplemented one-directional edge in graphing")
                continue
            elif which_edges[i][j] and which_edges[j][i]:
                gv_str += f"\t{i} -> {j} ["
                # now add special names or anything that is needed or added
                if False:
                    # special edges
                    pass # Nothing for now
                else:
                    gv_str += "dir=none"
                gv_str += "]\n"
            else:
                pass # no edge




    gv_str += "}"

    return gv_str

def string_to_file(gv_str, fname=None):
    if fname is None:
        fname = "EBA_graphviz/test.dot"
    f = open(fname, "w")
    f.write(gv_str)
    f.close()

def all_timeslice_to_files(node_states, tdir="EBA_graphviz/testrun/"):
    max_timeslice = 9999
    for timeslice in node_states:
        if timeslice > max_timeslice:
            print("warning: too many timeslices.")
            print("Only doing ones no greater than {max_timeslice}")
            break
        fname = f"sim{timeslice:04}.dot"
        gv_str = state_to_gv(node_states[timeslice])
        string_to_file(gv_str, fname=tdir+"/"+fname)

def dot_to_png(fname):
    if fname[-4:] == ".dot":
        fbase = fname[:-4]
    os.system(f"dot -Tsvg fname > {fbase}.svg")
    os.system(f"convert {fbase}.svg {fbase}.png")

def all_dot_to_png(tdir="EBA_graphviz/testrun/"):
    for fname in os.listdir(tidr):
        if fname[-4:] == ".dot":
            dot_to_png(fname)

def all_png_to_gif(tdir="EBA_graphviz/testrun/", output_name="out.gif"):
    os.system(f"convert -size 1080x1080 -delay 0 -loop 0 {tdir}/*.png {tdir}/{output_name}")
