libuvserver: libuvserver.c
	cc -g -Wall $< -o $@ -I/usr/local/include  /usr/local/lib/libuv.a

clean:
	rm -rf libuvserver libuvserver.dSYM

.PHONY: clean
