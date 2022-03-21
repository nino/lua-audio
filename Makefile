all:
	clang audio.c -o audio -llua -I/usr/local/include/lua

lib:
	clang audio.c -shared -o libaudio.so -fPIC -llua -I/usr/local/include/lua
