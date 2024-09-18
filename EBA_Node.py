import enum
import numpy as np


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
        self.RID = 0
        self.waiting_requests = {}
        self.message_queue = []

    def message_template(self):
        message = blank_message_template()
        message["sender"] = self.name
        message["RID"] = self.name+str(self.RID)
        self.RID += 1
        return message

    def response_to_message(self, message):
        response = blank_message_template()
        response["sender"] = self.name
        response["recipient"] = message["sender"]
        response["RID"] = message["RID"]
        response["API"]["request"] = message["API"]["request"]
        return response

    def show_buffer(self, indent=0):
        spc = " "*indent
        print(spc+f"-------")
        for bufname in self.buffers:
            buf = self.buffers[bufname]
            print(spc+f"name: {bufname}")
            print(spc+f"owner: {buf['owner']}")
            print(spc+f"for: {buf['for']}")
            print(spc+f"size: {buf['size']}")
            print(spc+f"-------")

    def show_messages(self, indent=0):
        print("Not implemented!")
        # TODO: implement this

    def show(self, indent=0):
        spc = " "*indent
        print(spc+f"EBA Node {self.name}")
        print(spc+f"in state {self.state}")
        print(spc+f"neighbors: {list(self.neighbors.keys())}")
        print(spc+f"buffers:")
        self.show_buffer(indent=indent+4)

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

    def resolve_message(self, message):
        if message["RID"] in self.waiting_requests:
            # resovle the request
            print(f"{self.name} received:\n{message}")
            print("you need to implement code to resolve messages.")
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


class EBA_Manager:
    def __init__(self):
        self.nodes = {}
    def new_node(self, name):
        if name in self.nodes:
            print(f"refusing to add node '{name}'. Name already exists.")
        else:
            self.nodes[name] = EBA_Node(name, manager=self)

    def show(self):
        print(f"manager of EBA nodes {list(self.nodes.keys())}")
        for node in self.nodes.values():
            print()
            node.show(indent=4)

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
                if terminate_at is not None:
                    terminate_at -= 1
            if self.all_empty():
                break
