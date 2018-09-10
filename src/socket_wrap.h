#ifndef BOB_SOCKET_WRAP_H_
#define BOB_SOCKET_WRAP_H_

#include <napi.h>
#include "socket.h"
#include "passthrough/js-passthrough.h"

class Socket_Wrap : public Napi::ObjectWrap<Socket_Wrap> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Socket_Wrap(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;

  Napi::Value Connect(const Napi::CallbackInfo& info);
  Napi::Value Start(const Napi::CallbackInfo& info);
  Napi::Value Stop(const Napi::CallbackInfo& info);

  Napi::Reference<Napi::Value> self_;

  Socket* socket_;

  PassThrough* source_pt_;
  PassThrough* sink_pt_;

  Napi::FunctionReference cb_ref_;
  Napi::Env env_;
};

#endif // BOB_SOCKET_WRAP_H_
