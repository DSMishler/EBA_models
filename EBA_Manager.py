import os
import shutil
import subprocess

import random # for shuffling order of working node lists
import time # for sleeping and timestamping sent messages

# although not imported, this uses a lock from the threading module

NODEBUFDIRS_FNAME = "nodebufdirs"

class EBA_Manager:
    def __init__(self, mode, threading_lock):
        self.adj = None
        self.nodes = []
        self.running = True
        self.all_messages = []
        self.threading_lock = threading_lock
        if mode == "init":
            try:
                os.mkdir(NODEBUFDIRS_FNAME)
            except OSError:
                shutil.rmtree(NODEBUFDIRS_FNAME)
                os.mkdir(NODEBUFDIRS_FNAME)
        elif mode == "load":
            for n in os.listdir(NODEBUFDIRS_FNAME):
                self.load_node(n)

    # returns True is new_neighbor belongs to node write_to
    def read_neighbor(self, write_to, new_neighbor):
        node_dir_name = NODEBUFDIRS_FNAME + "/" + write_to
        os.chdir(node_dir_name)

        f = open("node_info.EBA", "r")
        info = eval(f.read())
        f.close()

        if new_neighbor in info["neighbors"]:
            ans = True
        else:
            ans = False

        os.chdir("../../")
        return ans

    # writes new_neighbor to node write_to's neighbors
    def write_neighbor(self, write_to, new_neighbor):
        node_dir_name = NODEBUFDIRS_FNAME + "/" + write_to
        os.chdir(node_dir_name)

        f = open("node_info.EBA", "r")
        info = eval(f.read())
        f.close()

        info["neighbors"].append(new_neigbhor)

        f = open("node_info.EBA", "w")
        f.write(repr(info))
        f.close()

        os.chdir("../../")

    def connect(self, node1, node2):
        with self.threading_lock:
            if self.connected(node1, node2):
                print(f"warning: nodes {node1} and {node2} already connected")
                print(f"ignoring.")
                return
            if node1 not in self.nodes:
                print(f"refusing to connect {node1} and {node2}.")
                print(f"{node1} not in {self.nodes}")
                return
            if node2 not in self.nodes:
                print(f"refusing to connect {node1} and {node2}.")
                print(f"{node2} not in {self.nodes}")
                return

            self.write_neighbor(node1, node2)
            self.write_neighbor(node2, node1)

    def connected(self, node1, node2):
        a_to_b = self.read_neighbor(node1, node2)
        b_to_a = self.read_neighbor(node2, node1)

        if a_to_b ^ b_to_a: # logical xor
            # one is connected to the other but not vise versa
            print(f"error, asymmetric connection detected")
            print(f"nodes producing error: {node1} and {node2}")
            return None
        elif not a_to_b and not b_to_a:
            return False
        elif a_to_b and b_to_a:
            return True
        else:
            print("unknown error")
            assert False

    def deliver_message(self, message_txt):
        message = eval(message_txt)
        sender = message["sender"]
        recipient = message["recipient"]
        if sender == "ROOT" or recipient == "ROOT":
            # no need to check if they are neigbhors, ROOT is involved.
            pass
        elif self.connected(sender, recipient):
            # all good here, nodes connected
            pass
        else:
            print(f"refusing to deliver message {message_txt}.")
            print(f"{sender} and {recipient} not connected.")
            return

        if recipient == "ROOT":
            print(message_txt)
        else:
            node_dir_name = NODEBUFDIRS_FNAME + "/" + recipient
            os.chdir(node_dir_name)
            f = open("message_queue.EBA", "a")
            f.write(message_txt)
            f.write('\n')
            f.close()
            os.chdir("../../")

        message["time"] = time.perf_counter()
        self.all_messages.append(message)

    def empty_queue(self, queue_text):
        for m in queue_text.split('\n'):
            if m != "":
                self.deliver_message(m)

    def new_node(self, node_name):
        with self.threading_lock:
            if node_name == "ROOT":
                print(f"refusing to make a node with reserved name")
                return # you can't name a node 'ROOT' - that's for the shell
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

    def node_has_work(self, node_name):
        node_dir_name = NODEBUFDIRS_FNAME + "/" + node_name
        os.chdir(node_dir_name)
        work = False
        f = open("call_queue.EBA", "r")
        if len(f.read()) > 0:
            work = True
        f.close()
        f = open("message_queue.EBA", "r")
        if len(f.read()) > 0:
            work = True
        f.close()
        os.chdir("../../")
        return work

    def run_node(self, node_name):
        node_dir_name = NODEBUFDIRS_FNAME + "/" + node_name
        os.chdir(node_dir_name)
        subprocess.run(["python3", "EBA_Node.py", "mode=load"])
        f = open("send_buf.EBA", "r")
        self.empty_queue(f.read())
        f.close()
        f = open("send_buf.EBA", "w")
        f.close()
        os.chdir("../../")

    def run_all(self):
        work_nodes = []
        for node in self.nodes:
            if self.node_has_work(node):
                work_nodes.append(node)

        random.shuffle(work_nodes)

        for node in work_nodes:
            self.run_node(node)

        return len(work_nodes)

    def run_continuously(self, sleep_time=2):
        while True:
            if self.running:
                # TODO: consider cleaning up this loop code
                run_again = True
                while run_again == True:
                    with self.threading_lock:
                        nodes_ran = self.run_all()
                        run_again = True if nodes_ran > 0 else False
            # print(f"going to sleep for {sleep_time}s")
            time.sleep(sleep_time)

    def deliver_shell_message(self, message):
        with self.threading_lock:
            self.deliver_message(message)


if __name__ == "__main__":
    Manager = EBA_Manager()
    Manager.new_node("node1")
