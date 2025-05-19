# DFS propagate up write
# ARGS: 2
# 0: bufname (always)
# 1: bufname of inventory buffer
# 2: buffer that contains the name of the buffer we can write to
# 3: buffer for personal info dict


API = {
    "request": "READ",
    "target": self.call_args[1]}
inventory_txt = self.node_interface(API)["response"]
inventory_dict = eval(inventory_txt)


API = {
    "request": "READ",
    "target": self.call_args[2]}
buf_resp = self.node_interface(API)["response"]
target_buf = eval(buf_resp)["name"]


API = {
    "request": "READ",
    "target": self.call_args[3]}
personal_info_txt = self.node_interface(API)["response"]
personal_info = eval(personal_info_txt)

# now write and invoke to parent
pi_info = inventory_dict["data"]["parent_invoke"]
pi_info["API"]["call_args"].append(target_buf)
API = {
    "request": "WRITE",
    "mode": "START",
    "target": target_buf,
    "length": len(repr(personal_info)),
    "payload": repr(personal_info)}

self.send_message(API, pi_info["who"], None, "green")
self.send_message(pi_info["API"], pi_info["who"], None, "red")
