#include <napi.h>
#include "socket_wrap.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  return Socket_Wrap::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
