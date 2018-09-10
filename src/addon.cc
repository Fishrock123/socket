#include <napi.h>
#include "socket_wrap.h"
#include "server_wrap.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  Socket_Wrap::Init(env, exports);
  Server_Wrap::Init(env, exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
