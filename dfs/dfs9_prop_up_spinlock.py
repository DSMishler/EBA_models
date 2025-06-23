# DFS prop up spinlock
# ARGS: 2
# 0: bufname (always)
# 1: bufname of inventory buffer
# 2: buffer we spinlock waiting for a response from
# 3: buffer that has our personal info dict from last step


API = {
    "request": "READALL",
    "target": self.call_args[1]}
inventory_txt = self.node_interface(API)["response"]
inventory_dict = eval(inventory_txt)


API = {
    "request": "READALL",
    "target": self.call_args[2]}
buf_resp = self.node_interface(API)["response"]

if len(buf_resp) == 0:
    # then invoke self
    API = {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": self.call_args[0],
        "call_args": self.call_args[1:]}
    self.node_interface(API)
else:
    # then propagate forward
    API = {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": inventory_dict["code"]["dfs10_prop_up_write.py"],
        "call_args": self.call_args[1:]}
    self.node_interface(API)
