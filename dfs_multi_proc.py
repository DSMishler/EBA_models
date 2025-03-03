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
        EBA.set_proc_state("DFS_UPDATE")
    else:
        EBA.set_proc_state("DFS_I_AM_ROOT")
elif proc_state == "DFS_I_AM_ROOT":
    # DFS assuming I am root
    EBA.bufreq(
        neighbor=EBA.retrieve_response("PROCVAR_idreq"),
        size=-1,
        local_name=None,
        tags={whole_path_key:whole_path_bufname},
        request_name="PROCVAR_if_root_bufreq")
    EBA.write(
        neighbor=EBA.retrieve_response("PROCVAR_idreq"),
        tag=whole_path_bufname,
        mode="START",
        length=len([]),
        payload=[],
        request_name="PROCVAR_if_root_write",
        extra_keys=[whole_path_key])
    EBA.read(whole_path_bufname, [whole_path_key], "PROCVAR_readreq_old_path")
    # Assumes success
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
        print("strange result: some buffers written, but not others.")

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
                length=len(None),
                payload=None,
                request_name=c+".writereq",
                extra_keys=[whole_path_key])
        for n in neighbors:
            EBA.bufreq(
                neighbor=n,
                size=-1,
                local_name=bufname_this_code,
                tags={whole_path_key: bufname_this_code},
                request_name=f"PROCVAR_next_host_{n}_bufreq")
        EBA.set_proc_state("DFS_PROPAGATE_DOWN")
            
elif proc_state == "DFS_REPORT_UP":
    EBA.set_terminate_flag(True)

elif proc_state == "DFS_PROPAGATE_DOWN":
    old_path = EBA.retrieve_response("PROCVAR_readreq_old_path")
    neighbors = EBA.retrieve_response("PROCVAR_nreq")
    my_nodename = EBA.retrieve_response("PROCVAR_idreq")

    current_path = old_path
    current_path.append(my_nodename)
    
    waiting_bufreq_responses = [EBA.waiting_for_response(f"PROCVAR_next_host_{n}_bufreq") for n in neighbors]
    if True in waiting_bufreq_responses:
        pass
    else:
    
        sent_invokes = []

        for n in neighbors:
            n_bufreq = f"PROCVAR_next_host_{n}_bufreq"
            n_bufreq_response = EBA.retrieve_response(n_bufreq)

            if n_bufreq_response

            # now write our path to that new buffer
            EBA.bufreq(
                neighbor=next_target,
                size=-1,
                local_name=None,
                tags={whole_path_key:whole_path_bufname},
                request_name=f"PROCVAR_next_pfile_{n}_bufreq")
            EBA.write(
                neighbor=next_target,
                tag=whole_path_bufname,
                mode="START",
                length=len(current_path),
                payload=current_path,
                request_name=f"PROCVAR_next_pfile_{n}_write",
                extra_keys=[whole_path_key])



"""
    current_path = old_path
    current_path.append(my_nodename)
    visited = list(set(current_path))
    unvisited_neighbors = [n for n in neighbors if n not in visited]

    # now find next target
    if len(unvisited_neighbors) == 0:
        # then we have to return to a parent
        me_first_in_path = current_path.index(EBA.retrieve_response("PROCVAR_idreq"))
        if me_first_in_path == 0:
            next_target = None # we are done
            target_is_parent = None
        else:
            next_target = current_path[me_first_in_path-1]
            target_is_parent = True
    else:
        # one of the unvisited neighbors is next
        next_target = unvisited_neighbors[0]
        target_is_parent = False


    if next_target is not None:
        if target_is_parent:
            pass # no need to allocate a new buffer
        else:
            EBA.bufreq(
                neighbor=next_target,
                size=-1,
                local_name=None,
                tags={whole_path_key:whole_path_bufname},
                request_name="PROCVAR_next_pfile_bufreq")

        # now write our path to that new buffer
        EBA.write(
            neighbor=next_target,
            tag=whole_path_bufname,
            mode="START",
            length=len(current_path),
            payload=current_path,
            request_name="PROCVAR_next_pfile_write",
            extra_keys=[whole_path_key])

    else:
        print("DFS is done!")
        print(f"path was {current_path}")
        EBA.set_terminate_flag(True)

    EBA.set_proc_state("DFS_PROPAGATE")
    EBA.set_proc_var("PROCVAR_next_host_name", next_target)
"""

elif proc_state == "DFS_PROPAGATE":
    next_target = EBA.get_proc_var("PROCVAR_next_host_name")
    this_code = EBA.retrieve_response("PROCVAR_readreq_this_code")
    EBA.bufreq(
        neighbor=next_target,
        size=-1,
        local_name=bufname_this_code,
        tags={whole_path_key: bufname_this_code},
        request_name="PROCVAR_next_host_bufreq")
    EBA.write(
        neighbor=next_target,
        tag=bufname_this_code,
        mode="START",
        length=len(this_code),
        payload=this_code,
        request_name="PROCVAR_next_host_write",
        extra_keys=[])
    EBA.invoke_pyexec(
        neighbor=next_target,
        tag=bufname_this_code,
        keys=[],
        request_name="PROCVAR_next_host_invoke",
        extra_keys=[])

    EBA.set_proc_state("DFS_ALL_IS_WELL")

elif proc_state == "DFS_ALL_IS_WELL":
    if EBA.waiting_for_response("PROCVAR_next_host_invoke"):
        pass
    else:
        resp = EBA.retrieve_response("PROCVAR_next_host_invoke")
        if resp == True:
            pass
        else:
            print("error in dfs! we didn't get the response we expected!")
        EBA.set_terminate_flag(True)

else:
    assert False, f"unknown proc_state {proc_state}"


EBA.prep_dropoff_and_pickup_files()
