# Daniel Mishler
import EBA_Manager
import gv_utils

import threading
import time


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




def ship_message_to_manager(API, show_response=True):
    assert type(show_response) is bool
    if show_response == False:
        response_buffer = None
    elif show_response == True:
        response_buffer = "terminal"
    message = {
        "recipient": current_node,
        "sender": "ROOT",
        "API": API,
        "response_buffer": response_buffer,
        "color": None}
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
    allowed_whats = ["init", "load", "save", "pause", "resume", "exit"]
    if what == "init":
        # gv_utils.refresh_directory(tdir="EBA_graphviz/testrun/")
        manager = EBA_Manager.EBA_Manager(mode="init", threading_lock=Shell_Lock)
        t = threading.Thread(target=manager.run_continuously, args=[])
        t.daemon = True # thread dies if I exit or CTRL+C
        t.start()
    elif what == "load":
        print("sys load not implemented")
    elif what == "save":
        print("sys save not implemented")
    elif what == "pause":
        if shell_check_manager() == False:
            return
        manager.pause()
    elif what == "resume":
        if shell_check_manager() == False:
            return
        manager.resume()
    elif what == "exit":
        if shell_check_manager() == False:
            return
        manager.set_exit()
        manager = None
        global current_node
        current_node = None
    else:
        print(f"error: <what> must be one of: {allowed_whats}")

shell_dict["sys"] = {
    "function": shell_sys,
    "usage": "sys <function>",
    "required_nargs": 2,
    "comment": "load, init, or save a system.\n" +
               "'pause' and 'resume' stop operation.\n" +
               "'exit' to stop the thread gracefully"}

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

def shell_connect(usrin):
    if shell_check_manager() == False:
        return
    nodename1 = get_arg_if_exists(usrin, 1)
    nodename2 = get_arg_if_exists(usrin, 2)
    manager.connect(nodename1, nodename2)

shell_dict["connect"] = {
    "function": shell_connect,
    "usage": "connect <nodename1> <nodename2>",
    "required_nargs": 3,
    "comment": "connects two nodes in the system"}

def shell_disconnect(usrin):
    if shell_check_manager() == False:
        return
    nodename1 = get_arg_if_exists(usrin, 1)
    nodename2 = get_arg_if_exists(usrin, 2)
    manager.disconnect(nodename1, nodename2)

shell_dict["disconnect"] = {
    "function": shell_disconnect,
    "usage": "disconnect <nodename1> <nodename2>",
    "required_nargs": 3,
    "comment": "disconnects two nodes in the system"}

def shell_adj_matrix(usrin):
    if shell_check_manager() == False:
        return
    print(f"in order: {manager.nodes}")
    adj = manager.adj_matrix()
    for row in adj:
        print(row)

shell_dict["adj_matrix"] = {
    "function": shell_adj_matrix,
    "usage": "adj_matrix",
    "required_nargs": 1,
    "comment": "shows an adjacency matrix of the whole network"}


def shell_show(usrin):
    if shell_check_manager() == False:
        return
    mode = get_arg_if_exists(usrin, 1)
    if mode == "contents":
        show_contents = True
    else:
        show_contents = False
    if shell_check_current_node() == False:
        return
    manager.show_node(current_node, show_contents)

shell_dict["show"] = {
    "function": shell_show,
    "usage": "show <opt:mode>",
    "required_nargs": 1,
    "comment": "show contents of current node to shell."}

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
        "size": int(size),
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
    fname = get_arg_if_exists(usrin, 3)
    f = open(fname, "r")
    payload = f.read()
    f.close()
    length = len(payload)

    API = {
        "request": "WRITE",
        "mode": mode,
        "target": target,
        "length": length,
        "payload": payload}

    ship_message_to_manager(API)

shell_dict["write"] = {
    "function": shell_write,
    "usage": "write <mode> <target> <fname>",
    "required_nargs": 3,
    "comment": "call the primitive WRITE on the ssh-ed node on buffer `target`"}

def shell_invoke(usrin):
    if shell_check_manager() == False:
        return
    if shell_check_current_node() == False:
        return
    mode = get_arg_if_exists(usrin, 1)
    target = get_arg_if_exists(usrin, 2)
    call_args = []
    cur_idx = 3
    while True:
        next_arg = get_arg_if_exists(usrin, cur_idx)
        cur_idx += 1
        if next_arg is None:
            break
        else:
            call_args.append(next_arg)

    API = {
        "request": "INVOKE",
        "mode": mode,
        "target": target,
        "call_args": call_args}

    ship_message_to_manager(API)

shell_dict["invoke"] = {
    "function": shell_invoke,
    "usage": "invoke <mode> <target> ...",
    "required_nargs": 3,
    "comment": "call the primitive INVOKE on the ssh-ed node"}

