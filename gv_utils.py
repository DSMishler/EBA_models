import os
import sys
import shutil

def refresh_directory(tdir="EBA_graphviz/testrun/"):
    if "EBA_graphviz" not in tdir: # safety
        print(f"refusing to remove directory {tdir}. I don't think it's safe")
        return
    try:
        shutil.rmtree(tdir)
    except FileNotFoundError:
        pass
    os.mkdir(tdir)

def state_to_gv(state_slice):
    just_sent = state_slice["recent_sends"]
    nodes = state_slice["nodes"]
    nvertices = len(nodes)
    # we shouldn't need vertex numbers, right?
    # Just the vertex names will be fine, but we're going to make the matrix
    # anyway
    name_to_num = {}
    for (i, key) in enumerate(nodes):
        name_to_num[key] = i
    which_edges = [[0 for i in range(nvertices)] for j in range(nvertices)]

    # Build adjacency matix
    for node_name in nodes:
        node = nodes[node_name]
        for neighbor in node["neighbors"]:
            which_edges[name_to_num[node_name]][name_to_num[neighbor]] = 1

    # Graph heading
    gv_str = ""
    gv_str += "// network test graph\n"
    gv_str += "digraph\n"
    gv_str += "{\n"

    # Add vertices to the graph
    for v in nodes:
        vnum = name_to_num[v]
        nprocs = len(nodes[v]["process_dict"])
        gv_str += f"\t{vnum} ["
        # now add special names or anything that is needed or added
        #
        # gv_str += f"label=\"{vnum}:{nprocs}\""
        gv_str += f"label=\"{v}:{nprocs}\""
        if nprocs > 0:
            gv_str += f",style=filled,fillcolor=turquoise"
        if nodes[v]["pos"] is not None:
            pos = nodes[v]["pos"]
            gv_str += f",pos=\"{pos[0]},{pos[1]}\""
        gv_str += "]\n"

    # now add special names or anything that is needed or added
    # special edges
    message_edges_added = [[0 for i in range(nvertices)] for j in range(nvertices)]
    for msg in just_sent:
        colorstr = msg["message"]["color"]
        if colorstr is None:
            print("warning: color for message shouldn't end up 'None'")
            print("defaulting to red.")
            print(f"message: {msg['message']}")
            colorstr = "black"
        i = name_to_num[msg["sender"]]
        j = name_to_num[msg["receiver"]]
        gv_str += f"\t{i} -> {j} ["
        gv_str += f"dir=forward,color={colorstr}"
        gv_str += "]\n"
        message_edges_added[i][j] += 1

    # Add default edges to graph
    for i in range(nvertices):
        for j in range(i+1, nvertices):
            if which_edges[i][j] + which_edges[j][i] == 1: # exclusive or
                print("warning: unimplemented one-directional edge in graphing")
                continue
            elif message_edges_added[i][j] > 0 or message_edges_added[j][i] > 0:
                pass # do not repeat the default edge if a message is already there
            elif which_edges[i][j] and which_edges[j][i]:
                gv_str += f"\t{i} -> {j} ["
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
    os.system(f"neato -Tsvg {fname} > {fbase}.svg")
    os.system(f"convert {fbase}.svg {fbase}.png")

def all_dot_to_png(tdir="EBA_graphviz/testrun/"):
    for fname in os.listdir(tdir):
        if fname[-4:] == ".dot":
            dot_to_png(tdir+"/"+fname)

def all_png_to_gif(tdir="EBA_graphviz/testrun/", output_name="out.gif"):
    os.system(f"convert -size 1080x1080 -delay 0 -loop 0 {tdir}/*.png {tdir}/{output_name}")
