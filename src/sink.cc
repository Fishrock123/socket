#include <uv.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "sink.h"
#include "socket.h"
#include "utils-inl.h"

Socket_Sink::Socket_Sink(uv_tcp_t* tcp, uint32_t bufsize)
    : source_(nullptr),
      tcp_(tcp),
      w_req_(nullptr),
      s_req_(nullptr) {
  buf_ = uv_buf_init(new char[bufsize], bufsize);
}

Socket_Sink::~Socket_Sink() {}

//
//  C++ only binding
//

BOB::Base* Socket_Sink::BindSource(BOB::Base* source) {
  source->BindSink(static_cast<BOB::Base*>(this));
  source_ = source;

  return this;
}

void Socket_Sink::BindSink(BOB::Base* sink) {
  printf("this is the sink!");
  abort();
}

//
// C++ API
//

void Socket_Sink::Next(int bob_status, void** error, char* data, size_t bytes) {
  if (tcp_ == nullptr) {
    printf("Socket_Sink::Next tcp null");
    abort();
  }

  int err;

  uv_buf_t buf = uv_buf_init(const_cast<char*>(data), bytes);

  delete w_req_;
  w_req_ = new uv_write_t;

  w_req_->data = static_cast<void*>(this);

  uv_os_fd_t fd;
  uv_fileno(reinterpret_cast<uv_handle_t*>(tcp_), &fd);
  printf("Write fd: %i\n", fd);

  uv_stream_t* stream = reinterpret_cast<uv_stream_t*>(tcp_);
  if (bob_status != BOB::CONTINUE) {
    err = uv_write(w_req_,
                  stream,
                  &buf,
                  1u,
                  [](uv_write_t* req, int status) {
      static_cast<Socket_Sink*>(req->data)->ShutdownOnUVWrite();
    });
  } else {
    err = uv_write(w_req_,
                  stream,
                  &buf,
                  1u,
                  [](uv_write_t* req, int status) {
      static_cast<Socket_Sink*>(req->data)->OnUVWrite();
    });
  }
  if (err != 0) {
    printf("Socket write Error: %s - %s\n", uv_strerror(err), uv_err_name(err));
    return;
  }
}

void Socket_Sink::OnUVWrite() {
    void* error_data = nullptr;
    source_->Pull(&error_data, buf_.base, buf_.len);
}

void Socket_Sink::ShutdownOnUVWrite() {
  int err;

  s_req_ = new uv_shutdown_t;

  uv_stream_t* stream = reinterpret_cast<uv_stream_t*>(tcp_);
  err = uv_shutdown(s_req_, stream, [](uv_shutdown_t* req, int status) {
    if (status != 0) {
      printf("Shutdown: %s\n", uv_err_name(status));
    } else {
      printf("Clean shutdown\n");
    }
  });
  if (err != 0) {
    printf("Socket shutdown Error: %s - %s\n", uv_strerror(err), uv_err_name(err));
    return;
  }
}

void Socket_Sink::Pull(void** error, char* data, size_t size) {
  printf("Sinks do not have a public ::Pull!");
  abort();
}

void Socket_Sink::Start() {
  void* error_data = nullptr;
  source_->Pull(&error_data, buf_.base, buf_.len);
}
