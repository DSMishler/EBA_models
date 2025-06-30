circ_buf = shell_return_bufreq(f"BUFREQ 100")

code_buf = shell_load_file(f"LOAD circular_buffer/init.py")
write_buf = shell_load_file(f"LOAD circular_buffer/write.py")
read_buf = shell_load_file(f"LOAD circular_buffer/read.py")
junk_buf = shell_load_file(f"LOAD circular_buffer/ts.txt")

shell_invoke(f"INVOKE PYEXEC {code_buf} 500 {circ_buf}")

shell_invoke(f"INVOKE PYEXEC {write_buf} {circ_buf} {code_buf} 50")
shell_invoke(f"INVOKE PYEXEC {write_buf} {circ_buf} {code_buf} 50")
shell_invoke(f"INVOKE PYEXEC {read_buf} {circ_buf} {junk_buf} 60")
shell_invoke(f"INVOKE PYEXEC {write_buf} {circ_buf} {code_buf} 460")
shell_invoke(f"INVOKE PYEXEC {read_buf} {circ_buf} {junk_buf} 470")
