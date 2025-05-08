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

def state_slice_to_gv(vertices, messages, adj):
    nvertices = len(vertices)

    edges_dict = [[[] for i in range(nvertices)] for j in range(nvertices)]
    # only the top-right half will be filled

    def name_to_num(vertex):
        return list(vertices.keys()).index(vertex)
    def num_to_name(vnum):
        return list(vertices.keys())[vnum]

    # first, go through all the messages and fill in the edges dict with
    # the special edges
    for message in messages:
        s = message["sender"]
        r = message["recipient"]
        if s == "ROOT":
            continue # ignore messages from ROOT
        if r == "ROOT":
            # a message *to* root might ask us to change a vertex color
            if message["API"]["request"] == "NODEVIS":
                vertices[s] = message["API"]["args"]
            continue
        sn = name_to_num(s)
        rn = name_to_num(r)

        if sn == rn:
            print("error in vis: message sent to self??")
            print(message)
            continue

        edge_str = ""
        if sn > rn:
            edge_str += "dir=back"
            smaller_n = rn
            larger_n = sn
        else:
            edge_str += "dir=forward"
            smaller_n = sn
            larger_n = rn

        if message["color"] is not None:
            edge_str += f",color={message['color']}"

        edges_dict[smaller_n][larger_n].append(edge_str)

    # now, fill in the edges dict with the regular edges where no special
    # edge was found
    for i in range(nvertices):
        for j in range(i+1, nvertices):
            if len(edges_dict[i][j]) == 0 and adj[i][j] == 1:
                # then and edge goes here
                edges_dict[i][j].append("dir=none")


    # now build the whole graphviz str
    # preamble
    gv_str = ""
    gv_str += "// network test graph\n"
    gv_str += "digraph\n"
    gv_str += "{\n"

    for v in vertices:
        gv_str += f"\t{v} [label={v}"

        # check for special instructions in the vertex dict
        for key in vertices[v]:
            gv_str += f",{key}={vertices[v][key]}"

        gv_str += "]\n"

    for i in range(nvertices):
        vi = num_to_name(i)
        for j in range(i+1, nvertices):
            vj = num_to_name(j)
            for edge in edges_dict[i][j]:
                gv_str += f"\t{vi} -> {vj} [{edge}]\n"

    gv_str += "}"

    # changes may have been made to the vertices, but they are a mutable
    # so we will not return them
    return gv_str

def string_to_file(gv_str, fname=None):
    if fname is None:
        fname = "EBA_graphviz/test.dot"
    f = open(fname, "w")
    f.write(gv_str)
    f.close()

def all_to_gv(manager, tdir="EBA_graphviz/testrun/"):
    vertices = {node: {} for node in manager.nodes}
    # vertices is mutable and might be changed by future functions:
    # this is intentional
    adj = manager.adj_matrix()

    # now slice up the messages by major iteration
    max_major_iter = manager.all_messages[-1]["major"]

    sliced_messages = []
    for i in range(max_major_iter+1):
        iter_msgs = [msg for msg in manager.all_messages if msg["major"] == i]
        sliced_messages.append(iter_msgs)
    for i in range(max_major_iter+1):
        fname = f"sim{i:04}.dot"
        gv_str = state_slice_to_gv(vertices, sliced_messages[i], adj)
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
