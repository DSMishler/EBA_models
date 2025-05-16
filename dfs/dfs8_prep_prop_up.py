# DFS propagate up
# ARGS: 2
# 0: bufname (always)
# 1: bufname of inventory buffer

API = {
    "request": "NODEVIS",
    "args": {"style": "filled","fillcolor":"green"}}
self.send_message(API, "ROOT", None, None)

API = {
    "request": "READ",
    "target": self.call_args[1]}
inventory_txt = self.node_interface(API)["response"]
inventory_dict = eval(inventory_txt)



locks_and_bufs = inventory_dict["data"]["neighbor_locks_and_bufs"]

personal_info = {
    "neighbors": list(locks_and_bufs.keys()),
    "children": {}}

for n in locks_and_bufs:
    val = locks_and_bufs[n]
    if type(val) is str:
        API = {
            "request": "READ",
            "target": val}
        child_dict_txt = self.node_interface(API)["response"]
        personal_info["children"][n] = eval(child_dict_txt)
    else:
        # the value should be an int holding "2" or "0"
        # TODO could check if desired
        pass


API = {
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": 50,
    "time": 50}
resp_buf = self.node_interface(API)["name"]

# now request a buffer from the parent
parent = inventory_dict["data"]["parent_invoke"]["who"]
API = {
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": len(repr(personal_info)),
    "time": 50}
self.send_message(API, parent, resp_buf, "blue")

# now spinlock and wait for the buffer
API = {
    "request": "INVOKE",
    "mode": "PYEXEC",
    "target": inventory_dict["code"]["dfs9_prop_up_spinlock.py"],
    "call_args": [self.call_args[1], resp_buf]}
self.node_interface(API)
