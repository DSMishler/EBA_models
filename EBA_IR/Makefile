test_glfw: test.c reader.c reader.h interpreter.c interpreter.h
	gcc -Wall -Wpedantic -pthread -g -I . -o test reader.c test.c interpreter.c

test: test.c reader.c reader.h interpreter.c interpreter.h
	gcc -Wall -Wpedantic -pthread -g -I . -o test reader.c test.c interpreter.c

clean:
	rm test
