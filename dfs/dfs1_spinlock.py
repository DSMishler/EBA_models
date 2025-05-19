# DFS spinlock
# ARGS: 5
# 0: bufname (always)
# 1: bufname of inventory buffer
# 2: bufname of buffer I spinlock on
# 3: neighbor I am waiting on

lockbuf = self.call_args[2]

API = {
    "request": "READ",
    "target": lockbuf}

resp = self.node_interface(API)["response"]

code = eval(resp)["response"]

if code == 2:
    # then we are still waiting. Invoke self.
    API = {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": self.call_args[0],
        "call_args": self.call_args[1:]}
    self.node_interface(API)
else:
    API = {
        "request": "READ",
        "target": self.call_args[1]}

    inventory = self.node_interface(API)["response"]
    inventory_dict = eval(inventory)

    # update the inventory dict with our response
    n = self.call_args[3]
    inventory_dict["data"]["neighbor_locks_and_bufs"][n] = code
    API = {
        "request": "WRITE",
        "mode": "START",
        "target": self.call_args[1],
        "length": len(repr(inventory_dict)),
        "payload": repr(inventory_dict)}
    self.node_interface(API)

    if code == 0:
        # then we aren't waiting, but didn't get the lock.
        # Check whether or not we are a leaf: invoke the code
        # that checks
        next_buf = inventory_dict["code"]["dfs6_am_i_leaf.py"]
    elif code == 1:
        # then we indeed got our lock. We pass the buck
        # to dfs2_propagate
        next_buf = inventory_dict["code"]["dfs2_propagate.py"]
    else:
        print(f"uhhh, what? code={code}")
    API = {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": next_buf,
        "call_args": [self.call_args[1], self.call_args[3]]}
    self.node_interface(API)




