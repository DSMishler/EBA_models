import EBA_Node
import gv_utils

class Input_Queue:
    def __init__(self):
        self.inputs = []
    def next(self, input_str=None):
        if len(self.inputs) == 0:
            retval = input(f"{input_str}")
        else:
            retval = self.inputs[0]
            self.inputs.remove(retval)
        return retval
    def load(self, fname):
        f = open(fname, "r")
        newlines = f.read()
        f.close()

        for line in newlines.split('\n'):
            self.inputs.append(line)

def check_nargs(usrin, target_nargs, usage_msg):
    args = usrin.split()
    if len(args) < target_nargs:
        print(f"rejected. {target_nargs} args required.")
        print(f"usage: {usage_msg}")
        return False
    else:
        return True

in_queue = Input_Queue()
current_node = None
while True:
    try:
        current_node_str = f" (on node {current_node.name})"
    except AttributeError:
        current_node_str = ""
    usrin = in_queue.next(input_str = f"PyEBA{(current_node_str)}: ")

    if len(usrin) == 0:
        continue

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
    elif first_arg in ["show"]:
        if check_nargs(usrin, 2, "show <what>"):
            what = usrin.split()[1]
            allowed_whats = ["manager", "node"]
            if what == "manager":
                manager.show()
            elif what == "node":
                if current_node is None:
                    print("error: you currently aren't ssh-ed into a node.")
                    print("ssh into a node first.")
                else:
                    # TODO: add show_buffer_status as an optional arg
                    EBA_Node.show_node_state(current_node.all_state(), indent=4)
            else:
                print(f"error: <what> must be one of: {allowed_whats}")
        else:
            pass
    elif first_arg in ["loadcmds"]: # TODO: later extend to load system as well
        if check_nargs(usrin, 2, "loadcmds <fname>"):
            fname = usrin.split()[1]
            in_queue.load(fname)
        else:
            pass
    elif first_arg in ["ssh"]:
        if check_nargs(usrin, 2, "ssh <nodename>"):
            nodename = usrin.split()[1]
            current_node = manager.nodes[nodename]
        else:
            pass
    else:
        print(f"unkown first argument {first_arg}")
