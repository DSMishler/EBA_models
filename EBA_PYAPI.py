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



# TODO: These are prety much the same function. Should we just make one function?
def prep_dropoff_file(dropoff_info, dropoff_fname):
    pf = open(dropoff_fname, "wb")
    pickle.dump(dropoff_info, pf)
    pf.close()

def prep_pickup_file(pickup_info, pickup_fname):
    pf = open(pickup_fname, "wb")
    pickle.dump(pickup_info, pf)
    pf.close()
