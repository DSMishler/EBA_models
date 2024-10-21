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


import enum
# import numpy as np
import copy
import os
import shutil
import pickle

import gv_utils


# API Calls:
# BUFFER REQUEST
# buffer request, size (in bytes) (-1 is infinity)
# ex. BUFREQ -1
# response will have "ACK" next along with the buffer name and space
# ex. BUFREQ ACK BUF1 1000
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
# invoke request, type of invoke (always pyexec here), buffer name
# invoke what is in a buffer to be run as code
# ex. INVOKE PYEXEC BUF1
# response will only be given when the code is terminated with an exit code
# ex. INVOKE 0
# and the exit code will determine if there was an error
# ex. INVOKE 1
# it might be a good idea in the future to make more data optional


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
        self.fnames_for = {
                "all_state": "NODE_ALL_STATE.txt",
                "interrupt_state": "NODE_INTERRUPT_STATE.txt",
                "neighbors": "NODE_NEIGHBORS.txt",
                "buffers": "NODE_BUFFERS.txt",
                "waiting_requests": "NODE_WAITING_REQUESTS.txt",
                "message_queue": "NODE_MESSAGE_QUEUE.txt",
                "message_queue": "NODE_PROCESS_DICT.txt"}

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
    # BUFREQ
    ############################################################################

    # neighbor: the name of the neighbor this message is for
    # space: the requested space of the buffer
    # process: the name of the process on this node which requests the buffer
        # process is allowed to be None, but this is not going to be implemented
        # as a default
    def request_buffer_from(self, neighbor, space, process):
        message = self.message_template()
        message["recipient"] = neighbor
        message["API"]["request"] = "BUFREQ"
        message["API"]["space"] = space
        message["process"] = process
        
        # note that we are now awaiting a response
        self.waiting_requests[message["RID"]] = message
        
        # send the message
        self.manager.send(self.name, neighbor, message)


    # Now the neighbor, which received a buffer request,
    # will resolve it on its end. This neighbor will currently always
    # give a buffer of requested size, but this does not *have* to be this way
    def resolve_buffer_request(self, message):
        bufname = "BUF_"+message["RID"]

        # Allocate the buffer
        if bufname in self.buffers:
            pass
        else:
            self.buffers[bufname] = {
                    "owner": self.name,
                    "for": message["sender"],
                    "size": -1,
                    "contents": None
                    }

        # Send a message back to the sender
        response = self.response_to_message(message)
        response["API"]["response"] = "ACK"
        response["API"]["name"] = bufname
        response["API"]["size"] = self.buffers[bufname]["size"]

        self.manager.send(self.name, message["sender"], response)


    # Now the original sender of the buffer request will acknowledge
    # that it has a buffer. It doesn't need to send a message again, just
    # needs to update its own data.
    def acknowledge_buffer(self, message):
        if message["API"]["response"] == "ACK":
            bufname = message["API"]["name"]
            owner = message["sender"]
            # TODO: the API should maybe tell me if I'm the only writer or not
            # TODO: was there a process that wanted to know about this?
            #       if so, then we need to inform that process of what's up
            buf_for = message["recipient"]
            size = message["API"]["size"]

            self.buffers[bufname] = {
                    "owner": owner,
                    "for": buf_for,
                    "size": size,
                    "contents": None
                    }
        elif message["API"]["response"] == "REJ":
            print("rejected buffer request. We don't handle these.")
            exit(1)
        else:
            print(f"unknown API response {message['API']['response']}")
            exit(1)

        return None # Explicitly writing that no news is good news


    ############################################################################
    # WRITE
    ############################################################################
    # neighbor: the name of the neighbor this message is for
    # bufname: name of the target buffer
    # mode: START or APPEND (only start implemented right now)
    # length: length of payload
    # payload: the content that will be written to the buffer
    # process: the name of the process on this node which requests the bufrfer
        # process is allowed to be None, but this is not going to be implemented
        # as a default
    def write_to_buffer(self, neighbor, bufname, mode, length, payload, process):
        assert length != 0, f"illegal request: write with length of 0"
        assert length == len(payload), f"lllegal request: payload {payload} claimed to have length {length} instead of {len(payload)}"

        message = self.message_template()
        message["recipient"] = neighbor
        message["API"]["request"] = "WRITE"
        message["API"]["target"] = bufname
        message["API"]["mode"] = mode
        message["API"]["length"] = length
        message["API"]["payload"] = payload
        message["process"] = process

        self.waiting_requests[message["RID"]] = message
        self.manager.send(self.name, neighbor, message)

    # Now the neighbor, which received a write request,
    # will resolve it on its end. This neighbor will currently always
    # successfully write, but this does not *have* to be this way
    def resolve_write_request(self, message):
        target = message["API"]["target"]
        payload = message["API"]["payload"]
        mode = message["API"]["mode"]
        response = self.response_to_message(message)
        if target not in self.buffers:
            # then the write request will fail
            response["API"]["response"] = 0
        else:
            # successful request for name
            if mode == "START":
                self.buffers[target]["contents"] = payload
            elif mode == "APPEND":
                print("append not yet implemented")
                # self.buffers[target]["contents"] += payload
                exit(1)
            else:
                print(f"ERROR: unknown write mode {mode}")
                exit(1)

            response["API"]["response"] = message["API"]["length"]

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

        return None # writing it explicitly. No news is good news.

    ############################################################################

    def resolve_message(self, message):
        if message["RID"] in self.waiting_requests:
            # resovle the request, the recipient has given a response back to us
            # print(f"{self.name} received:\n{message}")
            if message["API"]["request"] == "BUFREQ":
                return_code = self.acknowledge_buffer(message)
                self.waiting_requests.pop(message["RID"])
            elif message["API"]["request"] == "WRITE":
                expected_len = self.waiting_requests[message["RID"]]["API"]["length"]
                return_code = self.acknowledge_write_request(message, expected_len)
                self.waiting_requests.pop(message["RID"])
            else:
                print(f"unknown message type for the following:\n{message}")
            # TODO: if a process needed to know the return code, let it know
        else:
            # This message is a request to me
            if message["API"]["request"] == "BUFREQ":
                self.resolve_buffer_request(message)
            elif message["API"]["request"] == "WRITE":
                self.resolve_write_request(message)
            else:
                print(f"unknown message type for the following:\n{message}")
        return

    def run_one(self): # as if the interrupt was just triggered
        # TODO: The interrupt system needs much more work.
        if len(self.process_dict.keys()) == 0 and len(self.message_queue) == 0:
            return # no need to do anything
        elif len(self.process_dict.keys()) > 0:
            # TODO: run a process
            pass
        else:
            # parse and respond to a message
            this_message = self.message_queue[0]
            self.message_queue = self.message_queue[1:]
            self.resolve_message(this_message)




