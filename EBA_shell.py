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
        gv_utils.refresh_directory(tdir="EBA_graphviz/testrun/")
        manager = EBA_Node.EBA_Manager(manager_mode="init")
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

def shell_newnode(usrin):
    if shell_check_manager() == False:
        return
    nodename = get_arg_if_exists(usrin, 1)
    manager.new_node(name=nodename)

shell_dict["newnode"] = {
    "function": shell_newnode,
    "usage": "newnode <nodename>",
    "required_nargs": 2,
    "comment": "add a new node to the EBA system"}

def shell_connect(usrin):
    if shell_check_manager() == False:
        return
    node1 = get_arg_if_exists(usrin, 1)
    node2 = get_arg_if_exists(usrin, 2)
    manager.connect(node1, node2)

shell_dict["connect"] = {
    "function": shell_connect,
    "usage": "connect <node1> <node2>",
    "required_nargs": 3,
    "comment": "connect two nodes in the the EBA system"}

def shell_show(usrin):
    if shell_check_manager() == False:
        return
    show_nodes = []
    while True:
        new_node = get_arg_if_exists(usrin, len(show_nodes)+1)
        if new_node is None:
            break
        else:
            show_nodes.append(new_node)

    if len(show_nodes) == 0:
        # TODO: make showing buffer contents optional?
        manager.show(show_buffer_contents=False)
    else:
        for nodename in show_nodes:
            node = manager.nodes[nodename]
            EBA_Node.show_node_state(node.all_state(), indent=4, show_buffer_contents=True)

shell_dict["show"] = {
    "function": shell_show,
    "usage": "show <opt:nodename> <opt:nodename2> ...",
    "required_nargs": 1,
    "comment": "show contents of node or system to shell. " +
               "If no args, then show entire system"}

def shell_ls(usrin):
    if shell_check_manager() == False:
        return
    which_nodename = get_arg_if_exists(usrin, 1)
    node = manager.nodes[which_nodename]
    ls_keys = []
    while True:
        new_key = get_arg_if_exists(usrin, len(ls_keys)+2)
        if new_key is None:
            break
        else:
            ls_keys.append(new_key)
    ls_result = node.syscall_ls(ls_keys)
    if len(ls_result) == 0:
        if len(ls_keys) == 0:
            print(f"nothing found on node {which_nodename}.")
            print(f"perhaps try using some keys!")
        else:
            print(f"nothing found on node {which_nodename}.")
            print(f"(used keys {ls_keys}.)")
    else:
        for res in ls_result:
            print(res)

shell_dict["ls"] = {
    "function": shell_ls,
    "usage": "show <nodename> <opt:key1> <opt:key2> ...",
    "required_nargs": 2,
    "comment": "Call 'ls' on a node with specific keys"}

def shell_ssh(usrin):
    print("not implemented (yet).")
"""
    elif first_arg in ["ssh"]:
        if check_nargs(usrin, 2, "ssh <nodename>"):
            nodename = usrin.split()[1]
            current_node = manager.nodes[nodename]
        else:
            pass
"""

shell_dict["ssh"] = {
    "function": shell_ssh,
    "usage": "ssh <nodename>",
    "required_nargs": 1,
    "comment": "Not implmented at the moment."}

def shell_buf_alloc(usrin):
    if shell_check_manager() == False:
        return
    which_nodename = get_arg_if_exists(usrin, 1)
    # TODO: fail when node not found
    node = manager.nodes[which_nodename]

    buf_tags_args = []
    while True:
        new_tag = get_arg_if_exists(usrin, len(buf_tags_args)+2)
        if new_tag is None:
            break
        else:
            buf_tags_args.append(new_tag)

    tags = {}
    for tag in buf_tags_args:
        tag_as_list = tag.split(":")
        tags[tag_as_list[0]] = tag_as_list[1]

    # TODO: reconsider this extreme inefficiency
    i = 0
    while True:
        sys_bufname = f"SYSBUF_{i}"
        if sys_bufname in node.buffers:
            continue
        break

    node.syscall_alloc_buffer(sys_bufname, node.name, -1, tags, local_name=None)
    # TODO: check return values

shell_dict["buf_alloc"] = {
    "function": shell_buf_alloc,
    "usage": "buf_alloc <nodename> <opt:key1:name1>",
    "required_nargs": 2,
    "comment": "Allocate a buffer on a node with tags. " +
        "tags are given in the form `key:name`. Buffers are reffered to by " +
        "any of their names as long as you have the key corresponding " +
        "to that name. Buffers of these kinds always have size -1."}

