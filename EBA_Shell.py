# Daniel Mishler
import EBA_Manager

import threading

Shell_Lock = threading.Lock()

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
        try:
            f = open(fname, "r")
        except FileNotFoundError:
            print(f"no file of name '{fname}' found.")
            return
        newlines = f.read()
        f.close()

        for line in newlines.split('\n'):
            self.inputs.append(line)

def check_nargs(usrin, target_nargs):
    args = usrin.split()
    if len(args) < target_nargs:
        print(f"rejected. {target_nargs} args required.")
        return False
    else:
        return True

def get_arg_if_exists(usrin, which_arg):
    args = usrin.split()
    if len(args) <= which_arg:
        return None
    else:
        return args[which_arg]


in_queue = Input_Queue()
current_node = None
manager = None

shell_manager_help_string = """
error: you cannot issue that command yet.
First you must initialize an EBA system.
Try initializing a system with `sys load` or `sys init`.
for help, type 'help'
"""
shell_current_node_help_string = """
error: you cannot issue that command yet.
First you must SSH into an EBA node.
Try ssh-ing into a node with `ssh <nodename>`.
for help, type 'help'
"""

def shell_check_manager():
    if manager is None:
        print(shell_manager_help_string)
        return False
    else:
        return True


def shell_check_current_node():
    if current_node is None:
        return False
    else:
        return True




def ship_message_to_manager(API):
    message = {
        "recipient": current_node,
        "sender": "ROOT",
        "API": API,
        "response_buffer": None}
    manager.deliver_shell_message(repr(message))


shell_dict = {}


def shell_exit(usrin=None):
    print("terminating")
    exit()

shell_dict["exit"] = {
    "function": shell_exit,
    "usage": "exit",
    "required_nargs": 1,
    "comment": "exit PyEBA"}

def shell_help(usrin=None):
    print("shell functions")
    for shell_func in shell_dict:
        paren_str = shell_func+":"
        print(f"{paren_str:<15}{shell_dict[shell_func]['comment']}")

shell_dict["help"] = {
    "function": shell_help,
    "usage": "help",
    "required_nargs": 1,
    "comment": "display this message"}

def shell_loadcmds(usrin):
    fname = get_arg_if_exists(usrin, 1)
    in_queue.load(fname)

shell_dict["loadcmds"] = {
    "function": shell_loadcmds,
    "usage": "loadcmds <cmdfile>",
    "required_nargs": 2,
    "comment": "load a file full of shell commands and execute them"}

def shell_sys(usrin):
    global manager
    what = get_arg_if_exists(usrin, 1)
    allowed_whats = ["init", "load", "save"]
    if what == "init":
        # gv_utils.refresh_directory(tdir="EBA_graphviz/testrun/")
        manager = EBA_Manager.EBA_Manager(mode="init", threading_lock=Shell_Lock)
        t = threading.Thread(target=manager.run_continuously, args=[])
        t.daemon = True # dies if I exit or CTRL+C
        t.start()
    elif what == "load":
        print("sys load not implemented")
    elif what == "save":
        print("sys save not implemented")
    else:
        print(f"error: <what> must be one of: {allowed_whats}")

shell_dict["sys"] = {
    "function": shell_sys,
    "usage": "sys <function>",
    "required_nargs": 2,
    "comment": "load, init, or save a system"}

def shell_ssh(usrin):
    global current_node
    if shell_check_manager() == False:
        return
    nodename = get_arg_if_exists(usrin, 1)
    if nodename in manager.nodes:
        current_node = nodename
    else:
        print(f"not ssh-ing: no node named {nodename} exists")

shell_dict["ssh"] = {
    "function": shell_ssh,
    "usage": "ssh <nodename>",
    "required_nargs": 2,
    "comment": "Not implmented at the moment."}

def shell_newnode(usrin):
    if shell_check_manager() == False:
        return
    nodename = get_arg_if_exists(usrin, 1)
    manager.new_node(node_name=nodename)

shell_dict["newnode"] = {
    "function": shell_newnode,
    "usage": "newnode <nodename>",
    "required_nargs": 2,
    "comment": "add a new node to the EBA system"}

def shell_show(usrin):
    if shell_check_manager() == False:
        return
    nodename = get_arg_if_exists(usrin, 1)
    if nodename is not None:
        print("showing non-ssh-ed nodes is not implemented (yet)")
    else:
        if shell_check_current_node() == False:
            return
        manager.show_node(current_node)

shell_dict["show"] = {
    "function": shell_show,
    "usage": "show <opt:nodename> <opt:nodename2> ...",
    "required_nargs": 1,
    "comment": "show contents of node to shell. " +
               "If no args, show current node"}

def shell_bufreq(usrin):
    if shell_check_manager() == False:
        return
    if shell_check_current_node() == False:
        return
    size = get_arg_if_exists(usrin, 1)
    time = get_arg_if_exists(usrin, 2)

    API = {
        "request": "BUFREQ",
        "mode": "ALLOC",
        "size": size,
        "time": time}

    ship_message_to_manager(API)

shell_dict["bufreq"] = {
    "function": shell_bufreq,
    "usage": "bufreq <size> <time>",
    "required_nargs": 3,
    "comment": "call the primitive BUFREQ on the ssh-ed node"}

def shell_write(usrin):
    if shell_check_manager() == False:
        return
    if shell_check_current_node() == False:
        return
    mode = get_arg_if_exists(usrin, 1)
    target = get_arg_if_exists(usrin, 2)
    length = get_arg_if_exists(usrin, 3)
    payload = get_arg_if_exists(usrin, 4)

    API = {
        "request": "WRITE",
        "mode": mode,
        "target": target,
        "length": length,
        "payload": payload}

    ship_message_to_manager(API)

shell_dict["write"] = {
    "function": shell_write,
    "usage": "write <mode> <target> <length> <payload>",
    "required_nargs": 5,
    "comment": "call the primitive WRITE on the ssh-ed node on buffer `target`"}

def shell_invoke(usrin):
    if shell_check_manager() == False:
        return
    if shell_check_current_node() == False:
        return
    mode = get_arg_if_exists(usrin, 1)
    target = get_arg_if_exists(usrin, 2)

    API = {
        "request": "INVOKE",
        "mode": mode,
        "target": target}

    ship_message_to_manager(API)

shell_dict["invoke"] = {
    "function": shell_invoke,
    "usage": "invoke <mode> <target>",
    "required_nargs": 3,
    "comment": "call the primitive INVOKE on the ssh-ed node"}


while True:
    if current_node is not None:
        current_node_str = f" (on node {current_node})"
    else:
        current_node_str = ""
    usrin = in_queue.next(input_str = f"PyEBA3{(current_node_str)}: ")

    if len(usrin) == 0:
        continue

    first_arg = usrin.split()[0]

    if first_arg[0][0] == "#":
        continue

    if first_arg not in shell_dict:
        print(f"unknown function '{first_arg}'")
        shell_dict["help"]["function"]()
    elif check_nargs(usrin, shell_dict[first_arg]["required_nargs"]) == False:
        print(shell_dict[first_arg]["usage"])
    else:
        error_msg = shell_dict[first_arg]["function"](usrin)
        # allowed functionality to print function-specific error messages
        if error_msg is not None:
            print(shell_dict[first_arg][error_msg])
