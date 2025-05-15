# DFS spinlock
# ARGS: 3
# 0: bufname (always)
# 1: bufname of inventory buffer
# 2: bufname of buffer I spinlock on

print(f"SPINLOCK script with args {self.call_args}")

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
elif code == 0:
    # then we aren't waiting, but didn't get the lock.
    # No further invokes, let it die
    pass
elif code == 1:
    # then we indeed got our lock. We pass the buck
    # to dfs2_propagate
    API = {
        "request": "READ",
        "target": self.call_args[1]}

    inventory = self.node_interface(API)["response"]
    dfs_buf_dict = eval(inventory)
    next_buf = dfs_buf_dict["dfs2_propagate.py"]

    API = {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": next_buf,
        "call_args": [self.call_args[1]]}

    self.node_interface(API)
else:
    print(f"uhhh, what? code={code}")
