file_locs = {}

files_to_load = [
    "dfs/base.py",
    "dfs/dfs0_ping.py",
    "dfs/dfs1_spinlock.py",
    "dfs/dfs2_propagate.py",
    "dfs/dfs3_prop_spinlock.py",
    "dfs/dfs4_prop_prep_write.py",
    "dfs/dfs5_prop_write.py"]

for file in files_to_load:
    file_locs[file[4:]] = shell_load_file(f"LOAD {file}")

f = open("dfs_inventory.txt", "w")
f.write(repr(file_locs))
f.close()
inventory_buf = shell_load_file(f"LOAD dfs_inventory.txt")

shell_invoke(f"INVOKE PYEXEC {file_locs['base.py']} {inventory_buf}")
