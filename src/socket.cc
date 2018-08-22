#include <assert.h>
#include <uv.h>
#include <bob.h>
#include "socket.h"
#include "sink.h"
#include "source.h"
#include "passthrough/js-passthrough.h"

Socket::Socket(uv_loop_t *loop, uint32_t bufsize)
    : source_(nullptr),
      sink_(nullptr),
      tcp_(static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t))))
{
  buf_ = uv_buf_init(new char[bufsize], bufsize);

  uv_tcp_init(loop, tcp_);
  tcp_->data = this;

  source_ = new Socket_Source(tcp_);
  sink_ = new Socket_Sink(tcp_, 1024);
}

Socket::~Socket() {
  delete &addr_;
  delete &tcp_;
  delete[] buf_.base;
}

void Socket::Connect(const char* ip, int port) {
  sockaddr_in addr_in;
  int err = uv_ip4_addr(ip, port, &addr_in);
  if (err != 0) {
    printf("IPv4 ADDR: %s - %s\n", uv_strerror(err), uv_err_name(err));
    assert(false);
  }
  addr_ = reinterpret_cast<sockaddr *>(&addr_in);

  uv_tcp_connect(&req_, tcp_, addr_, [](uv_connect_t* req, int status) {
    printf("TCP CONNECT STATUS: %s - %s\n", uv_strerror(status), uv_err_name(status));
  });
}

void Socket::Start() {
  sink_->Start();
}

void Socket::Stop() {

}
