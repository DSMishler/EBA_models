# DFS ping
# the first stage of DFS when on a new node
# gets neighbors and pings each with TESTANDSET
# Will pass that information to next stage
# ARGS: 2
# 0: bufname (always)
# 1: bufname of inventory buffer

# First, mark this node as taken
API = {
    "request": "NODEVIS",
    "args": {"style": "filled","fillcolor":"turquoise"}}
self.send_message(API, "ROOT", None, None)


API = {
    "request": "INVOKE",
    "mode": "SYSCALL",
    "target": "NEIGHBORS",
    "call_args": []}

neighbors = self.node_interface(API)["response"]

# For later, we'll need a buffer to check whether or not all
# neighbors have been accessed. To do this, we make a
# dictionary of responses and we'll pass that info to
# the spinlock

n_resp_dict = {n: 2 for n in neighbors}
API = {
    "request": "READALL",
    "target": self.call_args[1]}

inventory = self.node_interface(API)["response"]
inventory_dict = eval(inventory)

inventory_dict["data"]["neighbor_locks_and_bufs"] = n_resp_dict
API = {
    "request": "OVERWRITE",
    "target": self.call_args[1],
    "length": len(repr(inventory_dict)),
    "payload": repr(inventory_dict)}
self.node_interface(API)



for neighbor in neighbors:
    # for each neighbor, set up a buffer for whether
    # or not you got the lock and then ping that neighbor
    # to write to our buffer
    API = {
        "request": "BUFREQ",
        "mode": "ALLOC",
        "size": 15,
        "time": 50}

    lockbuf = self.node_interface(API)["name"]

    # write the reserved value of "2" in there for "waiting"
    API = {
        "request": "OVERWRITE",
        "target": lockbuf,
        "length": len(repr({"response":2})),
        "payload": repr({"response":2})}

    self.node_interface(API)

    # now we invoke a testandset
    API = {
        "request": "INVOKE",
        "mode": "TESTANDSET",
        "target": "SYNC_0.sys",
        "call_args": []}

    self.send_message(API, neighbor, lockbuf, "red")

    # and we invoke a program that will spinlock and wait
    # for the message
    # first, find the buffer
    next_buf = inventory_dict["code"]["dfs1_spinlock.py"]

    API = {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": next_buf,
        "call_args": [self.call_args[1], lockbuf, neighbor]}

    self.node_interface(API)
