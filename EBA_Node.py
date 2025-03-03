# TODO
# - I realized that unique node names and also unique buffer names does not
#   guaranee unique node-buffer pairs. Example: nodes A and AAAA and buffers
#   AAAAA and AA. You can have node-buffer pairs A.AAAAA and AAAA.AA
#   (dots added). This means more logic (like dots) might be needed

# As processes are implemented, I currently have them running until termination.
# This is why on the code writing side, I have the code written to take frequent
# breaks. Since a process will run until termination, it also won't get any API
# call responses since it will hog all of the cycles. It is the process's
# responsibility to place its API calls, then get out of the way.

ROOT_STR = "root"
SELF_BUFNAME = "me.EBA"

import enum
import random
import copy
import os
import shutil
import pickle
import subprocess

import gv_utils
import EBA_Utils


# API Calls:
# BUFFER REQUEST
# buffer request, size (in bytes) (-1 is infinity), local_name, tags
# local_name, or local name, will be how the calling process refers the buffer later
# tags are key-value pairs that processes can later use to refer to the buffer
    # (tags are useful since they will be needed if you want other processes
    # to see the buffer)
# ex. BUFREQ -1 mylocalbuf {"secretkey": "DFS_checkpoint"}
# response will have "ACK" next along with the buffer tags requested and space
# ex. BUFREQ ACK mylocalbuf {"secretkey": "DFS_checkpoint"} 1000
# (you didn't get an infinite amount of space, just 1000 bytes)
# or REJ if rejected
# ex BUFREQ REJ
# and of course this is gonna be a dictionary in python.

# WRITE
# write request, buffer name, number of bytes to write,
# where to write (start or end), message to write (string)
# write to a known buffer
# ex. WRITE BUF1 START 11 "hello world"
# reponse is given with how many bytes were actually written
# ex. WRITE 11
# a failed write would look like
# ex. WRITE 0
# Jury's out on whether we want append. Currently only START is implemented

# INVOKE CODE
# invoke request, type of invoke, then args
# invoke what is in a buffer to be run as code
# ex. INVOKE PYEXEC BUF1 DFSKEY
# response will be given when the process is spawned
# ex. INVOKE True
# and the exit code will determine if there was an error
# ex. INVOKE False
# If a process wants data from invoke, code needs to already exist to phone home
# in the invoked code
# Invoke is also how we use system calls/operations
# ex. INVOKE SYSTEM NEIGHBORS
# would return a list of neighbors


# SYSTEM CALLS/OPERATIONS (not inter-node, only intra-node)
# NEIGHBORS/ID/MYBUF
# states which nodes connected to this node. This probably never needs to
# be used over the network but *does* need to be used for local system calls.
# Otherwise a distance routing process could never get implemented.
# ex. NEIGHBORS
# response is simply a list of neighbors
# ex. NEIGHBORS "one" "two" "three"
# ex. ID
# ex. ID "zero"

# READ
# list a buffer and receive the contents of that buffer
# ex. READ buf0
# response is the payload
# ex. READ "Hello world"

# LS
# return a list of what's on this buffer given your process keys and extra keys
# ex. LS "dfs"
# response is the list
# ex. LS "['mybuf', 'dfs_tree'], ['visited'], ['scratch']"



# message format:
# sender id
# recipient id
# sender request # (or recipient if responding)
# API call specifics
# which process it is waiting on (if any)

def blank_message_template():
    return {
        "sender": None,
        "recipient": None,
        "RID": None,
        "API": {
            "request": None,
            "response": None
            # It is possible more will be added here, depending on the API call
            },
        "process": None
        }

class EBA_State(enum.Enum):
    IDLE = 0 # Default state
    RESPOND = 1 # Responding to an interrupt. Important so that a node
                # doesn't run off responding to a chain of interrupts.
                # It must complete its response to one interrupt before
                # addressing another.


