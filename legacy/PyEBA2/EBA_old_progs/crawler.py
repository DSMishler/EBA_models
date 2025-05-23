import sys
import random
import EBA_PYAPI as EBA

EBA.init_PYAPI(sys.argv)

proc_state = EBA.get_proc_state()
idreq = "idreq"
nreq = "nreq"
myreq = "myreq"
readreq = "readreq"
bufname_next_host = "bufname_next_host_name"
if proc_state == "BEGIN": # Call all of the builtins then wait on process queue
    EBA.id(idreq)
    EBA.neighbors(nreq)
    EBA.mybuf(myreq)
    EBA.set_proc_state("ACK")
    random_stuff = "".join([random.choice(["a","j","z"]) for i in range(20)])
    EBA.set_proc_var(bufname_next_host, "host_"+random_stuff)
elif proc_state == "ACK":
    # technically this "if" is not necessary because system calls
    # will always respond in the current implementation (it's done before
    # control is handed somewhere else)
    if (EBA.waiting_for_response(idreq) or
            EBA.waiting_for_response(nreq) or
            EBA.waiting_for_response(myreq)):
        print("waiting for response in crawler!")
        pass
    else:
        print(f"crawler on node {EBA.retrieve_response(idreq)}")
        # print("got response in baby proc!")
        # print(f"my id?: {EBA.retrieve_response(idreq)}")
        # print(f"neighbors?: {EBA.retrieve_response(nreq)}")
        # print(f"my buffer?: {EBA.retrieve_response(myreq)}")
        EBA.read(EBA.retrieve_response(myreq), None, readreq)
        EBA.set_proc_state("PHASE0")
elif proc_state == "PHASE0": # Identify which neighbor to host and bufreq
    neighbors = EBA.retrieve_response(nreq)
    bufname = EBA.get_proc_var(bufname_next_host)
    next_host = random.choice(neighbors)
    EBA.set_proc_var("next_host", next_host)
    EBA.bufreq(next_host, -1, bufname, None, "bufreq_"+next_host)
    EBA.set_proc_state("PHASE1")
elif proc_state == "PHASE1": # Ensure the bufreq worked, write the code forward
    next_host = EBA.get_proc_var("next_host")
    bufname = EBA.get_proc_var(bufname_next_host)
    thiscode = EBA.retrieve_response(readreq)
    bufresponse = EBA.retrieve_response("bufreq_"+next_host)
    if EBA.waiting_for_response("bufreq_"+next_host):
        print(f"still waiting on a buffer from node {next_host}")
    elif bufresponse["response"] != "ACK":
        print("error in crawler, someone refused me a buffer! Stop")
        EBA.set_terminate_flag(True)
    elif bufresponse["size"] != -1:
        print("error in crawler, someone refused me infinite space! Stop")
        EBA.set_terminate_flag(True)
    else:
        # No errors: good to move on
        wreq_name = "writereq_"+next_host
        EBA.write(next_host, bufname, "START", len(thiscode), thiscode, wreq_name, extra_keys=None)
        EBA.set_proc_state("PHASE2")
elif proc_state == "PHASE2": # Ensure the writing worked, propagate with invoke
    next_host = EBA.get_proc_var("next_host")
    bufname = EBA.get_proc_var(bufname_next_host)
    thiscode = EBA.retrieve_response(readreq)
    writeresponse = EBA.retrieve_response("writereq_"+next_host)
    bufresponse = EBA.retrieve_response("bufreq_"+next_host)
    if EBA.waiting_for_response("writereq_"+next_host):
        print(f"still waiting on writing ack from node {next_host}")
    elif writeresponse != len(thiscode):
        print("error in crawler, writing to {next_host} failed! Stop")
        all_responses = False
        EBA.set_terminate_flag(True)
    else:
        invoke_name = "invokereq_"+next_host
        EBA.invoke(next_host, bufname, "PYEXEC", [], invoke_name, extra_keys=None)
        EBA.set_proc_state("PHASE3")
elif proc_state == "PHASE3": # Acknowledge the crawler is propagated
    next_host = EBA.get_proc_var("next_host")
    invokeresponse = EBA.retrieve_response("invokereq_"+next_host)
    if EBA.waiting_for_response("invokereq_"+next_host):
        print(f"still waiting on invoke ack from node {next_host}")
        all_responses = False
    elif invokeresponse != True:
        print("error in crawler, invoke to {next_host} failed! Stop")
        all_responses = False
        EBA.set_terminate_flag(True)
    else:
        EBA.set_proc_state("PHASEZ")
        EBA.set_terminate_flag(True)
else:
    assert False, f"unknown proc_state {proc_state}"


EBA.prep_dropoff_and_pickup_files()
