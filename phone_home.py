import EBA_PYAPI as EBA
import sys

EBA.init_PYAPI(sys.argv)
proc_state = EBA.get_proc_state()


if proc_state == "BEGIN":
    EBA.bufreq(
        neighbor="n1",
        size=-1,
        local_name=None,
        tags={"demo_read_key": "demo_databuf.txt"},
        request_name="PROCVAR_n2_to_n1_bufreq")
    EBA.read("demo_databuf.txt", ["key1"], "PROCVAR_n2_read")
    EBA.set_proc_state("WRITE")
elif proc_state == "WRITE":
    text = EBA.retrieve_response("PROCVAR_n2_read")
    EBA.write(
        neighbor="n1",
        tag="demo_databuf.txt",
        mode="START",
        length=len(text),
        payload=text,
        extra_keys=["demo_read_key"],
        request_name="PROCVAR_n2_to_n1_write")
    EBA.set_proc_state("MSG")
elif proc_state == "MSG":
    if EBA.waiting_for_response("PROCVAR_n2_to_n1_write"):
        pass
    else:
        EBA.bufreq(
            neighbor="n1",
            size=-1,
            local_name=None,
            tags={"demo_read_key": "message_data_available"},
            request_name="PROCVAR_n2_to_n1_bufreq_2")
        EBA.set_proc_state("TERM")
elif proc_state == "TERM":
    EBA.set_terminate_flag(True)
else:
    print(f"error, unknown proc state {proc_state}")
EBA.prep_dropoff_and_pickup_files()