class EBA_Node:
    def __init__(self, name, manager):
        self.name = name
        self.interrupt_state = EBA_State.IDLE
        self.manager = manager
        # manager is for Python exchanges, so that no node
        # has access to the code of another
        self.neighbors = {}
        self.buffers = {}
        self.next_RID = 0
        self.waiting_requests = {}
        self.message_queue = []
        self.process_dict = {}
        self.next_PID = 0
        self.buf_counter = 0
        # Process dict info:
        # Process name (key, unique per node)
        #    name: just a copy of the process name, used for printing later
        #    keys: the unique keys that a process has for accessing buffers
        #    message: message that spawned the process
        #    bufname: buffer the process code lives in
        #    last_scheduled: number of times the scheduler did *not* choose
        #                    this process. Every time a scheduler chooses, all
        #                    processes have this number incremented by 1 except
        #                    the scheduled process, which has this set to 0
        self.fnames_for = {
                "all_state": "NODE_ALL_STATE.pkl",
                "interrupt_state": "NODE_INTERRUPT_STATE.pkl",
                "neighbors": "NODE_NEIGHBORS.pkl",
                "buffers": "NODE_BUFFERS.pkl",
                "waiting_requests": "NODE_WAITING_REQUESTS.pkl",
                "message_queue": "NODE_MESSAGE_QUEUE.pkl",
                "process_dict": "NODE_PROCESS_DICT.pkl"}

    def all_state(self):
        return {
                "name": self.name,
                "interrupt_state": self.interrupt_state,
                "neighbors": self.neighbors,
                "buffers": self.buffers,
                "waiting_requests": self.waiting_requests,
                "message_queue": self.message_queue,
                "process_dict": self.process_dict}

    def save_all_state(self, tdir):
        fb = open(tdir+"/"+self.fnames_for["all_state"], "wb")
        pickle.dump(self.all_state(), fb)
        fb.close()

    def load_all_state(self, tdir):
        fb = open(tdir+"/"+self.fnames_for["all_state"], "rb")
        all_state = pickle.load(fb)
        fb.close()

        self.name = all_state["name"]
        self.interrupt_state = all_state["interrupt_state"]
        self.neighbors = all_state["neighbors"]
        self.buffers = all_state["buffers"]
        self.waiting_requests = all_state["waiting_requests"]
        self.message_queue = all_state["message_queue"]
        self.process_dict = all_state["process_dict"]


    def message_template(self):
        message = blank_message_template()
        message["sender"] = self.name
        message["RID"] = self.name+str(self.next_RID)
        self.next_RID += 1
        return message

    def response_to_message(self, message):
        response = blank_message_template()
        response["sender"] = self.name
        response["recipient"] = message["sender"]
        response["RID"] = message["RID"]
        response["API"]["request"] = message["API"]["request"]
        response["process"] = message["process"]
        return response


    ############################################################################
    # BUFREQ                                                                   #
    ############################################################################

    # neighbor: the name of the neighbor this message is for
    # size: the requested size of the buffer
    # local_name: the name that the buffer will be locally called from
        # the perspective of the caller. This will be done transparently.
        # None is allowed.
    # tags: a dictionary of keys and names. Processes requesting to write
        # or read from this buffer will be required to have one of these keys
        # and processes calling "ls" on the node will only see buffers to which
        # they own keys
        # you are allowed to pass "None" to tags
    # process: the info of the process on this node which requests the buffer
        # process is allowed to be None, but this is not going to be implemented
        # as a default
    def request_buffer_from(self, neighbor, size, tags, local_name, process):
        message = self.message_template()
        message["recipient"] = neighbor
        message["API"]["request"] = "BUFREQ"
        message["API"]["size"] = size
        message["API"]["local_name"] = local_name
        message["API"]["tags"] = tags.copy()
        message["process"] = process
        
        # note that we are now awaiting a response
        self.waiting_requests[message["RID"]] = message

        # send the message
        self.manager.send(self.name, neighbor, message)

    # Now the neighbor, which received a buffer request,
    # will resolve it on its end. This neighbor will currently always
    # give a buffer of requested size, but this does not *have* to be this way
    def resolve_buffer_request(self, message):

        syscall_response = self.syscall_alloc_buffer(
            message["sender"],
            message["API"]["size"],
            message["API"]["tags"].copy(),
            message["API"]["local_name"],
            message["process"])

        # Send a message back to the sender
        response = self.response_to_message(message)
        for el in syscall_response:
            response["API"][el] = syscall_response[el]

        self.manager.send(self.name, message["sender"], response)


    # Now the original sender of the buffer request will acknowledge
    # that it has a buffer. It doesn't need to send a message again, just
    # needs to update its own data.
    def acknowledge_buffer(self, message):
        if message["API"]["response"] == "ACK":
            # TODO: the API should maybe tell me if I'm the only writer or not
            pass

        elif message["API"]["response"] == "REJ":
            pass
        else:
            print(f"unknown API response {message['API']['response']}")
            exit(1)

        return message["API"]["response"]


    ############################################################################
    # WRITE                                                                    #
    ############################################################################
    # neighbor: the name of the neighbor this message is for
    # tag: tag of the target buffer (given after filtering by process keys)
    # mode: START or APPEND (only start implemented right now)
    # length: length of payload
    # payload: the content that will be written to the buffer
    # process: the info of the process on this node which requests the buffer
        # You can get away with process=None as long as there are extra keys
    # extra_keys: in addition to process keys, other keys may be used. Often,
        # this is because a process uniquely requests buffers with this key
        # and then uses it later.

    def request_write_to_buffer(self, neighbor, tag, mode, length, payload, process, extra_keys=[]):
        assert length != 0, f"illegal request: write with length of 0"
        assert length == len(payload), f"lllegal request: payload {payload} claimed to have length {length} instead of {len(payload)}"

        message = self.message_template()
        message["recipient"] = neighbor
        message["API"]["request"] = "WRITE"
        message["API"]["target"] = tag
        message["API"]["mode"] = mode
        message["API"]["length"] = length
        message["API"]["payload"] = payload
        message["process"] = process
        message["extra_keys"] = extra_keys.copy()

        self.waiting_requests[message["RID"]] = message
        self.manager.send(self.name, neighbor, message)


    # Now the neighbor, which received a write request,
    # will resolve it on its end. This neighbor will currently always
    # successfully write, but this does not *have* to be this way
    def resolve_write_request(self, message):
        syscall_response = self.syscall_write_to_buffer(
            message["API"]["target"],
            message["API"]["mode"],
            message["API"]["length"],
            message["API"]["payload"],
            message["process"],
            message["extra_keys"])

        response = self.response_to_message(message)
        for el in syscall_response:
            response["API"][el] = syscall_response[el]

        self.manager.send(self.name, message["sender"], response)

    # Now the original sender of the write request will acknowledge
    # that it has, indeed, written what it wanted (or not).
    # It doesn't need to send a message again, just
    # needs to update its own data.
    def acknowledge_write_request(self, message, expected_len):
        if message["API"]["response"] == expected_len:
            # then the write was successful
            # Nothing needs done here. If a process needed to know,
            # then the wrapper will let it know
            pass
        
        else:
            print(f"rejected write request response. We don't handle these.")
            print(f"(response was {message['API']['response']})")
            assert False

        return message["API"]["response"]

    ############################################################################
    # INVOKE                                                                   #
    ############################################################################
    # neighbor: the name of the neighbor this message is for
    # mode: which execution mode for the code (PYEXEC only one allowed for now)
    # mode_args:
        # if PYEXEC:
            # target_name: name of the target buffer that contains the code
                # (after filtered through the process's keys)
            # keys_for_new_proc: keys that will be bestowed to the new process.
                # NOTE: These are NOT the keys used to unlock the buffer.
                # Those are in a later argument
            # process: info of the process on this node requesting the invoke.
            # extra_keys: in addition to process keys, other keys may be used.
                # Often, this is because a process uniquely allocates buffers
                # with a key passed to 'keys' and then uses that same key
                # later in 'extra_keys' to access them.
        # if SYSTEM:
            # syscall_name: the name of the syscall desired
            # syscall_args: the args to that specific syscall
    # process: the info of the process on this node requesting the invoke.
        # You can get away with process=None as long as there are extra keys
    def request_invoke_to_buffer(self, neighbor, mode, mode_args, process):
        message = self.message_template()
        message["recipient"] = neighbor
        message["API"]["request"] = "INVOKE"
        message["API"]["mode"] = mode
        message["API"]["mode_args"] = mode_args
        message["process"] = process

        self.waiting_requests[message["RID"]] = message
        self.manager.send(self.name, neighbor, message)


    # Now the neighbor, which received an invoke request,
    # will resolve it on its end.
    # It will spawn a process. Will it send a response immediately to
    # let the sender know it was spawned successfully.
    # Often, a sender will want more information from a process. This
    # information must be baked into the code itself. The code must have
    # instructions to write back to the sender, EBA does not manage return
    # codes of pyexec.
    # response: was a process successfully spawned? True or False.
    def resolve_invoke_request(self, message):
        syscall_response = self.syscall_invoke_to_buffer(
            message["API"]["mode"],
            message["API"]["mode_args"])

        response = self.response_to_message(message)
        for el in syscall_response:
            response["API"][el] = syscall_response[el]

        self.manager.send(self.name, message["sender"], response)

    # Now the original sender of the invoke request will acknowledge
    # that it has, indeed, invoked what it wanted (or not).
    # It doesn't need to send a message again, just
    # needs to update its own data, and possibly inform the requesting process
    def acknowledge_invoke_request(self, message):
        if message["API"]["response"] == True:
            # then the invoke was successful
            # Nothing needs done here. If a process needed to know,
            # then the wrapper will let it know
            pass
        
        else:
            print(f"warning: rejected invoke request response.")
            print(f"(response was {message['API']['response']})")

        return message["API"]["response"]


    ############################################################################

    # the manager pops a message off the queue for the node and the
    # node resolves that specific message
    def resolve_message(self, message):
        if message["RID"] in self.waiting_requests:
            # resovle the request, the recipient has given a response back to us
            # print(f"{self.name} received:\n{message}")
            if message["API"]["request"] == "BUFREQ":
                return_code = self.acknowledge_buffer(message)
            elif message["API"]["request"] == "WRITE":
                expected_len = self.waiting_requests[message["RID"]]["API"]["length"]
                return_code = self.acknowledge_write_request(message, expected_len)
            elif message["API"]["request"] == "INVOKE":
                return_code = self.acknowledge_invoke_request(message)
            else:
                print(f"unknown message type for the following:\n{message}")
            requesting_process_info = self.waiting_requests[message["RID"]]["process"]
            # if a process needed to know the return code, let it know
            if requesting_process_info is None:
                pass # No process needed to know this.
            else:
                proc_name = requesting_process_info["name"]
                which_pickup = requesting_process_info["which_pickup"]
                self.manager.inform_process(
                        self,
                        self.process_dict[proc_name],
                        which_pickup,
                        return_code)

            self.waiting_requests.pop(message["RID"])

        else:
            # This message is a request to me
            if message["API"]["request"] == "BUFREQ":
                self.resolve_buffer_request(message)
            elif message["API"]["request"] == "WRITE":
                self.resolve_write_request(message)
            elif message["API"]["request"] == "INVOKE":
                self.resolve_invoke_request(message)
            else:
                print(f"unknown message type for the following:\n{message}")
        return

    ############################################################################
    # SYSTEM CALLS                                                             #
    ############################################################################
    def syscall_alloc_buffer(
            self,
            buf_for,
            size,
            tags,
            local_name=None,
            process=None):

        bufname = "BUF_"+str(self.buf_counter)
        self.buf_counter += 1
        assert bufname not in self.buffers, f"{bufname} already in {self.buffers}"

        buffer_local_name = {}
        if (local_name is not None and process is not None):
            skey = process["keys"][0] #first key is secret key
            buffer_local_name[skey] = local_name

        sys_tags = {**buffer_local_name, **tags, ROOT_STR: bufname}

        # if any individual key is overloaded on this node, refuse creation.
        dup_keys = [key for key in sys_tags for buf in self.buffers.values()
            if key in buf["tags"] and buf["tags"][key] == sys_tags[key]]
        if len(dup_keys) > 0:
            # print(f"rejecting! found duplicate key(s) {dup_keys}")
            resp = "REJ"
        else:
            self.buffers[bufname] = {
                "owner": self.name,
                "for": buf_for,
                "tags": sys_tags,
                "size": size,
                "contents": None
                }
            resp = "ACK"

        syscall_response = {}
        syscall_response["response"] = resp
        syscall_response["local_name"] = local_name
        # Don't tell the requester ALL tags, just the ones they asked for.
        syscall_response["tags"] = tags
        syscall_response["size"] = size
        return syscall_response

    def syscall_write_to_buffer(
            self,
            target_name,
            mode,
            length,
            payload,
            process,
            extra_keys):
        if process is None:
            proc_keys = []
        else:
            proc_keys = process["keys"]
        keys = proc_keys + extra_keys
        # find the bufname
        sys_bufname = self.syscall_get_buffer(keys, target_name)
        syscall_response = {}
        if sys_bufname is None:
            # then the invoke request will fail
            print(f"error in write: {target_name} does not appear ", end=""),
            print(f"exactly once. Refusing write.")
            syscall_response["response"] = False
        else:
            # successful request for name
            assert sys_bufname is not None, f"failed to find sys_bufname"
            if mode == "START":
                self.buffers[sys_bufname]["contents"] = payload
                syscall_response["response"] = length
            elif mode == "APPEND":
                self.buffers[sys_bufname]["contents"] += payload
                syscall_response["response"] = length
            else:
                print(f"ERROR: unknown write mode {mode}")
                exit(1)
        return syscall_response


    def syscall_invoke_to_buffer(
            self,
            mode,
            mode_args={}):
    
        syscall_response = {}
        
        if mode == "PYEXEC":
            syscall_response["response"] = self.syscall_invoke_pyexec(**mode_args)
        elif mode == "SYSTEM":
            syscall_response["response"] = self.syscall_invoke_system(**mode_args)
        else:
            print(f"unknown mode {mode}. Refusing invoke.")
            syscall_response["response"] = None

        return syscall_response

    # Builtins/Operations
    ############################################################################

    # process: the dict telling the system where to store the information
    def syscall_id(self):
        response = self.name
        return response

    # process: the dict telling the system where to store the information
    def syscall_neighbors(self):
        response = [x for x in self.neighbors if self.neighbors[x] == "connected"]
        return response

    def syscall_mybuf(self):
        return SELF_BUFNAME

    def syscall_read(self, target_name, keys):
        sys_bufname = self.syscall_get_buffer(keys, target_name)
        target = self.buffers[sys_bufname]
        if target is None:
            print(f"READ error: failed to find target {target_name}")
            print("returning an empty response")
            response = False
        elif target["owner"] != self.name:
            response = False
            print(f"fatal error: attempt to access buffer {target_name} not from self")
            print("even more warning: this shouldn't be possible! It was deprecated. Stop.")
            exit(1)
        else:
            response = target["contents"]

        return response

    def syscall_ls(self, keys, as_root=False):
        hits_dict = {}
        for bufname in self.buffers:
            # k=key
            common = [k for k in keys if k in self.buffers[bufname]["tags"]]
            bufnames = [self.buffers[bufname]["tags"][k] for k in common]
            hits_dict[bufname] = bufnames

        all_hits_list = list(hits_dict.values())

        # check for duplicates
        all_names = [name for bufnames in all_hits_list for name in bufnames]
        if len(all_names) != len(set(all_names)):
            print(f"warning! duplicate names in ls returning {all_names}")
            print(f"buffers show {hits_dict}")

        if as_root:
            return hits_dict
        else:
            return all_names

    # HELPER functions
    ############################################################################
    # return the SYSTEM buffer name given a LOCAL buffer name. Similar to ls.
    def syscall_get_buffer(self, keys, target_local_bufname):
        buffer_hits = []
        for sbnm in self.buffers:
            # k=key
            # sbnm=sys_bufname
            common = [k for k in keys if k in self.buffers[sbnm]["tags"]]
            local_bufnames = [self.buffers[sbnm]["tags"][k] for k in common]
            if target_local_bufname in local_bufnames:
                buffer_hits.append(sbnm)

        if len(buffer_hits) == 0:
            print(f"warning! buffer not found. No buffer labeled ", end="")
            print(f"{target_local_bufname} found with keys {keys}.")
            return None
        elif len(buffer_hits) > 1:
            print(f"warning! duplicate names seeking ", end="")
            print(f"{target_local_bufname} with keys {keys}.")
            print(f"No names will be returned due to ambiguity.")
            self.manager.show()
            return None
        else:
            return buffer_hits[0]

    # invoke syscall for when the mode is PYEXEC
    def syscall_invoke_pyexec(
            self,
            target_name,
            keys_for_new_proc=[],
            process=None,
            extra_keys=[]):
        if process is None:
            proc_keys = []
        else:
            proc_keys = process["keys"]
        unlocking_keys = proc_keys + extra_keys
        # find the buffer that contains the target, assuming it exists
        sys_bufname = self.syscall_get_buffer(unlocking_keys, target_name)
        if sys_bufname is None:
            # then the invoke request will fail
            print(f"error in invoke: {target_name} does not appear", end="")
            print(f"exactly once. Refusing invoke.")
            pyexec_response = False
        else:
            # buffer target is in the list of buffers, and we can spawn
    
            # now spawn the process
            proc_name = "PROC_"+str(self.next_PID)
            self.next_PID += 1
            # see EBA node class description for more on dictionary fields
            self.process_dict[proc_name] = {}
            this_process = self.process_dict[proc_name]
            this_process["name"] = proc_name
            this_process["keys"] = keys_for_new_proc
            this_process["bufname"] = sys_bufname
            this_process["last_scheduled"] = 0
    
            # Finally, give the buffer this proc lives in and this proc
            # a unique key
            random_key = EBA_Utils.random_name(length=20)
            this_process["keys"].insert(0, random_key)
            # make this "secret" key always the first key
            # (Not that a process knows about its inherent keys anyway)
            # possible TODO: insert checks that check (possibly sloppily)
                # that this secret key is in fact first
                # these would end up going in the bufreq local name section

            # Add a way for this buffer to find itself
            self.buffers[sys_bufname]["tags"][random_key] = SELF_BUFNAME

            self.manager.init_process(self, this_process)
            pyexec_response = True
        return pyexec_response

    # invoke syscall for when the mode is SYSTEM
    def syscall_invoke_system(
            self,
            syscall_name,
            process,
            syscall_args):

        if "keys" in syscall_args and process is not None:
            syscall_args["keys"] = syscall_args["keys"] + process["keys"]

        syscall_funcs = {
            "ID": self.syscall_id,
            "NEIGHBORS": self.syscall_neighbors,
            "MYBUF": self.syscall_mybuf,
            "READ": self.syscall_read,
            "LS": self.syscall_ls}

        return syscall_funcs[syscall_name](**syscall_args)

    # System call wrapper
    ############################################################################
    def syscall_wrapper(self, req, process_pass):
        response = None
        if req["request"] == "BUFREQ":
            # Then do buffer request
            neighbor = req["neighbor"]
            size = req["size"]
            local_name = req["local_name"]
            tags = req["tags"]
            if neighbor is None or neighbor == self.name:
                # alloc-ing buffer on-node
                response = self.syscall_alloc_buffer(self.name, size, tags, local_name, process_pass)
            else:
                # alloc-ing buffer off-node
                self.request_buffer_from(neighbor, size, tags, local_name, process_pass)
        elif req["request"] == "WRITE":
            # Then do write request
            neighbor = req["neighbor"]
            target = req["target"]
            mode = req["mode"]
            length = req["length"]
            payload = req["payload"]
            extra_keys = req["extra_keys"]
            if neighbor is None or neighbor == self.name:
                # writing to buffer on-node
                response = self.syscall_write_to_buffer(target, mode, length, payload, process_pass, extra_keys)
            else:
                # writing to buffer off-node
                self.request_write_to_buffer(neighbor, target, mode, length, payload, process_pass, extra_keys)
        elif req["request"] == "INVOKE":
            # Then do invoke request
            neighbor = req["neighbor"]
            mode = req["mode"]
            mode_args = req["mode_args"]
            mode_args["process"] = process_pass
            if neighbor is None or neighbor == self.name:
                # invoke to buffer on-node
                response = self.syscall_invoke_to_buffer(mode, mode_args)
            else:
                # invoke to buffer off-node
                self.request_invoke_to_buffer(neighbor, mode, mode_args, process_pass)
        else:
            assert False, f"unknown EBA PYAPI request {req['request']}. Possibly it is not implemented yet?"
        """
        elif req["request"] == "ID":
            response = self.syscall_id()
        elif req["request"] == "NEIGHBORS":
            response = self.syscall_neighbors()
        elif req["request"] == "MYBUF":
            response = self.syscall_mybuf()
        elif req["request"] == "READ":
            target = req["target"]
            extra_keys = req["extra_keys"]
            if extra_keys is None:
                extra_keys = []
            proc_keys = process_pass["keys"]
            # TODO: should we allow system calls from non-processes?
            response = self.syscall_read(target, proc_keys + extra_keys)
        elif req["request"] == "LS":
            extra_keys = req["extra_keys"]
            if extra_keys is None:
                extra_keys = []
            proc_keys = process_pass["keys"]
            response = self.syscall_ls(proc_keys + extra_keys)
        """

        if response is not None: # If the call is on-node
            proc_name = process_pass["name"]
            which_pickup = process_pass["which_pickup"]
            self.manager.inform_process(
                    self,
                    self.process_dict[proc_name],
                    which_pickup,
                    response["response"])
        return


    ############################################################################
    # RUNNING THE NODE                                                         #
    ############################################################################

    def run_one(self): # as if the interrupt was just triggered
        # TODO: The interrupt system needs much more work.
        if len(self.process_dict.keys()) == 0 and len(self.message_queue) == 0:
            return # no need to do anything
        # parse and respond to all messages
        starting_queue_length = len(self.message_queue)
        while len(self.message_queue) > 0:
            this_message = self.message_queue[0]
            self.message_queue = self.message_queue[1:]
            self.resolve_message(this_message)
        # Then run one iteration of a process
        if len(self.process_dict.keys()) > 0:
            # scheduler: choose the process which was least recently given time
            proc_sched_times = {proc: self.process_dict[proc]["last_scheduled"] for proc in self.process_dict}
            chosen_proc = max(proc_sched_times, key=proc_sched_times.get)
            proc_keys = self.process_dict[chosen_proc]["keys"]
            for proc in self.process_dict:
                self.process_dict[proc]["last_scheduled"] += 1
            # mark the process that gets time
            self.process_dict[chosen_proc]["last_scheduled"] = 0

            dropoff_dict = self.manager.run_process(self, self.process_dict[chosen_proc])

            if dropoff_dict["terminate"] is True:
                self.process_dict.pop(chosen_proc)
                # print(f"{chosen_proc} terminated.")
            else:
                # Proc is already in back of queue
                # Parse dropoff & do API calls
                for req_name in dropoff_dict["requests"]:
                    req = dropoff_dict["requests"][req_name]
                    process_pass = {
                            "name": chosen_proc,
                            "keys": proc_keys,
                            "which_pickup": req_name}
                    self.syscall_wrapper(req, process_pass)

        else:
            if starting_queue_length == 0:
                assert False, "code should never touch this spot"




