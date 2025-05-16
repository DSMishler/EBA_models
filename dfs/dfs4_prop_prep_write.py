# DFS propagate prepare write
# ARGS: 5
# 0: bufname (always)
# 1: bufname of inventory buffer
# 2: neighbor I will propagate to
# 3: bufname of in-progress inventory buffer for child
# 4: bufname of the response for the child's home for the
#    inventory buffer

new_buf_dict_buf = self.call_args[3]
API = {
    "request": "READ",
    "target": new_buf_dict_buf}
inventory = self.node_interface(API)["response"]
new_inventory_dict = eval(inventory)

# go through the bfs dict and instead of the entries
# being the buffers which contain the responses to earlier
# requests, make the entries simply those buffers on the
# target system.

for key in new_inventory_dict["code"]:
    buf = new_inventory_dict["code"][key]
    API = {
        "request": "READ",
        "target": buf}
    response = self.node_interface(API)["response"]
    child_buf_name = eval(response)["name"]
    new_inventory_dict["code"][key] = child_buf_name

iin = self.call_args[4]
API = {
    "request": "READ",
    "target": iin}
response = self.node_interface(API)["response"]
child_inventory_bufname = eval(response)["name"]



API = {
    "request": "WRITE",
    "mode": "START",
    "target": new_buf_dict_buf,
    "length": len(repr(new_inventory_dict)),
    "payload": repr(new_inventory_dict)}
self.node_interface(API)


API = {
    "request": "READ",
    "target": self.call_args[1]}
inventory = self.node_interface(API)["response"]
inventory_dict = eval(inventory)
API = {
    "request": "INVOKE",
    "mode": "PYEXEC",
    "target": inventory_dict["code"]["dfs5_prop_write.py"],
    "call_args": self.call_args[1:4] + [child_inventory_bufname]}
self.node_interface(API)
