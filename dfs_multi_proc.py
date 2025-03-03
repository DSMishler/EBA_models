import EBA_PYAPI as EBA
import sys

################################################################################
# LOGIC
################################################################################
#                                  BEGIN                                       #
#                         (grab filename info, etc)                            #
#                                                                              #
#                                DFS_UPDATE                                    #
#                              (grab path info)                                #
#               (write a new whole path file if none existed)                  #
#                                                                              #
#                                DFS_PREP                                      #
#                       (choose next neighbor in dfs)                          #
#                      (alloc and write path on newnode)                       #
#                                                                              #
#                              DFS_PROPAGATE                                   #
#                    (alloc, write, and invoke on newnode)                     #
################################################################################

whole_path_bufname = "path.dfs"
whole_path_key = "my_whole_path"
bufname_this_code = "next_dfs_host"

EBA.init_PYAPI(sys.argv)
proc_state = EBA.get_proc_state()

if proc_state == "BEGIN": # Call all of the builtins then wait on process queue
    EBA.id("PROCVAR_idreq")
    EBA.neighbors("PROCVAR_nreq")
    EBA.mybuf("PROCVAR_myreq")
    EBA.ls([whole_path_key], "PROCVAR_lsreq")
    EBA.set_proc_state("DFS_READ_SELF")
elif proc_state == "DFS_READ_SELF":
    EBA.read(EBA.retrieve_response("PROCVAR_myreq"), [], "PROCVAR_readreq_this_code")

    ls_result = EBA.retrieve_response("PROCVAR_lsreq")
    if whole_path_bufname in ls_result:
        EBA.read(whole_path_bufname, [whole_path_key], "PROCVAR_readreq_old_path")
        EBA.set_proc_state("DFS_PREP")
    else:
        EBA.set_proc_state("DFS_I_AM_ROOT")
elif proc_state == "DFS_I_AM_ROOT":
    # DFS assuming I am root
    # Assumes success
    # Write my path to myself, and read it
    EBA.bufreq(
        neighbor=None,
        size=-1,
        local_name=None,
        tags={whole_path_key:whole_path_bufname},
        request_name="PROCVAR_if_root_bufreq")
    EBA.write(
        neighbor=None,
        tag=whole_path_bufname,
        mode="START",
        length=len([]),
        payload=[],
        request_name="PROCVAR_if_root_write",
        extra_keys=[whole_path_key])
    EBA.read(whole_path_bufname, [whole_path_key], "PROCVAR_readreq_old_path")
    # Establish that I have no parent
    EBA.bufreq(
        neighbor=None,
        size=-1,
        local_name=None,
        tags={whole_path_key:"parent.dfs"},
        request_name=f"PROCVAR_tell_self_I_am_root_bufreq")
    EBA.write(
        neighbor=None,
        tag="parent.dfs",
        mode="START",
        length=len("None.am.root"),
        payload="None.am.root",
        request_name=f"PROCVAR_tell_self_I_am_root_write",
        extra_keys=[whole_path_key])
    # Write my own code to myself so my children can invoke easily
    this_code = EBA.retrieve_response("PROCVAR_readreq_this_code")
    EBA.bufreq(
        neighbor=None,
        size=-1,
        local_name=bufname_this_code,
        tags={whole_path_key: bufname_this_code},
        request_name="PROCVAR_next_host_bufreq")
    EBA.write(
        neighbor=None,
        tag=bufname_this_code,
        mode="START",
        length=len(this_code),
        payload=this_code,
        request_name="PROCVAR_next_host_write",
        extra_keys=[])
    EBA.set_proc_state("DFS_PREP")

elif proc_state == "DFS_PREP":
    old_path = EBA.retrieve_response("PROCVAR_readreq_old_path")
    neighbors = EBA.retrieve_response("PROCVAR_nreq")
    my_nodename = EBA.retrieve_response("PROCVAR_idreq")

    # TODO: remove parent from this list of neighbors
    children_reporting_bufs = [f"{n}_to_{my_nodename}.dfsbuf" for n in neighbors]
    ls_result = EBA.retrieve_response("PROCVAR_lsreq")

    in_list = [c in ls_result for c in children_reporting_bufs]

    if len(set(in_list)) != 1:
        print("strange result: some buffers written previously, but not others.")

    if in_list[0] is True:
        # Then we need to return information to our parent node.
        EBA.set_proc_state("DFS_REPORT_UP")
    else:
        # Then we need to propagate downward to other nodes.
        for c in children_reporting_bufs:
            EBA.bufreq(
                neighbor=None,
                size=-1,
                local_name=c,
                tags={whole_path_key: c},
                request_name=c+".bufreq")
            EBA.write(
                neighbor=None,
                tag=c,
                mode="START",
                length=len("No info yet."),
                payload="No info yet.",
                request_name=c+".writereq",
                extra_keys=[whole_path_key])
        for n in neighbors:
            EBA.bufreq(
                neighbor=n,
                size=-1,
                local_name=bufname_this_code,
                tags={whole_path_key: bufname_this_code},
                request_name=f"PROCVAR_next_host_{n}_bufreq")
        EBA.set_proc_state("DFS_WAIT_FOR_CHILDREN_RESPONSES")
            
