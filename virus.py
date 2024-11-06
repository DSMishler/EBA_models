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
myreq = "myreq"
readreq = "readreq"
if proc_state == "BEGIN":
    EBA.id(idreq)
    EBA.neighbors(nreq)
    EBA.mybuf(myreq)
    EBA.set_proc_state("ACK")
elif proc_state == "ACK":
    # technically this "if" is not necessary because system calls
    # will always respond in the current implementation (it's done before
    # control is handed somewhere else)
    if (EBA.waiting_for_response(idreq) or
            EBA.waiting_for_response(nreq) or
            EBA.waiting_for_response(myreq)):
        print("waiting for response in virus!")
        pass
    else:
        print("got response in baby proc!")
        print(f"my id?: {EBA.retrieve_response(idreq)}")
        print(f"neighbors?: {EBA.retrieve_response(nreq)}")
        print(f"my buffer?: {EBA.retrieve_response(myreq)}")
        EBA.read(EBA.retrieve_response(myreq), readreq)
        EBA.set_proc_state("MYCODE")
elif proc_state == "MYCODE":
    if EBA.waiting_for_response(readreq):
        print("waiting for response in virus!")
        pass
    else:
        mycode = EBA.retrieve_response(readreq)
        print("this is my code:")
        print(mycode)
        EBA.set_terminate_flag(True)
        EBA.set_proc_state("WEGOOD")
else:
    assert False, f"unknown proc_state {proc_state}"


EBA.prep_dropoff_and_pickup_files()
