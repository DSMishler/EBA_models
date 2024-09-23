import enum
# import numpy as np
import copy

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


# message format:
# sender id
# recipient id
# sender request # (or recipient if responding)
# API call specifics

def blank_message_template():
    return {
        "sender": None,
        "recipient": None,
        "RID": None,
        "API": {
            "request": None,
            "response": None
            # It is possible more will be added here, depending on the API call
            }
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
        self.state = EBA_State.IDLE
        self.manager = manager
        # manager is for Python exchanges, so that no node
        # has access to the code of another
        self.neighbors = {}
        self.buffers = {}
        self.next_RID = 0
        self.waiting_requests = {}
        self.message_queue = []

    def all_state(self):
        return {
                "name": self.name,
                "state": self.state,
                "neighbors": self.neighbors,
                "buffers": self.buffers,
                "waiting_requests": self.waiting_requests,
                "message_queue": self.message_queue}

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
        return response

    def request_buffer_from(self, neighbor, space):
        message = self.message_template()
        message["recipient"] = neighbor
        message["API"]["request"] = "BUFREQ"
        message["API"]["space"] = space
        
        # note that we are now awaiting a response
        self.waiting_requests[message["RID"]] = message
        
        # send the message
        self.manager.send(self.name, neighbor, message)

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

    def acknowledge_buffer(self, message):
        if message["API"]["response"] == "ACK":
            bufname = message["API"]["name"]
            owner = message["sender"]
            # TODO: the API should maybe tell me if I'm the only writer or not
            buf_for = message["recipient"]
            size = message["API"]["size"]

            self.buffers[bufname] = {
                    "owner": owner,
                    "for": buf_for,
                    "size": size,
                    "contents": None
                    }
        elif message["API"]["response"] == "REJ":
            print("rejected request")
        else:
            print(f"unknown API response {message['API']['response']}")

    def resolve_message(self, message):
        if message["RID"] in self.waiting_requests:
            # resovle the request, the recipient has given a response back to us
            # print(f"{self.name} received:\n{message}")
            if message["API"]["request"] == "BUFREQ":
                self.acknowledge_buffer(message)
                self.waiting_requests.pop(message["RID"])
            else:
                print(f"unknown message type for the following:\n{message}")
        else:
            # This message is a request to me
            if message["API"]["request"] == "BUFREQ":
                self.resolve_buffer_request(message)
            else:
                print(f"unknown message type for the following:\n{message}")
        return

    def run_one(self): # as if the interrupt was just triggered
        # TODO: The interrupt system needs much more work.
        if len(self.message_queue) == 0:
            return # no need to do anything
        else:
            this_message = self.message_queue[0]
            self.message_queue = self.message_queue[1:]
            self.resolve_message(this_message)




def show_buffers(buffers, indent=0):
    spc = " "*indent
    dash = "-"*30
    print(spc+dash)
    for bufname in buffers:
        buf = buffers[bufname]
        print(spc+f"name: {bufname}")
        print(spc+f"owner: {buf['owner']}")
        print(spc+f"for: {buf['for']}")
        print(spc+f"size: {buf['size']}")
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
        print(spc+dash)

def show_node_state(state_dict, indent=0):
    spc = " "*indent
    dash = "-"*50
    print(spc+dash)
    print(spc+f"EBA Node {state_dict['name']}")
    print(spc+f"in state {state_dict['state']}")
    print(spc+f"neighbors: {list(state_dict['neighbors'].keys())}")
    print(spc+f"buffers:")
    show_buffers(state_dict["buffers"], indent=indent+4)
    print(spc+f"waiting messages in queue:")
    show_messages(state_dict["message_queue"], indent=indent+4)
    print(spc+f"waiting for responses to:")
    show_messages(state_dict["waiting_requests"], indent=indent+4)


class EBA_Manager:
    def __init__(self):
        self.nodes = {}
        self.node_states = {} # a dictionary of timeslices
                              # and states at those timeslices
        self.next_timeslice = 0
    def new_node(self, name):
        if name in self.nodes:
            print(f"refusing to add node '{name}'. Name already exists.")
        else:
            self.nodes[name] = EBA_Node(name, manager=self)

    def get_node_states(self):
        node_state_slice = {}
        for nodename in self.nodes:
            node = self.nodes[nodename]
            node_state_slice[nodename] = copy.deepcopy(node.all_state())
        return node_state_slice

    def show(self, state_slice=None):
        if state_slice is None:
            state_slice = self.get_node_states()
        print(f"manager of EBA nodes {list(self.nodes.keys())}")
        for node_state in state_slice.values():
            print()
            show_node_state(node_state, indent=4)

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
                self.node_states[next_timeslice] = self.get_node_states()
                self.next_timeslice += 1
                if terminate_at is not None:
                    terminate_at -= 1
            if self.all_empty():
                break
