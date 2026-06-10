w_glfw: test.c reader.c reader.h interpreter.c interpreter.h
	gcc -Wall -Wpedantic -rdynamic -pthread -g -I ./tools/graphics/glad/include -o test ./tools/graphics/glad/src/gl.c reader.c test.c interpreter.c -lglfw -lGL

test: test.c reader.c reader.h interpreter.c interpreter.h
	gcc -Wall -Wpedantic -pthread -g -I . -o test reader.c test.c interpreter.c

symbols: modules/scaffold.c
	gcc -g -I. -rdynamic -pthread -I ./tools/graphics/glad/include -fpic -shared -o libs/scaffold.so modules/scaffold.c ./tools/graphics/glad/src/gl.c

clean:
	rm test
