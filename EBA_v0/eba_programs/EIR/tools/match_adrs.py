import sys

if len(sys.argv) < 2:
    printf("usage: python match_adrs.py <target_file>")

fname = sys.argv[1]
f = open(fname, "r")
ftext = f.read()
lines = ftext.split('\n')
f.close()

allocs = []
releases = []

for line in lines:
    if "alloc-ed" in line:
        allocs.append(line.split()[-1])
    if "release-ed" in line:
        releases.append(line.split()[-1])


for a in allocs:
    if a not in releases:
        print(f"address alloc-ed but not releas-ed: {a}")

for r in releases:
    if r not in allocs:
        print(f"address release-ed but not alloc-ed: {a}")
