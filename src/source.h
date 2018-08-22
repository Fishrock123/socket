#ifndef Socket_Source_H_
#define Socket_Source_H_

#include <uv.h>
#include <bob.h>

class Socket_Source : public BOB::Base {
 public:
  Socket_Source(uv_tcp_t* tcp);
  virtual ~Socket_Source();

  virtual BOB::Base* BindSource(BOB::Base* source);
  virtual void BindSink(BOB::Base* sink);
  virtual void Next(int status, void** error, char* data, size_t bytes);
  virtual void Pull(void** error, char* data, size_t size);

  virtual void OnUVRead(ssize_t nread, const uv_buf_t *buf);

 private:
  BOB::Base *sink_;

  uv_tcp_t* tcp_;
};

#endif  // Socket_Source_H_
