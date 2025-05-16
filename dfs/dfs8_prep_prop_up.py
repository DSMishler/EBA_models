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

