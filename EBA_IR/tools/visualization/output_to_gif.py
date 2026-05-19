import sys
import os

w_python = "python3"

if len(sys.argv) < 2:
    print(f"usage: {w_python} output_to_jpgs.py <target>")
    exit(1)

ebaoutput_fname = sys.argv[1]

if ebaoutput_fname[-7:] != ".ebaout":
    print(f"only '.ebaout' files are allowed. You inputted {ebaoutput_fname}")
    exit(1)

base_fname = ebaoutput_fname[:-7]
while True:
    try:
        base_fname = base_fname[base_fname.index('/')+1:]
    except ValueError:
        break

f = open(f"{ebaoutput_fname}")
ftext = f.read()
f.close()

if os.getcwd() == "/home/danielmishler/Mishler_LL_work/EBA_Work/EBA_models/EBA_IR/tools/visualization":
    if len(os.listdir(os.getcwd()+"/jgrs/")) > 0:
        os.system("rm jgrs/*.jgr")
    if len(os.listdir(os.getcwd()+"/jpgs/")) > 0:
        os.system("rm jpgs/*.jpg") # clean up old runs to avoid difficult to track  errors
else:
    print(f"warning, this code has specific instructions to be run in:\n")
    print("/home/danielmishler/Mishler_LL_work/EBA_Work/EBA_models/EBA_IR/tools/visualization\n")
    print(f"but it is currently being run in {os.getcwd()}\n")

buffer_outputs = ftext.split("CURRENTLY_RUNNING_")[1:]
# each buffer that outputs something will start with this line
# but discard the first element, as '.split()' will always give you
# an element coming before the first 'CURRENTLY_RUNNING', but we
# only want what comes during/after each of these lines


imagemagick_convert_str = "convert -size 1080x1080 -delay 80 -loop 0"

sdisplays = {}
sdisplays["sched"] = {}
sdisplays["sched"]["occupants"] = None
sdisplays["sched"]["head"] = None
sdisplays["sched"]["tail"] = None
sdisplays["sched"]["name"] = "scheduler"
sdisplays["read"] = {}
sdisplays["read"]["occupants"] = None
sdisplays["read"]["head"] = None
sdisplays["read"]["tail"] = None
sdisplays["read"]["name"] = "read queue"
sdisplays["write"] = {}
sdisplays["write"]["occupants"] = None
sdisplays["write"]["head"] = None
sdisplays["write"]["tail"] = None
sdisplays["write"]["name"] = "write queue"

cdisplays = {}
cdisplays["cbuf"] = {}
cdisplays["cbuf"]["occupants"] = None
cdisplays["cbuf"]["head"] = None
cdisplays["cbuf"]["tail"] = None
cdisplays["cbuf"]["name"] = "circ buf 1"

arg_bufs_to_names = {}
arg_bufs_to_names["0xDEADBEEF"] = "empty"

sdims = {}
sdims["sched"] = {}
sdims["sched"]["lineheight"] = 5
sdims["sched"]["linewidth"] = 100
sdims["sched"]["start_x"] = -140
sdims["sched"]["start_y"] = -35
sdims["sched"]["num_entries"] = 25
sdims["read"] = {}
sdims["read"]["lineheight"] = 5
sdims["read"]["linewidth"] = 50
sdims["read"]["start_x"] = 0
sdims["read"]["start_y"] = -35
sdims["read"]["num_entries"] = 25
sdims["write"] = {}
sdims["write"]["lineheight"] = 5
sdims["write"]["linewidth"] = 50
sdims["write"]["start_x"] = 90
sdims["write"]["start_y"] = -35
sdims["write"]["num_entries"] = 25
cdims = {}
cdims["cbuf"] = {}
cdims["cbuf"]["lineheight"] = 5
cdims["cbuf"]["linewidth"] = 14
cdims["cbuf"]["start_x"] = -140
cdims["cbuf"]["start_y"] = 40
cdims["cbuf"]["num_xcells"] = 20
cdims["cbuf"]["num_ycells"] = 10

