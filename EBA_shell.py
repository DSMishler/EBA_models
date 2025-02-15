import EBA_Node
import gv_utils

def check_nargs(usrin, target_nargs, usage_msg):
    args = usrin.split()
    if len(args) < target_nargs:
        print(f"rejected. {target_nargs} args required.")
        print(f"usage: {usage_msg}")
        return False
    else:
        return True

current_node = None
while True:
    try:
        current_node_str = f" (on node {current_node.name})"
    except AttributeError:
        current_node_str = ""
    usrin = input(f"PyEBA{(current_node_str)}: ")

    first_arg = usrin.split()[0]

    if first_arg in ["exit", "quit", "exit()", "off"]:
        print("terminating")
        exit()
    elif first_arg in ["newsys"]:
        gv_utils.refresh_directory(tdir="EBA_graphviz/testrun/")
        manager = EBA_Node.EBA_Manager(manager_mode="init")
    elif first_arg in ["newnode"]:
        if check_nargs(usrin, 2, "newnode <nodename>"):
            nodename = usrin.split()[1]
            manager.new_node(name=nodename)
        else:
            pass
    elif first_arg in ["show_manager"]:
        manager.show()
        print("not implemented")
    elif first_arg in ["show_node"]:
        if current_node is None:
            print("error: you currently aren't ssh-ed into a node.")
            print("ssh into a node first.")
        else:
            # TODO: add show_buffer_status as an optional arg
            EBA_Node.show_node_state(current_node.all_state(), indent=4)
    elif first_arg in ["ssh"]:
        if check_nargs(usrin, 2, "ssh <nodename>"):
            nodename = usrin.split()[1]
            current_node = manager.nodes[nodename]
        else:
            pass
    else:
        print(f"unkown first argument {first_arg}")