def show_buffers(buffers, indent=0, show_contents=False):
    spc = " "*indent
    dash = "-"*30
    print(spc+dash)
    for bufname in buffers:
        buf = buffers[bufname]
        print(spc+f"name: {bufname}")
        print(spc+f"owner: {buf['owner']}")
        print(spc+f"for: {buf['for']}")
        print(spc+f"tags: {buf['tags']}")
        print(spc+f"size: {buf['size']}")
        if show_contents:
            print(spc+f"contents: {buf['contents']}")
        print(spc+dash)


def show_messages(messages, indent=0):
    spc = " "*indent
    dash = "-"*30
    if type(messages) is dict:
        messages = list(messages.values())
    print(spc+dash)
    for msg in messages:
        if type(msg) is str:
            # Aside for admin messages
            print(spc+msg)
        else:
            print(spc+f"message RID: {msg['RID']}")
            print(spc+f"message sender: {msg['sender']}")
            print(spc+f"message target: {msg['recipient']}")
            print(spc+f"message API: {msg['API']}")
            print(spc+f"message for process: {msg['process']}")
            if 'extra_keys' in msg:
                print(spc+f"message with extra keys: {msg['extra_keys']}")
            print(spc+dash)

def show_processes(processes, indent=0):
    spc = " "*indent
    dash = "-"*30
    if type(processes) is dict:
        processes = list(processes.values())
    print(spc+dash)
    for proc in processes:
        print(spc+f"process name {proc['name']}")
        print(spc+f"in buffer: {proc['bufname']}")
        print(spc+f"with keys: {proc['keys']}")
        print(spc+f"last scheduled: {proc['last_scheduled']}")
        print(spc+dash)


