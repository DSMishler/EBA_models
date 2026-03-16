import sys

if len(sys.argv) < 2:
    print("usage: python outputs_to_ebaouts.py <target>")
    exit(1)

target = sys.argv[1]
if target[-7:] != ".output":
    print(f"only '.output' files are allowed. You inputted {target}")
    exit(1)

rawname = target[:-7]


f = open(f"outputs/{target}")
ftext = f.read()
f.close()

lines = ftext.split('\n')

current_ebaout = -1
writemode = False

moving_f = open(f"ebaouts/{rawname}test.ebaout", "w")

for line in lines:
    if line == "CURRENTLY_RUNNING_SCHED_QUEUE_MAIN":
        current_ebaout += 1
        moving_f = open(f"ebaouts/{rawname}_{current_ebaout}.ebaout", "w")
        writemode = True
    elif line[:17] == "CURRENTLY_RUNNING":
        writemode = False
        if not moving_f.closed:
            moving_f.close()
    elif writemode == False:
        pass
    else: # writemode is true and we're inside a sched queue main print
        moving_f.write(line)
        moving_f.write('\n')
