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
        self.exit = False
        self.all_messages = []
        self.major_iteration = 0
        self.minor_iteration = 0
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

    def pause(self):
        with self.threading_lock:
            self.running = False

    def resume(self):
        with self.threading_lock:
            self.running = True

    def set_exit(self):
        with self.threading_lock:
            self.exit = True
        
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

        info["neighbors"].append(new_neighbor)

        f = open("node_info.EBA", "w")
        f.write(repr(info))
        f.close()

        os.chdir("../../")

    # ensures rm_neighbor is removes from node write_to's neighbors
    def unwrite_neighbor(self, write_to, rm_neighbor):
        node_dir_name = NODEBUFDIRS_FNAME + "/" + write_to
        os.chdir(node_dir_name)

        f = open("node_info.EBA", "r")
        info = eval(f.read())
        f.close()

        info["neighbors"].remove(rm_neighbor)

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

    def disconnect(self, node1, node2):
        with self.threading_lock:
            if not self.connected(node1, node2):
                print(f"warning: nodes {node1} and {node2} not connected")
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

            self.unwrite_neighbor(node1, node2)
            self.unwrite_neighbor(node2, node1)

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

    def adj_matrix(self):
        # create an adjacency matrix of nodes
        adj_size = len(self.nodes)
        adj = [[0 for i in self.nodes] for j in self.nodes]
        for i in range(len(self.nodes)):
            for j in range(i, len(self.nodes)):
                if i == j:
                    adj[i][j] = 1
                elif self.connected(self.nodes[i], self.nodes[j]):
                    adj[i][j] = 1
                    adj[j][i] = 1
        return adj

    def deliver_message(self, message_txt):
        message = eval(message_txt)
        sender = message["sender"]
        recipient = message["recipient"]
        if sender == "ROOT" or recipient == "ROOT":
            # no need to check if they are neighbors, ROOT is involved.
            pass
        elif self.connected(sender, recipient):
            # all good here, nodes connected
            pass
        else:
            print(f"refusing to deliver message {message_txt}.")
            print(f"{sender} and {recipient} not connected.")
            return

        if recipient == "ROOT":
            # write it to the shell pipe
            # no need to lock, since this is only called in `run_all` which
            # is already protected by the threading_lock.
            f = open("manager_shell_pipe.txt", "a")
            f.write(message_txt)
            f.write('\n')
            f.close()
        else:
            node_dir_name = NODEBUFDIRS_FNAME + "/" + recipient
            os.chdir(node_dir_name)
            f = open("message_queue.EBA", "a")
            f.write(message_txt)
            f.write('\n')
            f.close()
            os.chdir("../../")

        message["time"] = time.perf_counter()
        message["major"] = self.major_iteration
        message["minor"] = self.minor_iteration
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

    def show_node(self, node_name, contents=False):
        with self.threading_lock:
            node_dir_name = NODEBUFDIRS_FNAME + "/" + node_name
            os.chdir(node_dir_name)
            subprocess.run(["python3", "EBA_Node.py", "mode=show", f"contents={contents}"])
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
        queue_text = f.read()
        f.close()
        f = open("send_buf.EBA", "w")
        f.close()
        os.chdir("../../")
        self.empty_queue(queue_text)
        self.minor_iteration += 1

    def run_all(self):
        work_nodes = []
        for node in self.nodes:
            if self.node_has_work(node):
                work_nodes.append(node)

        random.shuffle(work_nodes)

        for node in work_nodes:
            self.run_node(node)

        self.major_iteration += 1
        return len(work_nodes)

    def run_continuously(self, sleep_time=0.2):
        while True:
            if self.running:
                while True:
                    with self.threading_lock:
                        nodes_ran = self.run_all()
                        if nodes_ran == 0:
                            break
            # print(f"going to sleep for {sleep_time}s")
            time.sleep(sleep_time)
            if self.exit == True:
                print("exiting")
                return

    def deliver_shell_message(self, message):
        with self.threading_lock:
            self.deliver_message(message)


if __name__ == "__main__":
    Manager = EBA_Manager()
    Manager.new_node("node1")
