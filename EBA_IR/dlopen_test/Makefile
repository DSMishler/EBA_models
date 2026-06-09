main: main.c
	gcc -Wpedantic -Wall -o test main.c -rdynamic -ldl

test1: targetfunc.c
	gcc -o targetfunc.so -rdynamic -fPIC -shared targetfunc.c

test2: untargetfunc.c
	gcc -o targetfunc.so -rdynamic -fPIC -shared untargetfunc.c