elif proc_state == "DFS_REPORT_UP":
    neighbors = EBA.retrieve_response("PROCVAR_nreq")
    my_nodename = EBA.retrieve_response("PROCVAR_idreq")


    children_reporting_bufs = [f"{n}_to_{my_nodename}.dfsbuf" for n in neighbors]

    for c in children_reporting_bufs:
        EBA.read(c, [whole_path_key], c+".read")

    EBA.set_proc_state("DFS_REPORT_UP_SECURE_LOCK")

elif proc_state == "DFS_REPORT_UP_SECURE_LOCK":
    neighbors = EBA.retrieve_response("PROCVAR_nreq")
    my_nodename = EBA.retrieve_response("PROCVAR_idreq")

    children_reporting_bufs = [f"{n}_to_{my_nodename}.dfsbuf" for n in neighbors]

    # assume we aren't waiting on any responses
    readreq_responses = [EBA.retrieve_response(c+".read") for c in children_reporting_bufs]
    if "No info yet." in readreq_responses:
        EBA.set_terminate_flag(True) # wait until another proc finishes this one
        # print(f"{my_nodename} terminating because info not ready yet")
    else:
        EBA.bufreq(
            neighbor=None,
            size=-1,
            local_name=None,
            tags={whole_path_key:"lock.dfs"},
            request_name=f"PROCVAR_secure_lock_bufreq")
        EBA.read("parent.dfs", [whole_path_key], "PROCVAR_parent_readreq")
        EBA.set_proc_state("DFS_REPORT_UP_SEND")

elif proc_state == "DFS_REPORT_UP_SEND":
    neighbors = EBA.retrieve_response("PROCVAR_nreq")
    my_nodename = EBA.retrieve_response("PROCVAR_idreq")

    # assume not waiting
    if EBA.retrieve_response("PROCVAR_secure_lock_bufreq") == "REJ":
        EBA.set_terminate_flag(True) # Someone else beat me here
        # print(f"{my_nodename} terminating because someone beat me here")
    else:
        my_dfsbuf_info = {my_nodename: neighbors}
        children_reporting_bufs = [f"{n}_to_{my_nodename}.dfsbuf" for n in neighbors]
        readreq_responses = [EBA.retrieve_response(c+".read") for c in children_reporting_bufs]
        full_responses = [r for r in readreq_responses if r != "Don't expect info."]
        full_responses.append(my_dfsbuf_info)
        full_dfsbuf_dict = {}
        for r in full_responses:
            full_dfsbuf_dict.update(r)
        parent_name = EBA.retrieve_response("PROCVAR_parent_readreq")

        if parent_name == "None.am.root":
            print(full_dfsbuf_dict)
            EBA.set_terminate_flag(True) # we are done

        else:
            EBA.write(
                neighbor=parent_name,
                tag=f"{my_nodename}_to_{parent_name}.dfsbuf",
                mode="START",
                length=len(full_dfsbuf_dict),
                payload=full_dfsbuf_dict,
                request_name=f"PROCVAR_{my_nodename}_to_{parent_name}.dfsbuf.up",
                extra_keys=[whole_path_key])

            EBA.invoke_pyexec(
                neighbor=parent_name,
                tag=bufname_this_code,
                keys=[],
                request_name=f"PROCVAR_next_host_{parent_name}_invoke",
                extra_keys=[whole_path_key])
        
            EBA.set_proc_var("PROCVAR_neighbors_sent_invokes", [parent_name])
            EBA.set_proc_state("DFS_ALL_IS_WELL")




