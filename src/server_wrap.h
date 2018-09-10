#ifndef BOB_SERVER_WRAP_H_
#define BOB_SERVER_WRAP_H_

#include <napi.h>
#include "socket.h"
#include "server.h"
#include "passthrough/js-passthrough.h"

class Server_Wrap : public Napi::ObjectWrap<Server_Wrap> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Server_Wrap(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;

  Napi::Value Listen(const Napi::CallbackInfo& info);
  Napi::Value Close(const Napi::CallbackInfo& info);

  Napi::Reference<Napi::Value> self_;

  Server* server_;

  Napi::FunctionReference cb_ref_;
  Napi::Env env_;
};

#endif // BOB_SERVER_WRAP_H