def shell_buf_write(usrin):
    if shell_check_manager() == False:
        return
    which_nodename = get_arg_if_exists(usrin, 1)
    # TODO: fail when node not found
    node = manager.nodes[which_nodename]
    which_bufname = get_arg_if_exists(usrin, 2)
    which_mode = get_arg_if_exists(usrin, 3)
    payload_fname = get_arg_if_exists(usrin, 4)

    extra_keys_args = []
    while True:
        new_key = get_arg_if_exists(usrin, len(extra_keys_args)+5)
        if new_key is None:
            break
        else:
            extra_keys_args.append(new_key)

    f = open(payload_fname, "r")
    payload = f.read()
    f.close()

    node.syscall_write_to_buffer(
        which_bufname,
        which_mode,
        payload,
        len(payload),
        extra_keys_args,
        process=None)

shell_dict["buf_write"] = {
    "function": shell_buf_write,
    "usage": "buf_write <nodename> <bufname> <mode> <payload> <opt:key1> ... ",
    "required_nargs": 5,
    "comment": "Write the contents of the file `payload` to `bufname` " +
        "on `nodename` in `mode` using the keys provided"}

def shell_buf_invoke(usrin):
    if shell_check_manager() == False:
        return
    which_nodename = get_arg_if_exists(usrin, 1)
    # TODO: fail when node not found
    node = manager.nodes[which_nodename]
    which_bufname = get_arg_if_exists(usrin, 2)
    which_mode = get_arg_if_exists(usrin, 3)

    extra_keys_args = []
    while True:
        new_key = get_arg_if_exists(usrin, len(extra_keys_args)+4)
        if new_key is None:
            break
        else:
            extra_keys_args.append(new_key)

    node.syscall_invoke_to_buffer(
        which_bufname,
        which_mode,
        extra_keys=extra_keys_args,
        spawning_message="Spawned by admin")

shell_dict["buf_invoke"] = {
    "function": shell_buf_invoke,
    "usage": "buf_invoke <nodename> <bufname> <mode> <opt:key1> ... ",
    "required_nargs": 4,
    "comment": "invoke the contents of the buffer `bufname` " +
        "on `nodename` in `mode` using the keys provided"}

def shell_run(usrin):
    if shell_check_manager() == False:
        return
    run_ops = get_arg_if_exists(usrin, 1)

    manager.run(terminate_at=run_ops)

shell_dict["run"] = {
    "function": shell_run,
    "usage": "run <opt:numops>",
    "required_nargs": 1,
    "comment": "run the manager for some number of operations. " +
        "If nothing is passed for numops, then the system default is used."}

# TODO: Allow for different directory selection in all of the below
def shell_export_dot(usrin=None):
    if shell_check_manager() == False:
        return
    gv_utils.all_timeslice_to_files(manager.system_state)

shell_dict["export_dot"] = {
    "function": shell_export_dot,
    "usage": "export_dot",
    "required_nargs": 1,
    "comment": "export the manager's recorded state to graphviz dot files"}

def shell_dot_to_png(usrin=None):
    # TODO: check that there are files? Or trust?
    gv_utils.all_dot_to_png()

shell_dict["dot_to_png"] = {
    "function": shell_dot_to_png,
    "usage": "dot_to_png",
    "required_nargs": 1,
    "comment": "turn all graphviz dot files to pngs in default directory"}

def shell_png_to_gif(usrin=None):
    # TODO: check that there are files? Or trust?
    gv_utils.all_png_to_gif()

shell_dict["png_to_gif"] = {
    "function": shell_png_to_gif,
    "usage": "png_to_gif",
    "required_nargs": 1,
    "comment": "turn all graphviz pngs in default directory into a gif"}

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

while True:
    try:
        current_node_str = f" (on node {current_node.name})"
    except AttributeError:
        current_node_str = ""
    usrin = in_queue.next(input_str = f"PyEBA{(current_node_str)}: ")

    if len(usrin) == 0:
        continue

    first_arg = usrin.split()[0]

    if first_arg[0] == "#":
        continue

    if first_arg not in shell_dict:
        print(f"unknown function '{first_arg}'")
        shell_dict["help"]["function"]()
    elif check_nargs(usrin, shell_dict[first_arg]["required_nargs"]) == False:
        print(shell_dict[first_arg]["usage"])
    else:
        error_msg = shell_dict[first_arg]["function"](usrin)
        if error_msg is not None:
            print(shell_dict[first_arg][error_msg])
