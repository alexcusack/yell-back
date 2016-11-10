### Yell back
#### A simple echo server built on libuv

Creates a sever and listens at `127.0.0.1:8000`. Connecting to the server will create a readable/writable socket that remains open until the client disconnects (or an error occurs). Messages sent by the client are yelled back (uppercased).

#### Run:

```
$ brew install libuv
$ make
$ ./libuvserver
```
In  a separate session
```
$ nc 127.0.0.1 8000
> hello
HELLO
> cats pajamas
CATS PAJAMAS
```
