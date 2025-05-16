file_locs = {}

files_to_load = [
    "dfs/base.py",
    "dfs/dfs0_ping.py",
    "dfs/dfs1_spinlock.py",
    "dfs/dfs2_propagate.py",
    "dfs/dfs3_prop_spinlock.py",
    "dfs/dfs4_prop_prep_write.py",
    "dfs/dfs5_prop_write.py",
    "dfs/dfs6_am_i_leaf.py",
    "dfs/dfs7_update_parent.py",
    "dfs/dfs8_prep_prop_up.py"]

for file in files_to_load:
    file_locs[file[4:]] = shell_load_file(f"LOAD {file}")

inventory_dict = {"code": file_locs, "data": {}}
f = open("dfs_inventory.txt", "w")
f.write(repr(inventory_dict))
f.close()
inventory_buf = shell_load_file(f"LOAD dfs_inventory.txt size=2x")

shell_invoke(f"INVOKE PYEXEC {file_locs['base.py']} {inventory_buf}")
