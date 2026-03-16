import sys

import os

if len(sys.argv) < 2:
    print("usage: python ebaouts_to_jpgs.py <target>")

target = sys.argv[1]

options = [x[:-7] for x in os.listdir("./ebaouts") if f"{target}_" in x]

for option in options:
    os.system(f"python3 ebaout_to_jgr.py {option}.ebaout > jgrs/{option}.jgr")
    os.system(f"python3 jgr_to_jpg.py {option}.jgr")
