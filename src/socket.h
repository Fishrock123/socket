#ifndef BOB_SOCKET_H_
#define BOB_SOCKET_H_

#include <uv.h>
#include <bob.h>
#include "socket.h"
#include "sink.h"
#include "source.h"

class Socket {
 public:
  Socket(uv_loop_t* loop, uint32_t bufsize);
  virtual ~Socket();

  virtual void Connect(const char* ip, int port);
  virtual void Start();
  virtual void Stop();

  Socket_Source* source_;
  Socket_Sink* sink_;

private:
  sockaddr* addr_;

  uv_tcp_t* tcp_;
  uv_connect_t req_;
  uv_buf_t buf_;
};

#endif  // BOB_SOCKET_H_
