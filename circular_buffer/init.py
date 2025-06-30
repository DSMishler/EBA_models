# Initialize a circular buffer
# Args: 3
# 0: self (always)
# 1: size of buffer (bytes)
# 2: name of buffer to put name of return buffer


return_bufname = self.call_args[2]


# structure of the circular buffer
# Data: the true buffer which holds the data
# size: buffer containing the size of said buffer
# head: buffer containing the start of where data is allocated
# tail: buffer containing the end of where data is allocated
# full: buffer containing "1" if the circular buffer is full

intbuflen = len(str(self.call_args[1]))
databufsize = int(self.call_args[1])

API = {
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": databufsize,
    "time": 3600}
databuf = self.node_interface(API)["name"]
API = {
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": intbuflen,
    "time": 3600}
sizebuf = self.node_interface(API)["name"]
headbuf = self.node_interface(API)["name"]
tailbuf = self.node_interface(API)["name"]
API = {
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": 1, # 1 or 0
    "time": 3600}
fullbuf = self.node_interface(API)["name"]

API = {
    "request": "WRITE",
    "target": sizebuf,
    "length": len(str(databufsize)),
    "payload": str(databufsize),
    "offset": 0}
self.node_interface(API)
API["length"] = 1
API["payload"] = "0"
API["target"] = headbuf
self.node_interface(API)
API["target"] = tailbuf
self.node_interface(API)
API["target"] = fullbuf
self.node_interface(API)


circ_buffer_desc = "\n".join([databuf, sizebuf, headbuf, tailbuf, fullbuf])

API = {
    "request": "WRITE",
    "target": self.call_args[2],
    "length": len(circ_buffer_desc),
    "payload": circ_buffer_desc,
    "offset": 0}
self.node_interface(API)
