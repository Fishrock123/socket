#ifndef BOB_SERVER_H_
#define BOB_SERVER_H_

#include <uv.h>
#include <bob.h>
#include "socket.h"
#include "sink.h"
#include "source.h"

typedef void (*server_connection_cb_t)(Socket* socket, void* data);
typedef void (*server_shutdown_cb_t)(int status, void* data);

typedef struct server_connection_cb_data_t {
  void* data;
  server_connection_cb_t callback;
} server_connection_cb_data_t;

typedef struct server_shutdown_cb_data_t {
  void* data;
  server_shutdown_cb_t callback;
} server_shutdown_cb_data_t;

class Server {
 public:
  Server(uv_loop_t* loop);
  virtual ~Server();

  virtual void Listen(sockaddr* addr, void* data, server_connection_cb_t callback);
  virtual void Close(void* data, server_shutdown_cb_t callback);

  server_connection_cb_data_t* connection_cb_data_;

 private:
  sockaddr* addr_;

  uv_loop_t* loop_;
  uv_tcp_t* tcp_;
  uv_shutdown_t shutdown_;
};

#endif  // BOB_SERVER_H_
