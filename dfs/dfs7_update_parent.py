# DFS update parent
# ARGS: 4
# 0: bufname (always)
# 1: bufname of inventory buffer
# 2: name of neighbor I got info from
# 3: name of buffer to find their update in

API = {
    "request": "READ",
    "target": self.call_args[1]}
inventory_txt = self.node_interface(API)["response"]
inventory_dict = eval(inventory_txt)

locks_and_buffers = inventory_dict["data"]["neighbor_locks_and_bufs"]

n = self.call_args[2]
b = self.call_args[3]
locks_and_buffers[n] = b

API = {
    "request": "WRITE",
    "mode": "START",
    "target": self.call_args[1],
    "length": len(repr(inventory_dict)),
    "payload": repr(inventory_dict)}
self.node_interface(API)

all_done = True
for n in locks_and_buffers:
    val = locks_and_buffers[n]
    if type(val) is str:
        pass
    elif val == 1:
        # Still waiting on this neighbor
        all_done = False
        break
    else:
        pass

if all_done:
    API = {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": inventory_dict["code"]["dfs8_prep_prop_up"],
        "call_args": [self.call_args[1]]}
    self.node_interface(API)
