# DFS propagate
# ARGS: 3 # 0: bufname (always)
# 1: bufname of inventory buffer
# 2: neighbor I will propagate to

# This script will only allocate all the needed buffers
# and the parent invoke

neighbor = self.call_args[2]

API = {
    "request": "READALL",
    "target": self.call_args[1]}

inventory = self.node_interface(API)["response"]
inventory_dict = eval(inventory)

new_inventory_dict = {}
new_inventory_dict["code"] = inventory_dict["code"].copy()
new_inventory_dict["data"] = {} # no data yet, first data here

API = {
    "request": "INVOKE",
    "mode": "SYSCALL",
    "target": "ID",
    "call_args": []}
nodename = self.node_interface(API)["response"]

new_inventory_dict["data"]["parent_invoke"] = {
    "who": nodename,
    "am_root": False,
    "API": {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": inventory_dict["code"]["dfs7_update_parent.py"],
        "call_args": [self.call_args[1], self.call_args[2]]}
        # later, the child will add as an arg where to find
        # their dropoff
    }

msg_iter = 0
for key in inventory_dict["code"]:
    msg_iter += 1
    if msg_iter < 3:
        col = "blue"
    else:
        col = "quiet"
    buf = inventory_dict["code"][key]
    API = {
        "request": "BUFREQ",
        "mode": "ALLOC",
        "size": 100,
        "time": 50}
    intermediate_name = self.node_interface(API)["name"]
    new_inventory_dict["code"][key] = intermediate_name

    API = {
        "request": "READALL",
        "target": buf}
    buflen = len(self.node_interface(API)["response"])

    API = {
        "request": "BUFREQ",
        "mode": "ALLOC",
        "size": buflen,
        "time": 50}
    self.send_message(API, neighbor, intermediate_name, col)

# make a home for the new inventory_dict
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
    "size": len(repr(new_inventory_dict))*2,
    "time": 50}
new_inventory_dict_buf = self.node_interface(API)["name"]
self.send_message(API, neighbor, iin, "blue")

API = {
    "request": "OVERWRITE",
    "target": new_inventory_dict_buf,
    "length": len(repr(new_inventory_dict)),
    "payload": repr(new_inventory_dict)}
self.node_interface(API)

next_buf = inventory_dict["code"]["dfs3_prop_spinlock.py"]
API = {
    "request": "INVOKE",
    "mode": "PYEXEC",
    "target": next_buf,
    "call_args": self.call_args[1:] + [new_inventory_dict_buf, iin]}
self.node_interface(API)
