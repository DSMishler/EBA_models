all: modules EIRtools EIRtest EBAtest

EBAtest: eba.c
	gcc -g -Wall -Wpedantic -rdynamic -pthread -o EBAtest eba.c

EIRtest: test.c reader.c reader.h interpreter.c interpreter.h
	gcc -g -I. -Wall -Wpedantic -rdynamic -pthread -o EIRtest reader.c test.c interpreter.c

modules: libs/bufreq.so libs/memop.so libs/invoke.so libs/mathop.so libs/cmp.so libs/print.so libs/log.so libs/scaffold.so

libs/bufreq.so: modules/bufreq.c
	gcc -g -I. -Wpedantic -Wall -rdynamic -pthread -fpic -shared -o libs/bufreq.so modules/bufreq.c

libs/memop.so: modules/memop.c
	gcc -g -I. -Wpedantic -Wall -rdynamic -pthread -fpic -shared -o libs/memop.so modules/memop.c

libs/invoke.so: modules/invoke.c
	gcc -g -I. -Wpedantic -Wall -rdynamic -pthread -fpic -shared -o libs/invoke.so modules/invoke.c

libs/mathop.so: modules/mathop.c
	gcc -g -I. -Wpedantic -Wall -rdynamic -pthread -fpic -shared -o libs/mathop.so modules/mathop.c

libs/cmp.so: modules/cmp.c
	gcc -g -I. -Wpedantic -Wall -rdynamic -pthread -fpic -shared -o libs/cmp.so modules/cmp.c

libs/print.so: modules/print.c
	gcc -g -I. -Wpedantic -Wall -rdynamic -pthread -fpic -shared -o libs/print.so modules/print.c

libs/log.so: modules/log.c
	gcc -g -I. -Wpedantic -Wall -rdynamic -pthread -fpic -shared -o libs/log.so modules/log.c

libs/scaffold.so: modules/scaffold.c
	gcc -g -I. -Wpedantic -Wall -rdynamic -pthread -I ./tools/graphics/glad/include -fpic -shared -o libs/scaffold.so modules/scaffold.c ./tools/graphics/glad/src/gl.c -lglfw -lGL

EIRtools: libs/EIRtest.so

libs/EIRtest.so: test.c reader.c reader.h interpreter.c interpreter.h
	gcc -g -I. -Wall -Wpedantic -rdynamic -pthread -fpic -shared -o libs/EIRtest.so reader.c test.c interpreter.c


clean:
	rm EIRtest EBAtest libs/*.so
