import sys

if len(sys.argv) < 2:
    print("usage: python lscpu_to_xml.py <fname>")
    exit(0)


# else

in_f = open(sys.argv[1])
ftext = in_f.read()
in_f.close()

flines = ftext.split('\n')

target_fname = "lscpu.xml"

out_f = open(target_fname, "w")


for l in flines:
    if len(l.split(':')) < 2:
        continue
    attr = l.split(':')[0]
    val = ":".join(l.split(':')[1:]).strip()
    out_f.write(f"<{attr}>\n")
    out_f.write(f"    {val}\n")
    out_f.write(f"</{attr}>\n")
