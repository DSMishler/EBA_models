# DFS propagate write (and invoke)
# ARGS: 5
# 0: bufname (always)
# 1: bufname of inventory buffer
# 2: neighbor I will propagate to
# 3: bufname of now-complete inventory buffer for child (parent's copy
# 4: bufname of inventory buffer to give to child


neighbor = self.call_args[2]

new_buf_dict_buf = self.call_args[3]
API = {
    "request": "READ",
    "target": new_buf_dict_buf}
inventory = self.node_interface(API)["response"]
new_inventory_dict = eval(inventory)

old_buf_dict_buf = self.call_args[1]
API = {
    "request": "READ",
    "target": old_buf_dict_buf}
inventory = self.node_interface(API)["response"]
old_inventory_dict = eval(inventory)

# copy all of our code from our bufs to the bufs
# in the child node

for key in old_inventory_dict["code"]:
    oldbuf = old_inventory_dict["code"][key]
    newbuf = new_inventory_dict["code"][key]

    API = {
        "request": "READ",
        "target": oldbuf}
    payload = self.node_interface(API)["response"]

    API = {
        "request": "WRITE",
        "mode": "START",
        "target": newbuf,
        "length": len(payload),
        "payload": payload}
    self.send_message(API, neighbor, None, "green")

# write the new dfs buf dict over as well
payload = repr(new_inventory_dict)
API = {
    "request": "WRITE",
    "mode": "START",
    "target": self.call_args[4],
    "length": len(payload),
    "payload": payload}
self.send_message(API, neighbor, None, "green")


# finally, send an invoke request to the child node to get
# started
API = {
    "request": "INVOKE",
    "mode": "PYEXEC",
    "target": new_inventory_dict["code"]["dfs0_ping.py"],
    "call_args": [self.call_args[4]]}
self.send_message(API, neighbor, None, "red")
