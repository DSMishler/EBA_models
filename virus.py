# NOTE: VIRUS IS NOW OUT OF DATE since the API was changed for tags
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
        print(f"virus on node {EBA.retrieve_response(idreq)}")
        # print("got response in baby proc!")
        # print(f"my id?: {EBA.retrieve_response(idreq)}")
        # print(f"neighbors?: {EBA.retrieve_response(nreq)}")
        # print(f"my buffer?: {EBA.retrieve_response(myreq)}")
        EBA.read(EBA.retrieve_response(myreq), readreq)
        EBA.set_proc_state("PHASE0")
elif proc_state == "PHASE0":
    neighbors = EBA.retrieve_response(nreq)
    for neighbor in neighbors:
        EBA.bufreq(neighbor, -1, "bufreq_"+neighbor)
    EBA.set_proc_state("PHASE1")
elif proc_state == "PHASE1":
    neighbors = EBA.retrieve_response(nreq)
    thiscode = EBA.retrieve_response(readreq)
    all_responses = True
    for neighbor in neighbors:
        if EBA.waiting_for_response("bufreq_"+neighbor):
            print(f"still waiting on a buffer from node {neighbor}")
            all_responses = False
        else:
            bufresponse = EBA.retrieve_response("bufreq_"+neighbor)
            if bufresponse["response"] != "ACK":
                print("error in virus, someone refused me a buffer! Stop")
                all_responses = False
                EBA.set_terminate_flag(True)
            if bufresponse["size"] != -1:
                print("error in virus, someone refused me infinite space! Stop")
                all_responses = False
                EBA.set_terminate_flag(True)
    if all_responses:
        for neighbor in neighbors:
            bufresponse = EBA.retrieve_response("bufreq_"+neighbor)
            bufname = bufresponse["name"]
            wreq_name = "writereq_"+neighbor
            EBA.write(neighbor, bufname, "START", len(thiscode), thiscode, wreq_name)
        EBA.set_proc_state("PHASE2")
    else:
        pass
elif proc_state == "PHASE2":
    neighbors = EBA.retrieve_response(nreq)
    thiscode = EBA.retrieve_response(readreq)
    all_responses = True
    for neighbor in neighbors:
        if EBA.waiting_for_response("writereq_"+neighbor):
            print(f"still waiting on writing ack from node {neighbor}")
            all_responses = False
        else:
            writeresponse = EBA.retrieve_response("writereq_"+neighbor)
            if writeresponse != len(thiscode):
                print("error in virus, writing to {neighbor} failed! Stop")
                all_responses = False
                EBA.set_terminate_flag(True)
    if all_responses:
        for neighbor in neighbors:
            bufresponse = EBA.retrieve_response("bufreq_"+neighbor)
            bufname = bufresponse["name"]
            invoke_name = "invokereq_"+neighbor
            EBA.invoke(neighbor, bufname, "PYEXEC", invoke_name)
        EBA.set_proc_state("PHASE3")
elif proc_state == "PHASE3":
    neighbors = EBA.retrieve_response(nreq)
    all_responses = True
    for neighbor in neighbors:
        if EBA.waiting_for_response("invokereq_"+neighbor):
            print(f"still waiting on invoke ack from node {neighbor}")
            all_responses = False
        else:
            invokeresponse = EBA.retrieve_response("invokereq_"+neighbor)
            if invokeresponse != True:
                print("error in virus, invoke to {neighbor} failed! Stop")
                all_responses = False
                EBA.set_terminate_flag(True)
    if all_responses:
        EBA.set_proc_state("PHASE0")
        # now begin anew
else:
    assert False, f"unknown proc_state {proc_state}"


EBA.prep_dropoff_and_pickup_files()