for i in range(len(buffer_outputs)):
    sched_jgr_fname = f"sched_{base_fname}_{i}.jgr"
    sched_jpg_fname = f"sched_{base_fname}_{i}.jpg"
    buf_jgr_fname = f"buf_{base_fname}_{i}.jgr"
    buf_jpg_fname = f"buf_{base_fname}_{i}.jpg"

    both_jpg_fname = f"{base_fname}_{i}.jpg"

    sched_jgr_f = open(f"jgrs/{sched_jgr_fname}", "w")
    buf_jgr_f = open(f"jgrs/{buf_jgr_fname}", "w")
    
    lines = buffer_outputs[i].split('\n')

    for line_num in range(len(lines)):
        line = lines[line_num]
        if line[:10] == "TRANSLATE_":
            # print(f"{line} is a translate line")
            arg_bufs_to_names[lines[line_num+1]] = line[10:]
        elif line == "DISPLAY_SCHEDULE_QUEUE":
            sdisplays["sched"]["head"] = int(lines[line_num+2], base=16)/8
            sdisplays["sched"]["tail"] = int(lines[line_num+3], base=16)/8
            sdisplays["sched"]["occupants"] = lines[line_num+5:line_num+5+sdims['sched']['num_entries']]
        elif line == "DISPLAY_READ_QUEUE":
            sdisplays["read"]["head"] = int(lines[line_num+2], base=16)/8
            sdisplays["read"]["tail"] = int(lines[line_num+3], base=16)/8
            sdisplays["read"]["occupants"] = lines[line_num+5:line_num+5+sdims['read']['num_entries']]
        elif line == "DISPLAY_WRITE_QUEUE":
            sdisplays["write"]["head"] = int(lines[line_num+2], base=16)/8
            sdisplays["write"]["tail"] = int(lines[line_num+3], base=16)/8
            sdisplays["write"]["occupants"] = lines[line_num+5:line_num+5+sdims['write']['num_entries']]
        elif line == "DISPLAY_CIRC_BUF":
            cdisplays["cbuf"]["head"] = int(lines[line_num+2], base=16)
            cdisplays["cbuf"]["tail"] = int(lines[line_num+3], base=16)
            start_occ_ln = line_num+5
            end_occ_ln = line_num+5+cdims['cbuf']['num_xcells']*cdims['cbuf']['num_ycells']
            cdisplays["cbuf"]["occupants"] = lines[start_occ_ln:end_occ_ln]


    sched_jgr_f.write("newgraph\n")
    sched_jgr_f.write("xaxis min -70 max 70 nodraw\n")
    sched_jgr_f.write("yaxis min -20 max 20 nodraw\n")
    sched_jgr_f.write("newstring font Time-Roman fontsize 25 x 0 y 95 hjc vjc :\n")
    sched_jgr_f.write(f"currently running: {lines[0]}\n")

    sched_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {-200} y {0} hjc vjc :\n")
    sched_jgr_f.write("l\n")
    sched_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {200} y {0} hjc vjc :\n")
    sched_jgr_f.write("l\n")
    sched_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {0} y {-60} hjc vjc :\n")
    sched_jgr_f.write("l\n")


    buf_jgr_f.write("newgraph\n")
    buf_jgr_f.write("xaxis min -70 max 70 nodraw\n")
    buf_jgr_f.write("yaxis min -20 max 20 nodraw\n")
    buf_jgr_f.write("newstring font Time-Roman fontsize 25 x 0 y 95 hjc vjc :\n")
    buf_jgr_f.write(f"Status of buffer:\n")

    buf_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {-200} y {0} hjc vjc :\n")
    buf_jgr_f.write("l\n")
    buf_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {200} y {0} hjc vjc :\n")
    buf_jgr_f.write("l\n")
    buf_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {0} y {-60} hjc vjc :\n")
    buf_jgr_f.write("l\n")

    # boxes for the sched side
    for d in sdims.values():
        sched_jgr_f.write("newline poly pfill 1 pts\n")
        sched_jgr_f.write(f"{d['start_x']} {d['start_y']} {d['start_x']} {d['start_y']+d['num_entries']*d['lineheight']}\n")
        sched_jgr_f.write("newline poly pfill 1 pts\n")
        sched_jgr_f.write(f"{d['start_x']+d['linewidth']} {d['start_y']} {d['start_x']+d['linewidth']} {d['start_y']+d['num_entries']*d['lineheight']}\n")
    
        for entry in range(d['num_entries']+1):
            sched_jgr_f.write("newline poly pfill 1 pts\n")
            sched_jgr_f.write(f"{d['start_x']} {d['start_y']+entry*d['lineheight']} {d['start_x']+d['linewidth']} {d['start_y']+entry*d['lineheight']}\n")
    
    # boxes for the cbuf side
    for d in cdims.values():
        # for now, it's just the one cbuf. But it'll also help us shorthand
        for yi in range(d['num_xcells']+1):
            # drawing all the y-lines
            buf_jgr_f.write("newline poly pfill 1 pts\n")
            buf_jgr_f.write(f"{d['start_x']+yi*d['linewidth']} {d['start_y']} {d['start_x']+yi*d['linewidth']} {d['start_y']+d['num_ycells']*d['lineheight']}\n")
        for xi in range(d['num_ycells']+1):
            # drawing all the x-lines
            buf_jgr_f.write("newline poly pfill 1 pts\n")
            buf_jgr_f.write(f"{d['start_x']} {d['start_y']+xi*d['lineheight']} {d['start_x']+d['num_xcells']*d['linewidth']} {d['start_y']+xi*d['lineheight']}\n")
        



    # putting in the text for the sched queues
    for key in sdisplays:
        d = sdims[key]
        t = sdisplays[key]
        if t["occupants"] is not None:
            # Assume head and tail are also not none
            assert t["head"] is not None
            assert t["tail"] is not None

            sched_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {d['start_x']-8} y {d['start_y']+d['lineheight']*t['head']+2} hjc vjc :\n")
            sched_jgr_f.write("h->\n")
            sched_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {d['start_x']+d['linewidth']+6} y {d['start_y']+d['lineheight']*t['tail']+2} hjc vjc :\n")
            sched_jgr_f.write("<-t\n")

            for occ in range(len(t['occupants'])):
                if t['occupants'][occ] in arg_bufs_to_names:
                    print_str = arg_bufs_to_names[t['occupants'][occ]]
                else:
                    print_str = t['occupants'][occ]
                sched_jgr_f.write(f"newstring font Time-Roman fontsize 15 x {d['start_x']+4} y {d['start_y']+d['lineheight']*occ+2} hjl vjc :\n")
                sched_jgr_f.write(f"{print_str}\n")
        sched_jgr_f.write(f"newstring font Time-Roman fontsize 20 x {d['start_x']+d['linewidth']/2} y {d['start_y']-5} hjc vjc :\n")
        sched_jgr_f.write(f"{t['name']}\n")

    # putting in the contents of the circ buf
    for key in cdisplays:
        # should just be cbuf for now
        d = cdims[key]
        t = cdisplays[key]
        if t["occupants"] is not None:
            # Assume head and tail are also not none
            assert t["head"] is not None
            assert t["tail"] is not None

            def to_xcoord(bnum):
                return (d["start_x"] + (bnum % d["num_xcells"])*d['linewidth'] + d["linewidth"]//2)
            def to_ycoord(bnum):
                return (d["start_y"] + (d["num_ycells"]-1)*d["lineheight"] - (bnum // d["num_xcells"])*d['lineheight'] + d["lineheight"]//2)

            # buf_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {to_xcoord(t['head'])} y {to_ycoord(t['head'])} hjc vjc :\n")
            # buf_jgr_f.write("*h*\n")
            # buf_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {to_xcoord(t['tail'])} y {to_ycoord(t['tail'])} hjc vjc :\n")
            # buf_jgr_f.write("*t*\n")
            buf_jgr_f.write("newline poly color 0.0 1.0 0.0 pfill 1 pts\n")
            buf_jgr_f.write(f"{-3+to_xcoord(t['head'])+2} {to_ycoord(t['head'])}\
                              {-3+to_xcoord(t['head'])} {to_ycoord(t['head'])+2}\
                              {-3+to_xcoord(t['head'])-2} {to_ycoord(t['head'])}\
                              {-3+to_xcoord(t['head'])} {to_ycoord(t['head'])-2}\n")
            buf_jgr_f.write("newline poly color 0.0 0.0 1.0 pfill 1 pts\n")
            buf_jgr_f.write(f"{3+to_xcoord(t['tail'])+2} {to_ycoord(t['tail'])}\
                              {3+to_xcoord(t['tail'])} {to_ycoord(t['tail'])+2}\
                              {3+to_xcoord(t['tail'])-2} {to_ycoord(t['tail'])}\
                              {3+to_xcoord(t['tail'])} {to_ycoord(t['tail'])-2}\n")

            for occ in range(len(t['occupants'])):
                text = t['occupants'][occ]
                byte = text.split()[-1]
                char = chr(int(byte, base=16))

                buf_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {to_xcoord(occ)} y {to_ycoord(occ)} hjc vjc :\n")
                buf_jgr_f.write(f"{char}\n")

        buf_jgr_f.write(f"newstring font Time-Roman fontsize 20 x {d['start_x']+(d['linewidth']*d['num_xcells'])//2} y {d['start_y']-5} hjc vjc :\n")
        buf_jgr_f.write(f"{t['name']}\n")

        legend_head_xcoord = d['start_x']+(d['linewidth']*d['num_xcells'])//2
        legend_head_ycoord = d['start_y']-15
        legend_tail_xcoord = d['start_x']+(d['linewidth']*d['num_xcells'])//2
        legend_tail_ycoord = d['start_y']-20

        buf_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {legend_head_xcoord+10} y {legend_head_ycoord} hjc vjc :\n")
        buf_jgr_f.write(f"head\n")

        buf_jgr_f.write("newline poly color 0.0 1.0 0.0 pfill 1 pts\n")
        buf_jgr_f.write(f"{-5+legend_head_xcoord+2} {legend_head_ycoord}\
                          {-5+legend_head_xcoord} {legend_head_ycoord+2}\
                          {-5+legend_head_xcoord-2} {legend_head_ycoord}\
                          {-5+legend_head_xcoord} {legend_head_ycoord-2}\n")

        buf_jgr_f.write(f"newstring font Time-Roman fontsize 18 x {legend_tail_xcoord+10} y {legend_tail_ycoord} hjc vjc :\n")
        buf_jgr_f.write(f"tail\n")

        buf_jgr_f.write("newline poly color 0.0 0.0 1.0 pfill 1 pts\n")
        buf_jgr_f.write(f"{-5+legend_tail_xcoord+2} {legend_tail_ycoord}\
                          {-5+legend_tail_xcoord} {legend_tail_ycoord+2}\
                          {-5+legend_tail_xcoord-2} {legend_tail_ycoord}\
                          {-5+legend_tail_xcoord} {legend_tail_ycoord-2}\n")





    sched_jgr_f.close()
    buf_jgr_f.close()

    os.system(f"{w_python} jgr_to_jpg.py {sched_jgr_fname}")
    os.system(f"{w_python} jgr_to_jpg.py {buf_jgr_fname}")
    print(f"written jpgs/{sched_jpg_fname}, jpgs/{buf_jpg_fname} ({i+1} of {len(buffer_outputs)})")
    # adding each individually instead of all at once with "*". This
    # ensures we only use the ones we parsed and not some other stuff
    # sitting around from a different run
    os.system(f"convert jpgs/{sched_jpg_fname} jpgs/{buf_jpg_fname} +append jpgs/{both_jpg_fname}")
    imagemagick_convert_str += f" jpgs/{both_jpg_fname}"

imagemagick_convert_str += f" gifs/{base_fname}.gif"

print(f"running: {imagemagick_convert_str}")
os.system(imagemagick_convert_str)


