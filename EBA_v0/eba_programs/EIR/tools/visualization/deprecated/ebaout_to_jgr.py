import sys

if len(sys.argv) < 2:
    print("usage: python ebaout_to_jgr.py <target>")

target = sys.argv[1]
if target[-7:] != ".ebaout":
    print(f"only '.ebaout' files are allowed. You inputted {target}")

rawname = target[:-7]

f = open(f"ebaouts/{target}")
target_txt = f.read()
f.close()
lines = target_txt.split('\n')








print("newgraph")
print("xaxis min -50 max 50 nodraw")
print("yaxis min -20 max 20 nodraw")
print("newstring font Time-Roman fontsize 30 x 0 y 95 hjc vjc :")
print("scheduler visualization!")

lineheight = 5
linewidth = 60
buf_start_x = -30
buf_start_y = -35
num_entries = 25

print(f"newstring font Time-Roman fontsize 20 x 0 y {buf_start_y-5} hjc vjc :")
print("scheduler base")

print("newline poly pfill 1 pts")
print(f"{buf_start_x} {buf_start_y} {buf_start_x} {buf_start_y+num_entries*lineheight}")
print("newline poly pfill 1 pts")
print(f"{buf_start_x+linewidth} {buf_start_y} {buf_start_x+linewidth} {buf_start_y+num_entries*lineheight}")

for i in range(num_entries+1):
    print("newline poly pfill 1 pts")
    print(f"{buf_start_x} {buf_start_y+i*lineheight} {buf_start_x+linewidth} {buf_start_y+i*lineheight}")

head = int(lines[1], 16)/8
tail = int(lines[2], 16)/8

print(f"newstring font Time-Roman fontsize 18 x {buf_start_x-15} y {buf_start_y+lineheight*head+2} hjc vjc :")
print("head -->")
print(f"newstring font Time-Roman fontsize 18 x {buf_start_x+linewidth+13} y {buf_start_y+lineheight*tail+2} hjc vjc :")
print("<-- tail")

string_begins = 4
for i in range(num_entries+1):
    print(f"newstring font Time-Roman fontsize 15 x {buf_start_x+4} y {buf_start_y+lineheight*i+2} hjl vjc :")
    print(lines[string_begins+i])
