# DFS propagate
# ARGS: 3
# 0: bufname (always)
# 1: bufname of inventory buffer
# 2: neighbor I will propagate to

# This script will only allocate all the needed buffers

neighbor = self.call_args[2]

API = {
    "request": "READ",
    "target": self.call_args[1]}

inventory = self.node_interface(API)["response"]
dfs_buf_dict = eval(inventory)

new_dfs_buf_dict = dfs_buf_dict.copy()

new_dfs_buf_dict["parent_invoke"] = {
    "who": self.call_args[0],
    "API": {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": "BUF_4", # TODO: get the name of the pickup
        "call_args": self.call_args[1]}
        # later, the child will add as an arg where to find
        # their dropoff
    }

for key in dfs_buf_dict:
    if key == "parent_invoke":
        continue
    buf = dfs_buf_dict[key]
    API = {
        "request": "BUFREQ",
        "mode": "ALLOC",
        "size": 100,
        "time": 50}
    intermediate_name = self.node_interface(API)["name"]
    new_dfs_buf_dict[key] = intermediate_name

    API = {
        "request": "READ",
        "target": buf}
    buflen = len(self.node_interface(API)["response"])

    API = {
        "request": "BUFREQ",
        "mode": "ALLOC",
        "size": buflen,
        "time": 50}
    self.send_message(API, neighbor, intermediate_name, "blue")

# make a home for the new dfs_buf_dict
# for us AND our neighbor
API = {
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": 100,
    "time": 50}
# iin = intermediate_inventory_name
iin = self.node_interface(API)["name"]
API = {
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": len(repr(new_dfs_buf_dict))*2,
    "time": 50}
new_buf_dict_buf = self.node_interface(API)["name"]
self.send_message(API, neighbor, iin, "blue")

API = {
    "request": "WRITE",
    "mode": "START",
    "target": new_buf_dict_buf,
    "length": len(repr(new_dfs_buf_dict)),
    "payload": repr(new_dfs_buf_dict)}
self.node_interface(API)

next_buf = dfs_buf_dict["dfs3_prop_spinlock.py"]
API = {
    "request": "INVOKE",
    "mode": "PYEXEC",
    "target": next_buf,
    "call_args": self.call_args[1:] + [new_buf_dict_buf, iin]}
self.node_interface(API)
