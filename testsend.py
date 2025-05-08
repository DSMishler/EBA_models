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
