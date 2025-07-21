# Read from a circular buffer
# Args: 4
# 0: self (always)
# 1: circular buffer to read from
# 2: buffer that we will read into
# 3: length of our read
# perhaps an offset will be added later


API = {
    "request": "READ",
    "target": self.call_args[1],
    "length": 100,
    "offset": 0}
buf_desc = self.node_interface(API)["response"]


API["target"] = buf_desc.split('\n')[1] # size
buf_size = int(self.node_interface(API)["response"])
API["target"] = buf_desc.split('\n')[2] # head
buf_head = int(self.node_interface(API)["response"])
API["target"] = buf_desc.split('\n')[3] # tail
buf_tail = int(self.node_interface(API)["response"])
API["target"] = buf_desc.split('\n')[4] # full
buf_full = int(self.node_interface(API)["response"])


occupied_space = buf_tail - buf_head + buf_size * buf_full
if buf_tail < buf_head:
    # then we had a wrap-around
    occupied_space += buf_size

read_size = int(self.call_args[3])
if occupied_space >= read_size:
    # Then we are safe to read
    if buf_head + read_size > buf_size:
        # then we can only read some portion
        # and must finish the rest later
        init_read_size = buf_size - buf_head
    else:
        init_read_size = read_size

    API = {
        "request": "READ",
        "target": buf_desc.split('\n')[0],
        "length": init_read_size,
        "offset": buf_head}
    first_read = self.node_interface(API)["response"]

    read_size -= init_read_size
    buf_head += init_read_size
    buf_head %= buf_size

    API = {
        "request": "READ",
        "target": buf_desc.split('\n')[0],
        "length": read_size,
        "offset": buf_head}
    second_read = self.node_interface(API)["response"]

    buf_head += read_size
    buf_head %= buf_size

    if read_size > 0 or init_read_size > 0:
        buf_full = 0

    whole_read = first_read + second_read

    API = {
        "request": "WRITE",
        "target": self.call_args[2],
        "length": len(whole_read),
        "payload": whole_read,
        "offset": 0}
    self.node_interface(API)


    API = {
        "request": "OVERWRITE",
        "target": buf_desc.split('\n')[2],
        "length": len(repr(buf_head)),
        "payload": repr(buf_head)}
    self.node_interface(API)
    API = {
        "request": "OVERWRITE",
        "target": buf_desc.split('\n')[4],
        "length": len(repr(buf_full)),
        "payload": repr(buf_full)}
    self.node_interface(API)


else:
    print("refusing read. Not enough content")
