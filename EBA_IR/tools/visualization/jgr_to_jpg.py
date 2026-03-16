import sys
import os

if len(sys.argv) < 2:
    print("usage: python jgr_to_jpg.py <target>")
    exit(1)


jgraph_local = "/home/danielmishler/Documents/courses/COSC594/jgraph/jgraph"

target = sys.argv[1]
if target[-4:] != ".jgr":
    print(f"error, target must be a jgraph file (ending in .jgr), received {target}")
rawname = target[:-4]

os.system(f"{jgraph_local} -P ./jgrs/{rawname}.jgr | ps2pdf - | convert -density 300 - -quality 100 ./jpgs/{rawname}.jpg")
