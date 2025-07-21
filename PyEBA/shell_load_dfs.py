file_locs = {}

files_to_load = [
    "dfs/dfs0_ping.py",
    "dfs/dfs1_spinlock.py",
    "dfs/dfs2_propagate.py",
    "dfs/dfs3_prop_spinlock.py",
    "dfs/dfs4_prop_prep_write.py",
    "dfs/dfs5_prop_write.py",
    "dfs/dfs6_am_i_leaf.py",
    "dfs/dfs7_update_parent.py",
    "dfs/dfs8_prep_prop_up.py",
    "dfs/dfs9_prop_up_spinlock.py",
    "dfs/dfs10_prop_up_write.py"]

for file in files_to_load:
    file_locs[file[4:]] = shell_load_file(f"LOAD {file}")

inventory_dict = {"code": file_locs, "data": {}}
f = open("dfs_inventory.txt", "w")
f.write(repr(inventory_dict))
f.close()
inventory_buf = shell_load_file(f"LOAD dfs_inventory.txt size=2x")
base_buf = shell_load_file(f"LOAD dfs/base.py")
close_buf = shell_load_file(f"LOAD dfs/close.py")

shell_invoke(f"INVOKE PYEXEC {base_buf} {inventory_buf} {close_buf}")
