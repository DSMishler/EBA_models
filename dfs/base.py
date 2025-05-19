# DFS base/root
# This code is root-unique. It simply claims the lock
# on root
# ARGS: 2
# 0: bufname (always)
# 1: bufname of inventory buffer
# 2: bufname of final code

API = {
    "request": "INVOKE",
    "mode": "TESTANDSET",
    "target": "SYNC_0.sys",
    "call_args": []}

val = self.node_interface(API)["response"]

if val == 1:
    # now call the next stage
    # first, find the buffer
    API = {
        "request": "READ",
        "target": self.call_args[1]}

    inventory = self.node_interface(API)["response"]
    inventory_dict = eval(inventory)
    next_buf = inventory_dict["code"]["dfs0_ping.py"]

    API = {
        "request": "INVOKE",
        "mode": "SYSCALL",
        "target": "ID",
        "call_args": []}
    nodename = self.node_interface(API)["response"]
    inventory_dict["data"]["parent_invoke"] = {
        "who": nodename,
        "am_root": True,
        "API": {
            "request": "INVOKE",
            "mode": "PYEXEC",
            "target": self.call_args[2],
            "call_args": [self.call_args[1]]}
        }
    API = {
        "request": "WRITE",
        "mode": "START",
        "target": self.call_args[1],
        "length": len(repr(inventory_dict)),
        "payload": repr(inventory_dict)}
    self.node_interface(API)

    API = {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": next_buf,
        "call_args": [self.call_args[1]]}
    self.node_interface(API)

else:
    print("error: the lock was not available. Aborting DFS.")

