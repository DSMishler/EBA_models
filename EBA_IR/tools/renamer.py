import os
import sys

if len(sys.argv) < 2:
    print("error: required 2 args! usage: python renamer.py <target_dir>")
    exit()

os.chdir(sys.argv[1])
def rename_rec(spacing=0):
    files = os.scandir(".")
    spaces = " "*spacing

    for file in files:
        if file.is_dir():
            os.chdir(file.name)

            rename_rec(spacing+4)

            os.chdir("..")
        else:
            fname = file.name
            if fname[-4:] == ".EBA":
                main_name = fname[:-4]

                new_name = main_name+".EIR"

                os.replace(fname, new_name)
                print(f"replaced {fname} to {new_name}")

            else:
                pass
                # print(f"what is a non-dir, non-EBA/EIR file doing here ('{file.name}')?")

        # print(spaces+file.name)
        # print(spaces+str(file.is_dir()))
        # print()



rename_rec()
