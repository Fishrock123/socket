#include <assert.h>
#include <uv.h>
#include <bob.h>
#include "server.h"
#include "socket.h"

#define DEFAULT_BACKLOG 128

Server::Server(uv_loop_t *loop)
    : loop_(loop),
      tcp_(new uv_tcp_t)
{
  uv_tcp_init(loop, tcp_);
  tcp_->data = this;
}

Server::~Server() {
  delete &addr_;
  delete &tcp_;
}

void Server::Listen(sockaddr* addr, void* data, server_connection_cb_t callback) {
  addr_ = addr;

  server_connection_cb_data_t* cb_data = new server_connection_cb_data_t;
  cb_data->data = data;
  cb_data->callback = callback;

  connection_cb_data_ = cb_data;

  int bind_status = uv_tcp_bind(tcp_, addr, 0);
  if (bind_status != 0) {
    printf("Bind Error: %s - %s\n", uv_strerror(bind_status), uv_err_name(bind_status));
    return;
  } else {
    printf("Bound to addr...\n");
  }

  uv_stream_t* stream = reinterpret_cast<uv_stream_t*>(tcp_);
  int listen_status = uv_listen(stream, DEFAULT_BACKLOG, [](uv_stream_t* server, int status) {
    if (status != 0) {
      printf("Connection Error: %s - %s\n", uv_strerror(status), uv_err_name(status));
      return;
    }
    printf("Got connection from libuv.\n");

    Server* self = reinterpret_cast<Server*>(server->data);

    uv_tcp_t* client = new uv_tcp_t;
    uv_tcp_init(self->loop_, client);

    uv_accept(server, reinterpret_cast<uv_stream_t*>(client));

    Socket* socket = new Socket(client, 1024);

    self->connection_cb_data_->callback(socket, self->connection_cb_data_->data);
  });
  if (listen_status != 0) {
    printf("Listen Error: %s - %s\n", uv_strerror(listen_status), uv_err_name(listen_status));
    return;
  } else {
    printf("Did listen setup...\n");
  }
}

void Server::Close(void* data, server_shutdown_cb_t callback) {
  server_shutdown_cb_data_t* cb_data = new server_shutdown_cb_data_t;
  cb_data->data = data;
  cb_data->callback = callback;

  shutdown_.data = static_cast<void*>(cb_data);
  uv_stream_t* stream = reinterpret_cast<uv_stream_t*>(tcp_);
  uv_shutdown(&shutdown_, stream, [](uv_shutdown_t* req, int status) {
    server_shutdown_cb_data_t* data = reinterpret_cast<server_shutdown_cb_data_t*>(req->data);
    server_shutdown_cb_t callback = data->callback;
    if (callback != nullptr) {
        callback(status, data->data);
    }
  });
}
