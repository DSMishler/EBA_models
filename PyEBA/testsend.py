API={
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": 30,
    "time": 30}

newvar = self.node_interface(API)

API={
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": 30,
    "time": 30}

self.send_message(API, "node2", newvar["name"], "blue")

API={
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": 30,
    "time": 30}

newvar2 = self.node_interface(API)

API={
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": 30,
    "time": 30}

self.send_message(API, "node2", newvar2["name"], "red")


API={
    "request": "NODEVIS",
    "args": {"style":"filled","fillcolor":"turquoise"}}

self.send_message(API, "ROOT", None, None)