def show_buffers(buffers, indent=0, show_contents=False):
    spc = " "*indent
    dash = "-"*30
    print(spc+dash)
    for bufname in buffers:
        buf = buffers[bufname]
        print(spc+f"name: {bufname}")
        print(spc+f"owner: {buf['owner']}")
        print(spc+f"for: {buf['for']}")
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
        print(spc+f"message RID: {msg['RID']}")
        print(spc+f"message sender: {msg['sender']}")
        print(spc+f"message target: {msg['recipient']}")
        print(spc+f"message API: {msg['API']}")
        print(spc+f"message for process: {msg['process']}")
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
                "all_state": "MANAGER_ALL_STATE.txt"}
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
            if self.manager_mode == "init":
                os.mkdir(self.nodebufdirs_fname+"/"+name)

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
        if not self.connected(sender, receiver):
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

    def all_empty(self):
        for node_name in self.nodes:
            node = self.nodes[node_name]
            if len(node.message_queue) > 0:
                return False
        return True

    def run(self, terminate_at=None):
        while terminate_at is None or terminate_at > 0:
            # random_node_name = np.random.choice(self.nodes)
            # Locally this numpy doesn't have random. Bruh. #TODO Fix this.
            for random_node_name in self.nodes: # just do one at a time for now
                rn = self.nodes[random_node_name]
                rn.run_one()
                # This is where we'd get extra data
                sys_state = {}
                sys_state["nodes"] = self.get_node_states()
                sys_state["recent_sends"] = self.purge_recently_sent()
                self.system_state[self.next_timeslice] = sys_state
                self.next_timeslice += 1
                if terminate_at is not None:
                    terminate_at -= 1
            if self.all_empty():
                break
