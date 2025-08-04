test: test.c EBA_buf.c EBA_buf.h EBA_invoke.c EBA_invoke.h circular_buffer.h circular_buffer.c
	gcc -Wall -Wpedantic -g -I . -o test EBA_buf.c EBA_invoke.c circular_buffer.c test.c

clean:
	rm test
