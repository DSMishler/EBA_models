# dfs close
# ARGS: 2
# 0: bufname (always)
# 1: bufname of inventory buffer
# 2: bufname of final node info


API = {
    "request": "READALL",
    "target": self.call_args[1]}
inventory_txt = self.node_interface(API)["response"]
inventory_dict = eval(inventory_txt)

API = {
    "request": "READALL",
    "target": self.call_args[2]}
final_info = self.node_interface(API)["response"]

children_of_dfs = eval(final_info)

print("dfs complete!")
global print_dict
def print_dict(d, indent):
    spaces = " " * indent
    print(f"{spaces}{d['me']}")
    for c in d["children"]:
        print_dict(d["children"][c], indent+4)

print_dict(children_of_dfs, 0)
