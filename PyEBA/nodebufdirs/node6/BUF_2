# Write to a circular buffer
# Args: 4
# 0: self (always)
# 1: circular buffer to write to
# 2: buffer that we will write from
# 3: length of our write
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
available_space = buf_size - occupied_space

write_size = int(self.call_args[3])
if available_space >= write_size:
    # then we write
    if buf_tail + write_size > buf_size:
        # then write only enough to wrap around
        init_write_size = buf_size - buf_tail
    else:
        # then write all
        init_write_size = write_size

    API = {
        "request": "READ",
        "target": self.call_args[2],
        "length": write_size,
        "offset": 0}
    write_payload = self.node_interface(API)["response"]

    API = {
        "request": "WRITE",
        "target": buf_desc.split('\n')[0],
        "length": init_write_size,
        "payload": write_payload[:init_write_size],
        "offset": buf_tail}
    self.node_interface(API)

    write_size -= init_write_size
    buf_tail += init_write_size
    buf_tail %= buf_size

    API = {
        "request": "WRITE",
        "target": buf_desc.split('\n')[0],
        "length": write_size,
        "payload": write_payload[init_write_size:],
        "offset": buf_tail}
    self.node_interface(API)

    buf_tail += write_size
    buf_tail %= buf_size

    if buf_tail == buf_head:
        buf_full = 1


    API = {
        "request": "OVERWRITE",
        "target": buf_desc.split('\n')[3],
        "length": len(repr(buf_tail)),
        "payload": repr(buf_tail)}
    self.node_interface(API)
    API = {
        "request": "OVERWRITE",
        "target": buf_desc.split('\n')[4],
        "length": len(repr(buf_full)),
        "payload": repr(buf_full)}
    self.node_interface(API)


else:
    print("refusing write. Not enough space")
