#include <uv.h>
#include <assert.h>
#include <stdio.h>
#include "sink.h"
#include "socket.h"
#include "utils-inl.h"

Socket_Sink::Socket_Sink(uv_tcp_t* tcp, uint32_t bufsize)
    : source_(nullptr),
      tcp_(tcp),
      req_() {
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

  req_.data = static_cast<void*>(this);

  uv_stream_t* stream = reinterpret_cast<uv_stream_t*>(tcp_);
  err = uv_write(&req_, 
                 stream, 
                 &buf, 
                 1u, 
                 [](uv_write_t* req, int status) {
    static_cast<Socket_Sink*>(req->data)->OnUVWrite(); 
  });
}

void Socket_Sink::OnUVWrite() {
    void* error_data = nullptr;
    source_->Pull(&error_data, buf_.base, buf_.len);
}

void Socket_Sink::Pull(void** error, char* data, size_t size) {
  printf("Sinks do not have a public ::Pull!");
  abort();
}

void Socket_Sink::Start() {
  void* error_data = nullptr;
  source_->Pull(&error_data, buf_.base, buf_.len);
}
