# DFS am I leaf?
# ARGS: 3
# 0: bufname (always)
# 1: bufname of inventory buffer
# 2: neighbor I am waiting on (just got response of "no" from)

# go into the response buffer and check. If everyone said no,
# and no one is still waiting, then it is up to us to realize
# that we are the leaf and propagate back up.

# NOTE: this relies on the code not being parallelizable.
#       specifically, no other code will read the response
#       buffer between when this code reads it and then writes

API = {
    "request": "READ",
    "target": self.call_args[1]}
inventory_txt = self.node_interface(API)["response"]
inventory_dict = eval(inventory_txt)

# we just had our lock refused

n_resp_dict = inventory_dict["data"]["neighbor_locks_and_bufs"]
all_locks_refused = True
already_propped = False

for n in n_resp_dict:
    resp = n_resp_dict[n]
    if resp != 0:
        all_locks_refused = False
    if resp == -1:
        already_propped = True

if all_locks_refused and not already_propped:
    # we need to invoke the code which starts propagation upward.
    next_buf = inventory_dict["code"]["dfs8_prep_prop_up.py"]
    API = {
        "request": "INVOKE",
        "mode": "PYEXEC",
        "target": next_buf,
        "call_args": self.call_args[1:2]}
    self.node_interface(API)
    
    # Now, to make sure no other node arrives here
    # and double invokes, we leave a flag.
    # It is possible that, without this flag, a node
    # may have multiple neighbors that it pinged and then
    # it got zero locks. Each time a lock is read,
    # it enters this region of code.
    # Surely the last lock which is written will yield all
    # locks showing they were not acquired. But it is possible
    # that earlier locks will see the same state by the time
    # they read!
    n_resp_dict = inventory_dict["data"]["neighbor_locks_and_bufs"]
    for n in n_resp_dict:
        n_resp_dict[n] = -1
    API = {
        "request": "WRITE",
        "mode": "START",
        "target": self.call_args[1],
        "length": len(repr(inventory_dict)),
        "payload": repr(inventory_dict)}
    self.node_interface(API)
else:
    # Then this node needs to wait on more resposnes.
    # Do nothing. Those waiting invokes are already spinlocking by now.
    pass