def shell_load_file(usrin):
    if shell_check_manager() == False:
        return
    if shell_check_current_node() == False:
        return

    with Shell_Lock:
        f = open("manager_shell_pipe.txt", "w") # flush the pipe
        f.close()

    fname = get_arg_if_exists(usrin, 1)

    f = open(fname, "r")
    ftext = f.read()
    f.close()

    invoke = False
    size = len(ftext)

    get_arg = 2
    while True:
        arg = get_arg_if_exists(usrin, get_arg)
        if arg is None:
            break
        arg_pair = arg.split("=")
        if len(arg_pair) != 2:
            print(f"I don't understand arg {arg}. Usage: no spaces (like a=1)")
            continue
        if arg_pair[0] == "invoke":
            invoke = eval(arg_pair[1])
        elif arg_pair[0] == "size":
            if "x" in arg_pair[1]:
                mstr = arg_pair[1].replace("x","")
                mul = int(mstr)
                size *= mul
            else:
                size = int(arg_pair[1])
        else:
            print(f"I don't understand keyword {arg_pair[0]}")

        get_arg += 1

    shell_bufreq(f"SHELL {size} 600")

    def pop_first_line_from(fname):
        f = open(fname, "r")
        ftext = f.read()
        f.close()
        if len(ftext) == 0:
            first_line = None
        else:
            first_line = ftext.split('\n')[0]
            new_text = "\n".join(ftext.split('\n')[1:])
            f = open(fname, "w")
            f.write(new_text)
            f.close()
        return first_line

    while True:
        with Shell_Lock:
            rtext = pop_first_line_from("manager_shell_pipe.txt")
        if rtext is None:
            time.sleep(0.2)
        else:
            msg = eval(rtext)
            payload = eval(msg["API"]["payload"])
            # it's a write request as a response, so the payload is the response
            # to our earlier request. But we have to make sure we didn't get
            # responses from earlier shell loads' writes or invokes. Easy.
            # just make sure we get a bufreq."
            if "name" not in payload:
                pass
            else:
                bufname = payload["name"]
                break

    shell_write(f"SHELL START {bufname} {fname}")

    if invoke:
        shell_invoke(f"SHELL PYEXEC {bufname}")

    return bufname

shell_dict["load_file"] = {
    "function": shell_load_file,
    "usage": "load_file <file> <optargs (invoke=??, size=??)>",
    "required_nargs": 2,
    "comment": "using BUFREQ and WRITE, load file <file> onto the node. " +
               "Possibly also runs INVOKE"}


# TODO: Allow for different directory selection in all of the below
def shell_refresh_directory(usrin=None):
    with Shell_Lock:
        gv_utils.refresh_directory()

shell_dict["refresh_visdir"] = {
    "function": shell_refresh_directory,
    "usage": "refresh_directory",
    "required_nargs": 1,
    "comment": "refresh (delete/clean) the default graphvis directory"}

def shell_export_dot(usrin=None):
    if shell_check_manager() == False:
        return
    with Shell_Lock:
        gv_utils.all_to_gv(manager)

shell_dict["export_dot"] = {
    "function": shell_export_dot,
    "usage": "export_dot",
    "required_nargs": 1,
    "comment": "export the manager's recorded state to graphviz dot files"}

def shell_dot_to_png(usrin=None):
    # TODO: check that there are files? Or trust?
    with Shell_Lock:
        gv_utils.all_dot_to_png()

shell_dict["dot_to_png"] = {
    "function": shell_dot_to_png,
    "usage": "dot_to_png",
    "required_nargs": 1,
    "comment": "turn all graphviz dot files to pngs in default directory"}

def shell_png_to_gif(usrin=None):
    # TODO: check that there are files? Or trust?
    with Shell_Lock:
        gv_utils.all_png_to_gif()

shell_dict["png_to_gif"] = {
    "function": shell_png_to_gif,
    "usage": "png_to_gif",
    "required_nargs": 1,
    "comment": "turn all graphviz pngs in default directory into a single gif"}

def shell_export_to_gif(usrin):
    if shell_check_manager() == False:
        return
    shell_export_dot()
    shell_dot_to_png()
    shell_png_to_gif()

shell_dict["export_to_gif"] = {
    "function": shell_export_to_gif,
    "usage": "export_to_gif",
    "required_nargs": 1,
    "comment": "runs `export_to_dot`, then `dot_to_png`, then `png_to_gif`"}

def shell_major(usrin):
    if shell_check_manager() == False:
        return
    with Shell_Lock:
        print(f"major iteration: {manager.major_iteration}")

shell_dict["major"] = {
    "function": shell_major,
    "usage": "major",
    "required_nargs": 1,
    "comment": "(debugging:) tells what major iteration the manager is on"}

def shell_echo(usrin):
    next_arg = get_arg_if_exists(usrin, 1)
    if next_arg is None:
        return
    
    next_arg_idx = usrin.find(next_arg)
    print(usrin[next_arg_idx:])

shell_dict["echo"] = {
    "function": shell_echo,
    "usage": "echo ...",
    "required_nargs": 1,
    "comment": "repeats everything you say after 'echo'"}

def shell_exec(usrin):
    fname = get_arg_if_exists(usrin, 1)
    if fname is None:
        print("no file to exec")
        return
    
    f = open(fname, "r")
    ftext = f.read()
    f.close()
    exec(ftext)

shell_dict["exec"] = {
    "function": shell_exec,
    "usage": "exec <file>",
    "required_nargs": 2,
    "comment": "calls python's `exec` on the file passed"}

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
        return_msg = shell_dict[first_arg]["function"](usrin)
        if return_msg is not None:
            print(return_msg)
