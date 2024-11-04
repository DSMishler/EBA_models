import sys
import pickle
import EBA_PYAPI as EBA


EBA.init_PYAPI(sys.argv)

proc_state = EBA.get_proc_state()
if proc_state == "BEGIN":
    # Then we do the initialization
    EBA.dropoff_info["message"] = "RAAAAAAAAHHHH"
    EBA.set_proc_state("STATE2")
elif proc_state == "STATE2":
    # Then terminate
    EBA.dropoff_info["message"] = "REEEEE"
    EBA.set_proc_state("BUFREQTIME")
elif proc_state == "BUFREQTIME":
    req_name = "mybufreq"
    if not EBA.have_requested_already(req_name):
        # Then make a request
        EBA.bufreq("three", -1, req_name)
    if EBA.waiting_for_response(req_name):
        print("waiting for response in baby proc!")
        pass
    else:
        print("got response in baby proc!")
        print(EBA.pickup_info["responses"][req_name])
        EBA.dropoff_info["message"] = "LETS GOOOO"
        EBA.set_terminate_flag(True)
        EBA.set_proc_state("WEGOOD")
else:
    assert False, f"unknown proc_state {proc_state}"

EBA.prep_dropoff_and_pickup_files()