elif proc_state == "DFS_WAIT_FOR_CHILDREN_RESPONSES":
    neighbors = EBA.retrieve_response("PROCVAR_nreq")
    my_nodename = EBA.retrieve_response("PROCVAR_idreq")
    resp_codes = [f"PROCVAR_next_host_{n}_bufreq" for n in neighbors]
    waiting_bufreq_responses = [EBA.waiting_for_response(r) for r in resp_codes]
    if True in waiting_bufreq_responses:
        pass
    else:
        bufreq_responses = [EBA.retrieve_response(r) for r in resp_codes]
        if "ACK" not in bufreq_responses:
            # Then everyone we talked to already knows the algorithm
            # and we should actually report back up!
            # Nobody ack-ed, so expect no info
            for n in neighbors:
                dfsbuf = f"{n}_to_{my_nodename}.dfsbuf"
                EBA.write(
                    neighbor=None,
                    tag=dfsbuf,
                    mode="START",
                    length=len("Don't expect info."),
                    payload="Don't expect info.",
                    request_name=dfsbuf+".writereq2",
                    extra_keys=[whole_path_key])

            EBA.set_proc_state("DFS_REPORT_UP")
        else:
            # we have secured at least one child node to propagate to
            EBA.set_proc_state("DFS_PROPAGATE_DOWN")

elif proc_state == "DFS_PROPAGATE_DOWN":
    old_path = EBA.retrieve_response("PROCVAR_readreq_old_path")
    neighbors = EBA.retrieve_response("PROCVAR_nreq")
    my_nodename = EBA.retrieve_response("PROCVAR_idreq")

    this_code = EBA.retrieve_response("PROCVAR_readreq_this_code")

    current_path = old_path
    current_path.append(my_nodename)
    
    waiting_bufreq_responses = [EBA.waiting_for_response(f"PROCVAR_next_host_{n}_bufreq") for n in neighbors]
    if True in waiting_bufreq_responses:
        pass
    else:
        neighbors_sent_invokes = []
        for n in neighbors:
            n_bufreq = f"PROCVAR_next_host_{n}_bufreq"
            n_bufreq_response = EBA.retrieve_response(n_bufreq)

            if n_bufreq_response == "REJ":
                # Assume someone else beat us there
                dfsbuf = f"{n}_to_{my_nodename}.dfsbuf"
                EBA.write(
                    neighbor=None,
                    tag=dfsbuf,
                    mode="START",
                    length=len("Don't expect info."),
                    payload="Don't expect info.",
                    request_name=dfsbuf+".writereq2",
                    extra_keys=[whole_path_key])
                continue


            # now write our path to that new buffer
            EBA.bufreq(
                neighbor=n,
                size=-1,
                local_name=None,
                tags={whole_path_key:whole_path_bufname},
                request_name=f"PROCVAR_next_pfile_{n}_bufreq")
            EBA.write(
                neighbor=n,
                tag=whole_path_bufname,
                mode="START",
                length=len(current_path),
                payload=current_path,
                request_name=f"PROCVAR_next_pfile_{n}_write",
                extra_keys=[whole_path_key])

            # Let it know that we are its parent
            EBA.bufreq(
                neighbor=n,
                size=-1,
                local_name=None,
                tags={whole_path_key:"parent.dfs"},
                request_name=f"PROCVAR_tell_{n}_I_am_parent_bufreq")
            EBA.write(
                neighbor=n,
                tag="parent.dfs",
                mode="START",
                length=len(my_nodename),
                payload=my_nodename,
                request_name=f"PROCVAR_tell_{n}_I_am_parent_write",
                extra_keys=[whole_path_key])

            # And invoke to it as well
            EBA.write(
                neighbor=n,
                tag=bufname_this_code,
                mode="START",
                length=len(this_code),
                payload=this_code,
                request_name=f"PROCVAR_next_host_{n}_write",
                extra_keys=[])
            EBA.invoke_pyexec(
                neighbor=n,
                tag=bufname_this_code,
                keys=[],
                request_name=f"PROCVAR_next_host_{n}_invoke",
                extra_keys=[])
            neighbors_sent_invokes.append(n)
        
        EBA.set_proc_var("PROCVAR_neighbors_sent_invokes", neighbors_sent_invokes)
        EBA.set_proc_state("DFS_ALL_IS_WELL")



elif proc_state == "DFS_ALL_IS_WELL":
    neighbors_sent_invokes = EBA.get_proc_var("PROCVAR_neighbors_sent_invokes")

    resp_codes = [f"PROCVAR_next_host_{n}_invoke" for n in neighbors_sent_invokes]
    waiting_for_responses = [EBA.waiting_for_response(r) for r in resp_codes]

    if True in waiting_for_responses:
        pass
    else:
        responses = [EBA.retrieve_response(r) for r in resp_codes]
        if False in responses:
            print("error in dfs! at least one response didn't get the response we expected!")
        else:
            pass
        EBA.set_terminate_flag(True)
        # print("all is well. terminating.")
else:
    assert False, f"unknown proc_state {proc_state}"


EBA.prep_dropoff_and_pickup_files()
