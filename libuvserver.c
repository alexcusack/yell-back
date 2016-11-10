#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 8000
#define DEFAULT_BACKLOG 128

typedef struct { uv_loop_t *loop; } context;

typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
  write_req_t *wr = (write_req_t *)req;
  // free the buffer
  free(wr->buf.base);
  // free the pointer
  free(wr);
}

void alloc_new_buff(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size); // char pointer to malloced buffer
  buf->len = suggested_size; // save buffer length as part of the struct
}

void after_write(uv_write_t *req, int status) {
  if (status) {
    fprintf(stderr, "Write error occured%s\n", uv_strerror(status));
  }
  free_write_req(req); // free the req struct
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  // nread = number of bytes from client
  // buf = a struct. buf->base = a buffer of bytes received from client
  if (nread > 0) {
    char *uppercased = malloc(nread);
    for (int i = 0; i < nread; ++i) {
      uppercased[i] = toupper(buf->base[i]);
    }
    write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
    req->buf = uv_buf_init(uppercased, nread);
    uv_write((uv_write_t *)req, client, &req->buf, 1, after_write);
    return;
  }
  if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "Read error: %s\n", uv_err_name(nread));
    }
    uv_close((uv_handle_t *)client, NULL);
  }
  free(buf->base);
}

void on_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    return;
  }

  context *ctx = server->data;

  // allocate a client, add it to the loop, start reading
  uv_tcp_t *client = malloc(sizeof(*client));
  uv_tcp_init(ctx->loop, client);
  if (uv_accept(server, (uv_stream_t *)client) == 0) {
    uv_read_start((uv_stream_t *)client, alloc_new_buff, echo_read);
  } else {
    uv_close((uv_handle_t *)client, NULL);
  }
}

int main() {
  // "context" struct, share refs between fn calls by pointing handler's
  // .data to the address of this struct. add to this stuct anything that
  // must be shared
  context ctx;

  // make loop
  uv_loop_t *loop = uv_default_loop();
  ctx.loop = loop;

  // make server handler
  uv_tcp_t server;
  uv_tcp_init(loop, &server);

  // ensure server handler can use context struct
  server.data = &ctx;

  // set host & port on server
  struct sockaddr addr;
  uv_ip4_addr(DEFAULT_HOST, DEFAULT_PORT, (struct sockaddr_in *)&addr);
  uv_tcp_bind(&server, &addr, 0);

  // register connection callback for server
  int r = uv_listen((uv_stream_t *)&server, DEFAULT_BACKLOG, on_connection);

  if (r) {
    fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    return 1;
  }

  // start loop
  uv_run(loop, UV_RUN_DEFAULT);

  // we never get here, main never returns
}
