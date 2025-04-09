import EBA_PYAPI as EBA
import sys

EBA.init_PYAPI(sys.argv)
proc_state = EBA.get_proc_state()

if proc_state == "BEGIN":
    EBA.bufreq(
        neighbor="n2",
        size=-1,
        local_name="code_here.py",
        tags=None,
        request_name="PROCVAR_n1_to_n2_bufreq")
    EBA.set_proc_state("WRITE")
elif proc_state == "WRITE":
    if EBA.waiting_for_response("PROCVAR_n1_to_n2_bufreq"):
        pass
    else:
        f = open("phone_home.py", "r")
        f_txt = f.read()
        f.close()
        EBA.write(
            neighbor="n2",
            tag="code_here.py",
            mode="START",
            length=len(f_txt),
            payload=f_txt,
            request_name="PROCVAR_n1_to_n2_write")
        EBA.set_proc_state("INVOKE")
elif proc_state == "INVOKE":
    if EBA.waiting_for_response("PROCVAR_n1_to_n2_write"):
        pass
    else:
        EBA.invoke_pyexec(
            neighbor="n2",
            tag="code_here.py",
            keys=[],
            request_name=f"PROCVAR_n1_to_n2_invoke",
            extra_keys=[])
        EBA.set_proc_state("CHECK")
elif proc_state == "CHECK":
    if EBA.waiting_for_response("PROCVAR_n1_to_n2_invoke"):
        pass
    else:
        EBA.set_proc_state("SPINLOCK")
elif proc_state == "SPINLOCK":
    EBA.ls(["demo_read_key"], "PROCVAR_n1_ls")
    EBA.set_proc_state("SPINLOCK_CHECK")
elif proc_state == "SPINLOCK_CHECK":
    resp = EBA.retrieve_response("PROCVAR_n1_ls")
    if "message_data_available" in resp:
        EBA.read("demo_databuf.txt", ["demo_read_key"], "PROCVAR_n1_read")
        EBA.set_proc_state("READ")
    else:
        EBA.set_proc_state("SPINLOCK")
        pass
elif proc_state == "READ":
    text = EBA.retrieve_response("PROCVAR_n1_read")
    print(text)
    EBA.set_terminate_flag(True)
else:
    print(f"error, unknown proc state {proc_state}")
EBA.prep_dropoff_and_pickup_files()
