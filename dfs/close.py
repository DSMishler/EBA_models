# dfs close
# ARGS: 2
# 0: bufname (always)
# 1: bufname of inventory buffer
# 2: bufname of final node info


API = {
    "request": "READ",
    "target": self.call_args[1]}
inventory_txt = self.node_interface(API)["response"]
inventory_dict = eval(inventory_txt)

API = {
    "request": "READ",
    "target": self.call_args[2]}
final_info = self.node_interface(API)["response"]

children_of_dfs = eval(final_info)

print("dfs complete!")
print(children_of_dfs)
