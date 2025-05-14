file_locs = {}

files_to_load = [
    "dfs/base.py"]

for file in files_to_load:
    file_locs[file] = shell_load_file(f"LOAD {file}")

shell_invoke(f"INVOKE PYEXEC {file_locs['dfs/base.py']} arg2")
