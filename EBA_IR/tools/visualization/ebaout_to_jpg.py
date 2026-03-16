import sys

import os

if len(sys.argv) < 2:
    print("usage: python ebaout_to_jpg.py <target>")

target = sys.argv[1]

os.system(f"python3 ebaout_to_jgr.py {target}.ebaout > jgrs/{target}.jgr")
os.system(f"python3 jgr_to_jpg.py {target}.jgr")
