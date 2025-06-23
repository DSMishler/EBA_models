# DFS propagate up
# ARGS: 2
# 0: bufname (always)
# 1: bufname of inventory buffer

API = {
    "request": "NODEVIS",
    "args": {"style": "filled","fillcolor":"green"}}
self.send_message(API, "ROOT", None, None)

API = {
    "request": "READALL",
    "target": self.call_args[1]}
inventory_txt = self.node_interface(API)["response"]
inventory_dict = eval(inventory_txt)



# build personal_info here (we will store it and grab it later)
locks_and_bufs = inventory_dict["data"]["neighbor_locks_and_bufs"]

API = {
    "request": "INVOKE",
    "mode": "SYSCALL",
    "target": "ID",
    "call_args": []}
nodename = self.node_interface(API)["response"]

personal_info = {
    "me": nodename,
    "neighbors": list(locks_and_bufs.keys()),
    "children": {}}

for n in locks_and_bufs:
    val = locks_and_bufs[n]
    if type(val) is str:
        API = {
            "request": "READALL",
            "target": val}
        child_dict_txt = self.node_interface(API)["response"]
        personal_info["children"][n] = eval(child_dict_txt)
    else:
        # the value should be an int holding "2" or "0"
        # TODO could check if desired
        pass

# Write the personal info dict to a buffer
API = {
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": len(repr(personal_info)),
    "time": 50}
personal_info_buf = self.node_interface(API)["name"]
API = {
    "request": "OVERWRITE",
    "target": personal_info_buf,
    "length": len(repr(personal_info)),
    "payload": repr(personal_info)}
self.node_interface(API)


pi_info = inventory_dict["data"]["parent_invoke"]
if pi_info["am_root"]:
    # we are done. execute final code
    pi_info["API"]["call_args"].append(personal_info_buf)
    self.node_interface(pi_info["API"])
else:
    # we have to keep going and prepare to propagate upward
    # now request a buffer from the parent
    parent = inventory_dict["data"]["parent_invoke"]["who"]
    API = {
        "request": "BUFREQ",
        "mode": "ALLOC",
        "size": 100,
        "time": 50}
    resp_buf = self.node_interface(API)["name"]
    
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
        "call_args": [self.call_args[1], resp_buf, personal_info_buf]}
    self.node_interface(API)
