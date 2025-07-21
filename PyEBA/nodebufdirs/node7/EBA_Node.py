# Daniel Mishler
# EBA_Node for PYEBA3

import time as pytime # calculating expiry time of buffers

# EBA node state is loaded in via a dictionary in a reserved file

# The node_state dictionary:
# name: this node's name
# neighbors: list of the names of direct neighboring nodes
# buf_range: list of ranges of ints for alloc-ed buffers
    # buffers are allocated on a rolling ring. The buf_range tells the node
    # what the next buffer to be allocated will be called
    # list is non-inclusive, ex. [0,0] is empty, [10,20] is 10 buffers 10-19
# buffers: a dict of all active buffers, keyed by buffer name
    # name: a repeat of the buffer name
    # size: size of buffer (-1 is infinite size)
    # exp: epoch time the buffer will expire (None is never)


################################################################################
# Reserved buffers
# node_info.EBA: stores the information above
# send_buf.EBA: buffer for staging messages to be sent. One message per line.
# call_queue.EBA: buffer for staging other buffers which are to be invoked
#                 One buffer per line.
# message_queue.EBA: buffer for staging messages received. One message per line.
#                    There is only one for now. Perhaps one per neighbor later.
################################################################################

# helper function
# unfortunately, our decision to implement buffers as files makes this very
# inneficient in Python. This is just the best we can do for now.
def pop_first_line_from(fname):
    f = open(fname, "r")
    ftext = f.read()
    f.close()
    if len(ftext) == 0:
        first_line = None
    else:
        first_line = ftext.split('\n')[0]
        new_text = "\n".join(ftext.split('\n')[1:])
        f = open(fname, "w")
        f.write(new_text)
        f.close()
    return first_line

