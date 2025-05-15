# DFS propagate spinlock
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
new_dfs_buf_dict = eval(inventory)


still_waiting = False

for key in new_dfs_buf_dict:
    if key == "parent_invoke":
        continue
    buf = new_dfs_buf_dict[key]
    API = {
        "request": "READ",
        "target": buf}
    response = self.node_interface(API)["response"]
    # now response is either nothing, or the response
    # to our bufreq
    if len(response) == 0:
        still_waiting = True
        break
    else:
        pass # keep checking

if not still_waiting:
    # also check on our response to the iin
    # iin = intermediate inventory name
    iin = self.call_args[4]
    API = {
        "request": "READ",
        "target": iin}
    response = self.node_interface(API)["response"]
    # now response is either nothing, or the response
    # to our bufreq
    if len(response) == 0:
        still_waiting = True
    else:
        pass


if still_waiting:
    API = {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": self.call_args[0],
        "call_args": self.call_args[1:]}
    self.node_interface(API)
else:
    # pass the buck to code which writes to the child node
    API = {
        "request": "READ",
        "target": self.call_args[1]}
    inventory = self.node_interface(API)["response"]
    dfs_buf_dict = eval(inventory)
    API = {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": dfs_buf_dict["dfs4_prop_prep_write.py"],
        "call_args": self.call_args[1:]}
    self.node_interface(API)
