import os
import shutil
import subprocess

NODEBUFDIRS_FNAME = "nodebufdirs"

class EBA_Manager:
    def __init__(self, mode):
        self.adj = None
        self.nodes = []
        if mode == "init":
            try:
                os.mkdir(NODEBUFDIRS_FNAME)
            except OSError:
                shutil.rmtree(NODEBUFDIRS_FNAME)
                os.mkdir(NODEBUFDIRS_FNAME)
        elif mode == "load":
            for n in os.listdir(NODEBUFDIRS_FNAME):
                self.load_node(n)

    def new_node(self, node_name):
        node_dir_name = NODEBUFDIRS_FNAME + "/" + node_name
        os.mkdir(node_dir_name)


        subprocess.run(["cp", "EBA_Node.py", node_dir_name])

        os.chdir(node_dir_name)
        subprocess.run(["python3", "EBA_Node.py", "mode=init", f"name={node_name}"])
        os.chdir("../../")

        self.nodes.append(node_name)

    def load_node(self, node_name):
        self.nodes.append(node_name)

    def show_node(self, node_name):
        node_dir_name = NODEBUFDIRS_FNAME + "/" + node_name
        os.chdir(node_dir_name)
        subprocess.run(["python3", "EBA_Node.py", "mode=show"])
        os.chdir("../../")

if __name__ == "__main__":
    Manager = EBA_Manager()
    Manager.new_node("node1")