class EBA_Node:
    def __init__(self, mode="load", name=None, contents=None):
        assert mode in ["load", "init", "show"]

        if mode == "init":
            assert name is not None, "you have to name the node"
            self.node_state = {
                    "name": name,
                    "neighbors": [],
                    "buf_range": [0,0],
                    "buffers": {}}
            def add_system_buf(bufname, init_val=None):
                bufdict = {
                    "name": bufname,
                    "size": -1,
                    "exp": None}
                self.node_state["buffers"][bufname] = bufdict
                f = open(bufname, "w")
                if init_val is not None:
                    f.write(repr(init_val))
                f.close()
            add_system_buf("node_info.EBA")
            add_system_buf("send_buf.EBA")
            add_system_buf("message_queue.EBA")
            add_system_buf("call_queue.EBA")
            for syncbuf in [f"SYNC_{i}.sys" for i in range(3)]:
                add_system_buf(syncbuf, 1)
            f = open("node_info.EBA", "w")
            f.write(repr(self.node_state))
            f.close()
        

        f = open("node_info.EBA", "r")
        self.node_state = eval(f.read())
        f.close()

        if mode == "show":
            assert contents in ["True", "False"]
            self.show(contents=eval(contents))
        else:
            # then run some of the jobs
            self.run()


            # write the new node state
            f = open("node_info.EBA", "w")
            f.write(repr(self.node_state))
            f.close()

    def run(self):
        # first, resolve the entire message queue
        # NOTE: this is a critical region and the code should not be interrupted
        # while here
        while True:
            message = pop_first_line_from("message_queue.EBA")
            if message is not None:
                self.resolve_message(eval(message))
            else:
                break
        f = open("message_queue.EBA", "w")
        f.close()

        # Now run the call queue
        f = open("call_queue.EBA", "r")
        call_queue_slice = f.read()
        f.close()
        # TODO: make this file resetting a function
        f = open("call_queue.EBA", "w")
        f.close()

        for line in call_queue_slice.split('\n'):
            if line == "":
                continue
            words = line.split()
            buf = words[0]
            f = open(buf, "r")
            buftext = f.read()
            f.close()
            self.call_args=words
            exec(buftext)


    def show(self, contents=False):
        for buf in self.node_state["buffers"].values():
            print(f"buffer {buf['name']}:")
            print(f"    size: {buf['size']}")
            if buf['exp'] is None:
                print(f"    expires: never")
            else:
                print(f"    expires: {buf['exp']-pytime.time()} seconds")
            if contents == True:
                print(f"    contents:")
                f = open(buf["name"], "r")
                print(f.read())
                f.close()
            print()

    def resolve_prim_READ(self, request, target, length, offset):
        assert request == "READ"
        if target not in self.node_state["buffers"]:
            print(f"READ error: {target} not in my buffers.")
            return {"response": None}
        else:
            # TODO: python automatically handles it for you if you read
            # from a file past its endpoint. This is technically undefined
            # behavior if buffers are not zeroed and Python doesn't check.
            # Should this be allowed? Other implementations should keep this
            # in mind.
            f = open(target, "r")
            f.seek(offset)
            text = f.read(length)
            f.close()
            return {"response": text}

    def resolve_prim_BUFREQ(self, request, mode, size, time):
        assert request == "BUFREQ"
        assert mode == "ALLOC" # this will be changed later
        assert type(size) is int
        buf_num = self.node_state["buf_range"][1]
        buf_name = f"BUF_{buf_num}"
        self.node_state["buf_range"][1] += 1

        # check that the file does not already exist
        try:
            f = open(buf_name, "r")
            assert False, f"buffer {buf_name} appears to already exist."
        except FileNotFoundError:
            pass

        # creating the buffer as a file so it can be seen
        f = open(buf_name, "w")
        f.close()

        # TODO: logic determining available space would go here
        self.node_state["buffers"][buf_name] = {
                "name": buf_name,
                "size": size,
                "exp": pytime.time() + float(time)}

        return {"response": True, **self.node_state["buffers"][buf_name]}

    def resolve_prim_WRITE(self, request, target, length, payload, offset):
        assert request == "WRITE"

        # Think about whether or not the users should be allowed to write to
        # reserved buffers. It seems like an extra step, but this is how
        # it would be done.
        # if target[-4:] == ".EBA":
            # # not allowed to write to reserved buffers
            # return {"response": 0}

        if target not in self.node_state["buffers"]:
            bufs_list = list(self.node_state["buffers"].keys())
            print(f"WRITE error! write {target} not in {bufs_list}")
            print(payload)
            return {"response": 0}

        if len(payload) != length:
            print(f"WRITE error! payload of len {len(payload)} (expected {length})")
            print(f"refusing write to {target}")
            print(payload)
            return {"response": 0}

        # now check if writing is possible:
        if self.node_state["buffers"][target]["size"] != -1:
            # if not infinite length
            bufsize = self.node_state["buffers"][target]["size"]
            if length + offset > bufsize:
                print(f"WRITE error: payload too large for buffer {target}")
                print(f"length of {length+offset} vs size of {bufsize}")
                print(f"payload {payload}")
                return {"response": 0}

        f = open(target, "r+")
        f.seek(offset)
        f.write(payload)
        f.close()

        return {"response": length}

    def resolve_prim_INVOKE(self, request, mode, target, call_args):
        assert request == "INVOKE"
        assert mode in ["SYSCALL", "PYEXEC", "TESTANDSET"]
        assert type(call_args) is list

        if mode == "PYEXEC":
            # NOTE: we don't check if the element is already in the queue.
            if target not in self.node_state["buffers"]:
                print(f"error! Cannot invoke {target} - it is not a buffer")
                print(f"buffers: {self.node_state['buffers']}")
                return {"response": False}
            f = open("call_queue.EBA", "a")
            f.write(target)
            for arg in call_args:
                assert type(arg) is str
                f.write(f" {arg}")
            f.write('\n')
            f.close()
            return {"response": True}

        elif mode == "SYSCALL":
            rd = {"response": None}
            if target == "ID":
                rd["response"] = self.node_state["name"]
            elif target == "NEIGHBORS":
                rd["response"] = self.node_state["neighbors"]
            else:
                print(f"unknown systcall '{target}'")
            return rd

        elif mode == "TESTANDSET":
            # TODO: if parallelized, add a lock here
            if target[-4:] != ".sys":
                print("EBA error, TESTANDSET on non-system buf")
                response = {"response": "ERROR"}

            else:
                f = open(target, "r")
                val = eval(f.read())
                f.close()
                f = open(target, "w")
                f.write(repr(0))
                f.close()
                response = {"response": val}

            return response

    # Operation APPEND finds the length of the buffer and then writes to it
    # NOTE: this really "cheats" EBA with Python. It uses the fact that
    # Python will have an end of the file and thus provide the length.
    # A non-Python implementation would have to have some sort of "end"
    # character reserved for a buffer, or perhaps simply disallow appending
    # to buffers
    def resolve_op_APPEND(self, request, target, length, payload):
        assert request == "APPEND"

        if target not in self.node_state["buffers"]:
            bufs_list = list(self.node_state["buffers"].keys())
            print(f"APPEND error! write {target} not in {bufs_list}")
            print(payload)
            return {"response": 0}

        bufsize = self.node_state["buffers"][target]["size"]
        readresp = self.resolve_prim_READ("READ", target, bufsize, 0)
        text = readresp["response"]

        offset = len(text)
        return self.resolve_prim_WRITE("WRITE", target, length, payload, len(text))

    # See note about append
    def resolve_op_READALL(self, request, target):
        assert request == "READALL"
        if target not in self.node_state["buffers"]:
            bufs_list = list(self.node_state["buffers"].keys())
            print(f"READALL error! {target} not in {bufs_list}")
            return {"response": 0}

        bufsize = self.node_state["buffers"][target]["size"]
        return self.resolve_prim_READ("READ", target, bufsize, 0)

    def resolve_op_OVERWRITE(self, request, target, length, payload):
        assert request == "OVERWRITE"

        if target not in self.node_state["buffers"]:
            bufs_list = list(self.node_state["buffers"].keys())
            print(f"OVERWRITE error! write {target} not in {bufs_list}")
            print(payload)
            return {"response": 0}

        f = open(target, "w")
        f.close()
        return self.resolve_prim_WRITE("WRITE", target, length, payload, 0)



    def resolve_message(self, message):
        which_prim_call = {
                "BUFREQ": self.resolve_prim_BUFREQ,
                "WRITE":  self.resolve_prim_WRITE,
                "INVOKE": self.resolve_prim_INVOKE,
                "OVERWRITE":  self.resolve_op_OVERWRITE,
                "APPEND":  self.resolve_op_APPEND}

        assert message["recipient"] == self.node_state["name"]
        assert message["API"]["request"] in which_prim_call

        # Now do the call to the primitive
        response = which_prim_call[message["API"]["request"]](**message["API"])

        if message["response_buffer"] is None:
            return # we are done
        else:
            API_for_new_message = {
                    "request": "WRITE",
                    "target": message["response_buffer"],
                    "length": len(repr(response)),
                    "payload": repr(response),
                    "offset": 0}

            new_message = {}
            new_message["recipient"] = message["sender"]
            new_message["sender"] = self.node_state["name"]
            new_message["API"] = API_for_new_message
            new_message["response_buffer"] = None # this is a response
            try:
                if message["color"] is None:
                    new_message["color"] = None
                else:
                    new_message["color"] = "dark"+message["color"]
            except KeyError:
                pass # no color included in the message? Okay, none in response.

            # Now put this message in our send buffer
            self.to_sendbuf(new_message)

    # These are the only allowed functions that can be called when interfacing
    # the node. It requires crafting a message via the standard message protocol
    # and helper functions are allowed for that. The message will be sent if
    # it is to someone else, and it will be immediately resolved if it is
    # on-node
    def node_interface(self, API):
        # TODO: combine somehow with code above for resolve_message?
        which_prim_call = {
                "BUFREQ": self.resolve_prim_BUFREQ,
                "WRITE":  self.resolve_prim_WRITE,
                "INVOKE": self.resolve_prim_INVOKE,
                "READ": self.resolve_prim_READ,
                "READALL": self.resolve_op_READALL,
                "OVERWRITE":  self.resolve_op_OVERWRITE,
                "APPEND":  self.resolve_op_APPEND}

        assert API["request"] in which_prim_call, f"{API['request']} not found"

        # Now do the call to the primitive
        response = which_prim_call[API["request"]](**API)
        return response

    def send_message(self, API, recipient, response_buffer=None, color=None):
        message = {}
        message["recipient"] = recipient
        message["sender"] = self.node_state["name"]
        message["API"] = API
        message["response_buffer"] = response_buffer
        message["color"] = color

        if message["recipient"] == message["sender"]:
            print("error, no sending messages to yourself. Stop.")
            return
        self.to_sendbuf(message)

    def to_sendbuf(self, message):
        # Make sure the recipient is legal first.
        if message["recipient"] not in self.node_state["neighbors"] + ["ROOT"]:
            print(f"node {self.node_state['name']} cannot send message.")
            print(f"{message['recipient']} is not a neighbor.")
            return
        API_for_send_buffer = {
                "request": "APPEND",
                "target": "send_buf.EBA",
                "length": len(repr(message)+"\n"),
                "payload": repr(message)+"\n"}
        self.resolve_op_APPEND(**API_for_send_buffer)


if __name__ == "__main__":
    import sys
    arg_dict = {}
    if len(sys.argv) >= 2:
        for i in range(1, len(sys.argv)):
            arg = sys.argv[i]
            [key,val] = arg.split("=")
            arg_dict[key] = val

    node = EBA_Node(**arg_dict)
