#include <uv.h>
#include <assert.h>
#include <stdio.h>
#include "source.h"
#include "socket.h"
#include "utils-inl.h"

Socket_Source::Socket_Source(uv_tcp_t* tcp)
    : sink_(nullptr),
      tcp_(tcp) {}

Socket_Source::~Socket_Source() {}

//
//  C++ only binding
//

BOB::Base* Socket_Source::BindSource(BOB::Base* source) {
  printf("this is the source!");
  abort();
}

void Socket_Source::BindSink(BOB::Base* sink) {
  sink_ = sink;
}

//
// C++ API
//

void Socket_Source::Next(int bob_status, void** error, char* data, size_t bytes) {
  printf("Sources do not have a public ::Next!\n");
  abort();
}

void Socket_Source::Pull(void** error, char* data, size_t size) {
  if (tcp_ == nullptr) {
    printf("Socket_Source::Pull tcp null\n");
    abort();
  }

  int err;

  uv_stream_t* stream = reinterpret_cast<uv_stream_t *>(tcp_);
  err = uv_read_start(stream, [](uv_handle_t *handle, 
                                 size_t suggested_size, 
                                 uv_buf_t *buf) { 
    buf->base = reinterpret_cast<char*>(malloc(suggested_size));
    buf->len = suggested_size;
  }, [](uv_stream_t* stream, 
        ssize_t nread, 
        const uv_buf_t *buf) {
    uv_read_stop(stream); // Pull only one "packet" at a time
    static_cast<Socket*>(stream->data)->source_->OnUVRead(nread, buf); 
  });
}

void Socket_Source::OnUVRead(ssize_t nread, const uv_buf_t *buf) {
  int ret_status = nread != UV_EOF ? BOB::Status::CONTINUE : BOB::Status::END;

  if (ret_status == BOB::Status::END) {
    uv_handle_t* handle = reinterpret_cast<uv_handle_t *>(tcp_);
    uv_close(handle, [](uv_handle_t* handle) {});
  }

  void* error_data = nullptr;
  sink_->Next(ret_status, &error_data, buf->base, nread < 0 ? 0 : nread);
}
