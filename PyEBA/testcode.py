API={
    "request": "BUFREQ",
    "mode": "ALLOC",
    "size": 30,
    "time": 30}

newvar = self.node_interface(API)

API={
    "request": "WRITE",
    "target": newvar["name"],
    "length": len("print('hello world')"),
    "payload": "print('hello world')",
    "offset": 0}

newvar2 = self.node_interface(API)

API={
    "request": "INVOKE",
    "mode": "PYEXEC",
    "target": newvar["name"],
    "call_args": []}

newvar3 = self.node_interface(API)
