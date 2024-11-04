# This file contains ways for python files to interface with the
# EBA architecture, such as making system calls to write to an adjacent buffer.
# This will also handle the PICKUP and DROPOFF files for each function

import pickle

def assert_correct_args(args): # args is sys.argv
    assert len(args) == 2, f"exactly two args allowed"

def get_pickup_fname(args):
    return args[1]

def get_pickup_info(pickup_fname):
    pf = open(pickup_fname, "rb")
    pickup_info = pickle.load(pf)
    assert "dropoff" in pickup_info, "pickup file has no dropoff field"
    pf.close()
    return pickup_info

def init_dropoff_info():
    return {"terminate": False, "requests": {}}

# TODO: These are prety much the same function. Should we just make one function?
def prep_dropoff_file(dropoff_info, dropoff_fname):
    pf = open(dropoff_fname, "wb")
    pickle.dump(dropoff_info, pf)
    pf.close()

def prep_pickup_file(pickup_info, pickup_fname):
    pf = open(pickup_fname, "wb")
    pickle.dump(pickup_info, pf)
    pf.close()


def have_requested_already(pickup_info, request_name):
    if request_name not in pickup_info["responses"]:
        # Then we never asked for this.
        return False
    else:
        return True

def waiting_for_response(pickup_info, request_name):
    if pickup_info["responses"][request_name] is None:
        return True
    else:
        return False # the thing is ready

# Leave a buffer request in the dropoff and flag where it should go in pickup
def bufreq(dropoff_info, pickup_info, neighbor, space, request_name):
    dropoff_info["requests"][request_name] = {}
    dropoff_info["requests"][request_name]["request"] = "BUFREQ"
    dropoff_info["requests"][request_name]["neighbor"] = neighbor
    dropoff_info["requests"][request_name]["space"] = space
    pickup_info["responses"][request_name] = None
