libuvserver: libuvserver.c
	cc -g -Wall $< -o $@ -I/usr/local/include  /usr/local/lib/libuv.a

clean:
	rm libuvserver
	rm libuvserver.dSYM

.PHONY: clean
