import sys
import pickle
import EBA_PYAPI as EBA


EBA.assert_correct_args(sys.argv)
pickup_fname = EBA.get_pickup_fname(sys.argv)
pickup_info = EBA.get_pickup_info(pickup_fname)
dropoff_fname = pickup_info["dropoff"]
dropoff_info = EBA.init_dropoff_info()

if pickup_info["proc_state"] == "BEGIN":
    # Then we do the initialization
    dropoff_info["message"] = "RAAAAAAAAHHHH"
    pickup_info["proc_state"] = "STATE2"
elif pickup_info["proc_state"] == "STATE2":
    # Then terminate
    dropoff_info["message"] = "REEEEE"
    pickup_info["proc_state"] = "BUFREQTIME"
elif pickup_info["proc_state"] == "BUFREQTIME":
    req_name = "mybufreq"
    if not EBA.have_requested_already(pickup_info, req_name):
        # Then make a request
        EBA.bufreq(dropoff_info, pickup_info, "three", -1, req_name)
    if EBA.waiting_for_response(pickup_info, req_name):
        print("waiting for response in baby proc!")
        pass
    else:
        print("got response in baby proc!")
        print(pickup_info["responses"][req_name])
        dropoff_info["message"] = "LETS GOOOO"
        dropoff_info["terminate"] = True
        pickup_info["proc_state"] = "WEGOOD"
else:
    assert False, f"unknown proc_state {pickup_info['proc_state']}"



EBA.prep_dropoff_file(dropoff_info, dropoff_fname)
EBA.prep_pickup_file(pickup_info, pickup_fname)
