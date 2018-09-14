#ifndef BOB_SOCKET_SINK_H_
#define BOB_SOCKET_SINK_H_

#include <uv.h>
#include <bob.h>

class Socket_Sink : public BOB::Base {
 public:
  Socket_Sink(uv_tcp_t* tcp, uint32_t bufsize);
  virtual ~Socket_Sink();

  virtual BOB::Base* BindSource(BOB::Base* source);
  virtual void BindSink(BOB::Base* sink);
  virtual void Next(int status, void** error, char* data, size_t bytes);
  virtual void Pull(void** error, char* data, size_t size);
  void Start();

  virtual void OnUVWrite();
  virtual void ShutdownOnUVWrite();

 private:
  BOB::Base *source_;

  uv_tcp_t* tcp_;
  uv_write_t* w_req_;
  uv_shutdown_t* s_req_;

  uv_buf_t buf_;
};

#endif  // BOB_SOCKET_SINK_H_
