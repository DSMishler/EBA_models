# DFS base/root
# This code is root-unique. It simply claims the lock
# on root

API = {
    "request": "INVOKE",
    "mode": "TESTANDSET",
    "target": "SYNC_0.sys",
    "call_args": []}

val = self.node_interface(API)["response"]

if val == 1:
    API = {
        "request": "NODEVIS",
        "args": {"style": "filled","fillcolor":"turquoise"}}
    self.send_message(API, "ROOT", None, None)