def show_node_state(state_dict, indent=0, show_buffer_contents=False):
    spc = " "*indent
    dash = "-"*50
    print(spc+dash)
    print(spc+f"EBA Node {state_dict['name']}")
    print(spc+f"in state {state_dict['interrupt_state']}")
    print(spc+f"neighbors: {list(state_dict['neighbors'].keys())}")
    print(spc+f"buffers (according to last known information by this node):")
    show_buffers(state_dict["buffers"], indent=indent+4, show_contents=show_buffer_contents)
    print(spc+f"waiting messages in queue:")
    show_messages(state_dict["message_queue"], indent=indent+4)
    print(spc+f"waiting for responses to:")
    show_messages(state_dict["waiting_requests"], indent=indent+4)
    print(spc+f"active processes:")
    show_processes(state_dict["process_dict"], indent=indent+4)


class EBA_Manager:
    def __init__(self, manager_mode="load"):
        self.manager_mode = manager_mode
        self.nodes = {}
        # a dictionary of timeslices and states at those timeslices
        self.system_state = {}
        self.next_timeslice = 0 # Also needs to load/save state and timeslice info
        self.recently_sent = []
        self.nodebufdirs_fname = "nodebufdirs"
        self.fnames_for = {
                "all_state": "MANAGER_ALL_STATE.pkl"}
        # Now create space for buffer files
        if self.manager_mode == "init":
            try:
                os.mkdir(self.nodebufdirs_fname)
            except OSError:
                shutil.rmtree(self.nodebufdirs_fname)
                os.mkdir(self.nodebufdirs_fname)
        elif self.manager_mode == "load":
            self.load()
        else:
            assert False, f"unknown mode '{self.manager_mode}'"

    def all_state(self):
        # Nodes are notably *NOT* in the manager all_state because
        # nodes are known by which directories are present and node
        # state is stored node-side, not manager-side
        return {
                "system_state": self.system_state,
                "next_timeslice": self.next_timeslice,
                "recently_sent": self.recently_sent}

    def new_node(self, name):
        if name in self.nodes:
            print(f"refusing to add node '{name}'. Name already exists.")
        else:
            self.nodes[name] = EBA_Node(name, manager=self)

            nodedirname = self.nodebufdirs_fname+"/"+name
            os.mkdir(nodedirname)
            subprocess.run(["cp", "EBA_PYAPI.py", f"{nodedirname}"])

    def get_node_states(self):
        node_state_slice = {}
        for nodename in self.nodes:
            node = self.nodes[nodename]
            node_state_slice[nodename] = copy.deepcopy(node.all_state())
        return node_state_slice

    # returns the "recently sent" list, but purges the class's handle first
    def purge_recently_sent(self):
        returnme = self.recently_sent
        self.recently_sent = []
        return returnme

    def show(self, state_slice=None, show_buffer_contents=False):
        if state_slice is None:
            state_slice = self.get_node_states()
        print(f"manager of EBA nodes {list(self.nodes.keys())}")
        for node_state in state_slice.values():
            print()
            show_node_state(node_state, indent=4, show_buffer_contents=show_buffer_contents)

    def save(self):
        tfname = self.nodebufdirs_fname+"/"+self.fnames_for["all_state"]
        fb = open(tfname, "wb")
        pickle.dump(self.all_state(), fb)
        fb.close()
        for nodename in self.nodes:
            node = self.nodes[nodename]
            tdir = self.nodebufdirs_fname+"/"+nodename
            node.save_all_state(tdir=tdir)

    def load(self):
        # first, load manager state
        tfname = self.nodebufdirs_fname+"/"+self.fnames_for["all_state"]
        fb = open(tfname, "rb")
        all_state = pickle.load(fb)
        fb.close()
        self.system_state = all_state["system_state"]
        self.next_timeslice = all_state["next_timeslice"]
        self.recently_sent = all_state["recently_sent"]

        # Now nodes and node states
        self.nodes = {}
        for entry in os.scandir(self.nodebufdirs_fname):
            if entry.is_dir():
                self.nodes[entry.name] = EBA_Node(entry.name, manager=self)
        for nodename in self.nodes:
            node = self.nodes[nodename]
            tdir = self.nodebufdirs_fname+"/"+nodename
            node.load_all_state(tdir=tdir)

    def connected(self, n1, n2):
        if n1 not in self.nodes[n2].neighbors:
            # n2 does not know about n1
            return False
        if n2 not in self.nodes[n1].neighbors:
            # n1 does not know about n2
            return False
        return True

    def connect(self, n1, n2):
        if self.connected(n1, n2):
            print(f"warning: '{n1}' and '{n2}' are already connected.")
            return
        # NOTE: if for some reason there was a one-way connection
        #       before this, this function will overwrite any existing
        #       information about that connection

        self.nodes[n1].neighbors[n2] = "connected"
        self.nodes[n2].neighbors[n1] = "connected"
        # "connected" is a placeholder value. Will probably be a whole
        # dictionary eventually

    def send(self, sender, receiver, message):
        if sender == receiver:
            # Just pass it, but no need for noising.
            # TODO this may eventually get removed, but it simplifies the
            # code for now. Yes, you can send to yourself.
            print(f"warning: self-send on {sender}-{receiver}")
            self.nodes[receiver].message_queue.append(message)
            # NOT marking any edges for the network. Self-messages don't
            # go over the network.
        elif not self.connected(sender, receiver):
            print(f"error: {sender} and {receiver} are not connected. Stop.")
            return
        else:
            # else they are connected. Deliver the message.
            # TODO: this is where noise would be added.
            self.nodes[receiver].message_queue.append(message)
            self.recently_sent.append({
                "sender": sender,
                "receiver": receiver,
                "message": message})
        return


    def init_process(self, host_node, process_info):
        node_dir = self.nodebufdirs_fname + "/" + host_node.name
        full_process_fname = node_dir + "/" + process_info["bufname"] + ".py"
        full_pickup_fname = node_dir + "/" + process_info["name"] + ".EBAPICKUP.pkl"
        full_dropoff_fname = node_dir + "/" + process_info["name"] + ".EBADROPOFF.pkl"

        init_pickup_dict = {}
        init_pickup_dict["dropoff"] = full_dropoff_fname
        init_pickup_dict["proc_state"] = "BEGIN"
        init_pickup_dict["proc_vars"] = {}
        init_pickup_dict["responses"] = {}

        pf = open(full_pickup_fname, "wb")
        pickle.dump(init_pickup_dict, pf)
        pf.close()

    def run_process(self, host_node, process_info):
        node_dir = self.nodebufdirs_fname + "/" + host_node.name
        full_process_fname = node_dir + "/" + process_info["bufname"] + ".py"
        # Possible TODO: pickup and dropoff fnames become obsolete fields
        # if we just always append something to them here.
        full_pickup_fname = node_dir + "/" + process_info["name"] + ".EBAPICKUP.pkl"
        full_dropoff_fname = node_dir + "/" + process_info["name"] + ".EBADROPOFF.pkl"

        # If the file's code doesn't already exist, write it in
        # TODO: Does this need done *now* or in the init?
        f = open(full_process_fname, "w")
        f.write(host_node.buffers[process_info['bufname']]['contents'])
        f.close()

        # TODO: possibly different invocation on different systems
        ### print(f"running {full_process_fname}")
        subprocess.run([f"python3", f"{full_process_fname}", f"{full_pickup_fname}"])


        pf = open(full_dropoff_fname, "rb")
        dropoff_dict = pickle.load(pf)
        pf.close()
        return dropoff_dict

    def inform_process(self, host_node, process_info, which_response, info):
        node_dir = self.nodebufdirs_fname + "/" + host_node.name
        full_pickup_fname = node_dir + "/" + process_info["name"] + ".EBAPICKUP.pkl"

        pf = open(full_pickup_fname, "rb")
        pickup_dict = pickle.load(pf)
        pf.close()

        pickup_dict["responses"][which_response] = info

        pf = open(full_pickup_fname, "wb")
        pickle.dump(pickup_dict, pf)
        pf.close()



    # If a node has no work to do
    def node_empty(self, node_name):
        node = self.nodes[node_name]
        if len(node.message_queue) == 0 and len(node.process_dict.keys()) == 0:
            return True
        else:
            return False

    def all_empty(self):
        for node_name in self.nodes:
            if not self.node_empty(node_name):
                return False
        return True

    def run(self, terminate_at=None, only_on=None, run_all=False):
        if terminate_at is None:
            terminate_at = 500 # Max timeslices for testing
        while terminate_at is None or terminate_at > 0:

            run_nodes = []
            if only_on:
                run_nodes.append(self.nodes[only_on])
            elif run_all == False: # choose one randomly
                nodes_with_work =[node_name for node_name in list(self.nodes.keys())
                    if not self.node_empty(node_name)]
                random_node_name = random.choice(nodes_with_work)
                rn = self.nodes[random_node_name]
                run_nodes.append(rn)
            else:
                # run all nodes once
                run_nodes = self.nodes

            # print(f"iteration {terminate_at} running node {rn.name}")
            for rn in run_nodes:
                rn.run_one()
            # This is where we'd get extra data
            sys_state = {}
            sys_state["nodes"] = self.get_node_states()
            sys_state["recent_sends"] = self.purge_recently_sent()
            self.system_state[self.next_timeslice] = sys_state
            self.next_timeslice += 1
            if terminate_at is not None:
                terminate_at -= 1
            if only_on is True and self.node_empty(only_on):
                break
            elif self.all_empty():
                # print("all nodes empty")
                break
