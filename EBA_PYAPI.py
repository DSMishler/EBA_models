# This file contains ways for python files to interface with the
# EBA architecture, such as making system calls to write to an adjacent buffer.
# This will also handle the PICKUP and DROPOFF files for each function

import pickle


# Globals
pickup_fname = None
pickup_info = None
dropoff_fname = None
dropoff_info = None

def init_PYAPI(args): # args is sys.argv
    assert_correct_args(args)

    set_pickup_fname(args)
    set_pickup_info()
    set_dropoff_fname()
    init_dropoff_info()


def assert_correct_args(args): # args is sys.argv
    assert len(args) == 2, f"exactly two args allowed"

def set_pickup_fname(args):
    global pickup_fname
    pickup_fname = args[1]
    return args[1]

def set_pickup_info():
    global pickup_info
    pf = open(pickup_fname, "rb")
    pickup_info = pickle.load(pf)
    assert "dropoff" in pickup_info, "pickup file has no dropoff field"
    pf.close()
    return pickup_info

def set_dropoff_fname():
    global dropoff_fname
    dropoff_fname = pickup_info["dropoff"]
    return dropoff_fname

def init_dropoff_info():
    global dropoff_info
    dropoff_info = {"terminate": False, "requests": {}}
    return dropoff_info

def prep_dropoff_and_pickup_files():
    prep_dropoff_file()
    prep_pickup_file()

def prep_dropoff_file():
    pf = open(dropoff_fname, "wb")
    pickle.dump(dropoff_info, pf)
    pf.close()

def prep_pickup_file():
    pf = open(pickup_fname, "wb")
    pickle.dump(pickup_info, pf)
    pf.close()

def get_proc_state():
    return pickup_info["proc_state"]

def set_proc_state(new_state):
    pickup_info["proc_state"] = new_state

def get_proc_var(var):
    return pickup_info["proc_vars"][var]

def set_proc_var(var, value):
    pickup_info["proc_vars"][var] = value


def have_requested_already(request_name):
    if request_name not in pickup_info["responses"]:
        # Then we never asked for this.
        return False
    else:
        return True

def waiting_for_response(request_name):
    if pickup_info["responses"][request_name] is None:
        return True
    else:
        return False # the thing is ready

def set_terminate_flag(value):
    dropoff_info["terminate"] = value

# Leave a buffer request in the dropoff and flag where it should go in pickup
def bufreq(neighbor, size, local_name, tags, request_name):
    if tags is None:
        tags = {}
    dropoff_info["requests"][request_name] = {}
    dropoff_info["requests"][request_name]["request"] = "BUFREQ"
    dropoff_info["requests"][request_name]["neighbor"] = neighbor
    dropoff_info["requests"][request_name]["size"] = size
    dropoff_info["requests"][request_name]["local_name"] = local_name
    dropoff_info["requests"][request_name]["tags"] = tags.copy()
    # Technically, the copy of "tags" is guaranteed by the EBA infrastructure.
    pickup_info["responses"][request_name] = None

# Leave a write request in the dropoff and flag where it should go in pickup
def write(neighbor, tag, mode, length, payload, request_name, extra_keys=[]):
    dropoff_info["requests"][request_name] = {}
    dropoff_info["requests"][request_name]["request"] = "WRITE"
    dropoff_info["requests"][request_name]["neighbor"] = neighbor
    dropoff_info["requests"][request_name]["target"] = tag
    dropoff_info["requests"][request_name]["mode"] = mode
    dropoff_info["requests"][request_name]["length"] = length
    dropoff_info["requests"][request_name]["payload"] = payload
    dropoff_info["requests"][request_name]["extra_keys"] = extra_keys.copy()
    pickup_info["responses"][request_name] = None

# Leave an invoke request in the dropoff and flag where it should go in pickup
def invoke(neighbor, mode, mode_args, request_name):
    dropoff_info["requests"][request_name] = {}
    dropoff_info["requests"][request_name]["request"] = "INVOKE"
    dropoff_info["requests"][request_name]["neighbor"] = neighbor
    dropoff_info["requests"][request_name]["mode"] = mode
    dropoff_info["requests"][request_name]["mode_args"] = mode_args.copy()
    pickup_info["responses"][request_name] = None

# wrapper for invoking with pyexec
def invoke_pyexec(neighbor, tag, keys, request_name, extra_keys=[]):
    mode_args = {"target_name": tag,
        # NOTE: "keys" are keys given to the child process, NOT used in the call
        # Copying "keys" could be guaranteed by the EBA infrastructure.
        "keys_for_new_proc": keys.copy(),
        "extra_keys": extra_keys.copy()}
    invoke(neighbor, "PYEXEC", mode_args, request_name)

# wrapper for invoking basic system calls
def invoke_system(mode_args, request_name):
    invoke(None, "SYSTEM", mode_args, request_name)

def neighbors(request_name):
    invoke_system({"syscall_name": "NEIGHBORS", "syscall_args": {}}, request_name)

def id(request_name):
    invoke_system({"syscall_name": "ID", "syscall_args": {}}, request_name)

def mybuf(request_name):
    invoke_system({"syscall_name": "MYBUF", "syscall_args": {}}, request_name)

def read(target_buf, extra_keys, request_name):
    if extra_keys is None:
        extra_keys = []
    sa = {"target_name": target_buf, "keys": extra_keys}
    invoke_system({"syscall_name": "READ", "syscall_args":  sa},request_name)

def ls(extra_keys, request_name):
    if extra_keys is None:
        extra_keys = []
    sa = {"keys": extra_keys}
    invoke_system({"syscall_name": "LS", "syscall_args": sa}, request_name)

# For retrieving a response to a system call
def retrieve_response(request_name):
    return pickup_info["responses"][request_name]
