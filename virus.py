"""
pseudocode:
    - pick up any new information and jump to the appropriate section of code
    - send an api message to all neighbors to give a copy of this code
"""

import sys
import EBA_PYAPI as EBA

EBA.init_PYAPI(sys.argv)

proc_state = EBA.get_proc_state()
idreq = "idreq"
nreq = "nreq"
if proc_state == "BEGIN":
    EBA.id(idreq)
    EBA.neighbors(nreq)
    EBA.set_proc_state("ACK")
elif proc_state == "ACK":
    # technically this "if" is not necessary because system calls
    # will always respond in the current implementation (it's done before
    # control is handed somewhere else)
    if EBA.waiting_for_response(idreq) or EBA.waiting_for_response(nreq):
        print("waiting for response in virus!")
        pass
    else:
        print("got response in baby proc!")
        print(f"my id?: {EBA.retrieve_response(idreq)}")
        print(f"neighbors?: {EBA.retrieve_response(nreq)}")
        EBA.set_terminate_flag(True)
        EBA.set_proc_state("WEGOOD")
else:
    assert False, f"unknown proc_state {proc_state}"


EBA.prep_dropoff_and_pickup_files()
