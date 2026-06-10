w_glfw: test.c reader.c reader.h interpreter.c interpreter.h
	gcc -Wall -Wpedantic -pthread -g -I ./tools/graphics/glad/include -o test ./tools/graphics/glad/src/gl.c reader.c test.c interpreter.c -lglfw -lGL

test: test.c reader.c reader.h interpreter.c interpreter.h
	gcc -Wall -Wpedantic -pthread -g -I . -o test reader.c test.c interpreter.c


clean:
	rm test
