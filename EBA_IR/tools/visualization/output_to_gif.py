import sys
import os

if len(sys.argv) < 2:
    print("usage: python output_to_jpgs.py <target>")
    exit(1)

ebaoutput_fname = sys.argv[1]

if ebaoutput_fname[-7:] != ".ebaout":
    print(f"only '.ebaout' files are allowed. You inputted {ebaoutput_fname}")
    exit(1)

base_fname = ebaoutput_fname[:-7]

f = open(f"ebaouts/{ebaoutput_fname}")
ftext = f.read()
f.close()

buffer_outputs = ftext.split("CURRENTLY_RUNNING_")[1:]
# each buffer that outputs something will start with this line
# but discard the first element, as '.split()' will always give you
# an element coming before the first 'CURRENTLY_RUNNING', but we
# only want what comes during/after each of these lines


imagemagick_convert_str = "convert -size 1080x1080 -delay 80 -loop 0"

displays = {}
displays["sched"] = {}
displays["sched"]["occupants"] = None
displays["sched"]["head"] = None
displays["sched"]["tail"] = None
displays["sched"]["name"] = "scheduler"
displays["read"] = {}
displays["read"]["occupants"] = None
displays["read"]["head"] = None
displays["read"]["tail"] = None
displays["read"]["name"] = "read queue"
displays["write"] = {}
displays["write"]["occupants"] = None
displays["write"]["head"] = None
displays["write"]["tail"] = None
displays["write"]["name"] = "write queue"

arg_bufs_to_names = {}
arg_bufs_to_names["0xDEADBEEF"] = "empty"

dims = {}
dims["sched"] = {}
dims["sched"]["lineheight"] = 5
dims["sched"]["linewidth"] = 100
dims["sched"]["start_x"] = -140
dims["sched"]["start_y"] = -35
dims["sched"]["num_entries"] = 25
dims["read"] = {}
dims["read"]["lineheight"] = 5
dims["read"]["linewidth"] = 50
dims["read"]["start_x"] = 0
dims["read"]["start_y"] = -35
dims["read"]["num_entries"] = 25
dims["write"] = {}
dims["write"]["lineheight"] = 5
dims["write"]["linewidth"] = 50
dims["write"]["start_x"] = 90
dims["write"]["start_y"] = -35
dims["write"]["num_entries"] = 25

for i in range(len(buffer_outputs)):
    jgr_fname = f"{base_fname}_{i}.jgr"
    jpg_fname = f"{base_fname}_{i}.jpg"

    jgr_f = open(f"jgrs/{jgr_fname}", "w")
    
    lines = buffer_outputs[i].split('\n')

    for line_num in range(len(lines)):
        line = lines[line_num]
        if line[:10] == "TRANSLATE_":
            # print(f"{line} is a translate line")
            arg_bufs_to_names[lines[line_num+1]] = line[10:]
        elif line == "DISPLAY_SCHEDULE_QUEUE":
            displays["sched"]["head"] = int(lines[line_num+2], 16)/8
            displays["sched"]["tail"] = int(lines[line_num+3], 16)/8
            displays["sched"]["occupants"] = lines[line_num+5:line_num+5+dims['sched']['num_entries']]
        elif line == "DISPLAY_READ_QUEUE":
            displays["read"]["head"] = int(lines[line_num+2], 16)/8
            displays["read"]["tail"] = int(lines[line_num+3], 16)/8
            displays["read"]["occupants"] = lines[line_num+5:line_num+5+dims['read']['num_entries']]
        elif line == "DISPLAY_WRITE_QUEUE":
            displays["write"]["head"] = int(lines[line_num+2], 16)/8
            displays["write"]["tail"] = int(lines[line_num+3], 16)/8
            displays["write"]["occupants"] = lines[line_num+5:line_num+5+dims['write']['num_entries']]


    jgr_f.write("newgraph\n")
    jgr_f.write("xaxis min -70 max 70 nodraw\n")
    jgr_f.write("yaxis min -20 max 20 nodraw\n")
    jgr_f.write("newstring font Time-Roman fontsize 25 x 0 y 95 hjc vjc :\n")
    jgr_f.write(f"currently running: {lines[0]}\n")

    jgr_f.write(f"newstring font Time-Roman fontsize 18 x {-200} y {0} hjc vjc :\n")
    jgr_f.write("l\n")
    jgr_f.write(f"newstring font Time-Roman fontsize 18 x {200} y {0} hjc vjc :\n")
    jgr_f.write("l\n")
    jgr_f.write(f"newstring font Time-Roman fontsize 18 x {0} y {-60} hjc vjc :\n")
    jgr_f.write("l\n")


    for d in dims.values():
        jgr_f.write("newline poly pfill 1 pts\n")
        jgr_f.write(f"{d['start_x']} {d['start_y']} {d['start_x']} {d['start_y']+d['num_entries']*d['lineheight']}\n")
        jgr_f.write("newline poly pfill 1 pts\n")
        jgr_f.write(f"{d['start_x']+d['linewidth']} {d['start_y']} {d['start_x']+d['linewidth']} {d['start_y']+d['num_entries']*d['lineheight']}\n")
    
        for entry in range(d['num_entries']+1):
            jgr_f.write("newline poly pfill 1 pts\n")
            jgr_f.write(f"{d['start_x']} {d['start_y']+entry*d['lineheight']} {d['start_x']+d['linewidth']} {d['start_y']+entry*d['lineheight']}\n")
    


    for key in displays:
        d = dims[key]
        t = displays[key]
        if t["occupants"] is not None:
            # Assume head and tail are also not none
            assert t["head"] is not None
            assert t["tail"] is not None

            jgr_f.write(f"newstring font Time-Roman fontsize 18 x {d['start_x']-8} y {d['start_y']+d['lineheight']*t['head']+2} hjc vjc :\n")
            jgr_f.write("h->\n")
            jgr_f.write(f"newstring font Time-Roman fontsize 18 x {d['start_x']+d['linewidth']+6} y {d['start_y']+d['lineheight']*t['tail']+2} hjc vjc :\n")
            jgr_f.write("<-t\n")

            for occ in range(len(t['occupants'])):
                if t['occupants'][occ] in arg_bufs_to_names:
                    print_str = arg_bufs_to_names[t['occupants'][occ]]
                else:
                    print_str = t['occupants'][occ]
                jgr_f.write(f"newstring font Time-Roman fontsize 15 x {d['start_x']+4} y {d['start_y']+d['lineheight']*occ+2} hjl vjc :\n")
                jgr_f.write(f"{print_str}\n")
        jgr_f.write(f"newstring font Time-Roman fontsize 20 x {d['start_x']+d['linewidth']/2} y {d['start_y']-5} hjc vjc :\n")
        jgr_f.write(f"{t['name']}\n")



    jgr_f.close()

    os.system(f"python3 jgr_to_jpg.py {jgr_fname}")
    print(f"written jpgs/{jpg_fname} ({i+1} of {len(buffer_outputs)})")
    # adding each individually instead of all at once with "*". This
    # ensures we only use the ones we parsed and not some other stuff
    # sitting around from a different run
    imagemagick_convert_str += f" jpgs/{jpg_fname}"

imagemagick_convert_str += f" gifs/{base_fname}.gif"

print(f"running: {imagemagick_convert_str}")
os.system(imagemagick_convert_str)


