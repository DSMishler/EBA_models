buf = shell_load_file(f"LOAD testcode.py")

shell_invoke(f"INVOKE PYEXEC {buf}")
