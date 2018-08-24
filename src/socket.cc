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

void Socket::Connect(sockaddr* addr, void* data, socket_connect_cb_t callback) {
  addr_ = addr;

  connect_cb_data_t* cb_data = new connect_cb_data_t;
  cb_data->data = data;
  cb_data->callback = callback;

  req_.data = static_cast<void*>(cb_data);
  uv_tcp_connect(&req_, tcp_, addr_, [](uv_connect_t* req, int status) {
    connect_cb_data_t* data = reinterpret_cast<connect_cb_data_t*>(req->data);
    socket_connect_cb_t callback = data->callback;
    if (callback != nullptr) {
      callback(status, data->data);
    }
    delete data;
  });
}

void Socket::Start() {
  sink_->Start();
}

void Socket::Stop() {

}
