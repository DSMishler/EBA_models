import sys
import pickle
import EBA_PYAPI as EBA


EBA.assert_correct_args(sys.argv)
pickup_fname = EBA.get_pickup_fname(sys.argv)
pickup_info = EBA.get_pickup_info(pickup_fname)
dropoff_fname = pickup_info["dropoff"]

print("I am baby proc")

if pickup_info["proc_state"] == "BEGIN":
    # Then we do the initialization
    dropoff_info = {"message": "RAAAAAAAAHHHH", "terminate": False}
    pickup_info["proc_state"] = "STATE2"
elif pickup_info["proc_state"] == "STATE2":
    # Then terminate
    dropoff_info = {"message": "REEEEE", "terminate": True}
    pickup_info["proc_state"] = "???"
else:
    assert False, f"unknown proc_state {pickup_info['proc_state']}"



EBA.prep_dropoff_file(dropoff_info, dropoff_fname)
EBA.prep_pickup_file(pickup_info, pickup_fname)
