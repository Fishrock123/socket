#include <assert.h>
#include <napi.h>
#include <node_api.h>
#include "server_wrap.h"
#include "socket_wrap.h"
#include "utils-inl.h"

Napi::FunctionReference Server_Wrap::constructor;

Napi::Object Server_Wrap::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Server", {
    InstanceMethod("listen", &Server_Wrap::Listen),
    InstanceMethod("close", &Server_Wrap::Close)
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Server", func);
  return exports;
}

Server_Wrap::Server_Wrap(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<Server_Wrap>(info),
      env_(info.Env()) {
  Napi::Env env = env_;
  Napi::HandleScope scope(env);

  self_ = Napi::Persistent(info.This());

  uv_loop_t* loop;
  napi_status status;
  status = napi_get_uv_event_loop(env, &loop);
  NAPI_CHECK(status, env);

  server_ = new Server(loop);
}

Napi::Value Server_Wrap::Listen(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length();

  if (length != 3) {
    Napi::TypeError::New(env, "Server.Listen() expects three arguments").ThrowAsJavaScriptException();
  } else if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Server.Listen() expected argument [0] to be String").ThrowAsJavaScriptException();
  } if (!info[1].IsNumber()) {
    Napi::TypeError::New(env, "Server.Listen() expected argument [1] to be Number").ThrowAsJavaScriptException();
  }

  void* cb_data = nullptr;
  if (!info[2].IsFunction()) {
    Napi::TypeError::New(env, "Server.Listen() expected argument [2] to be Function").ThrowAsJavaScriptException();
  } else {
    cb_ref_ = Napi::Persistent(info[2].As<Napi::Function>());

    cb_data = static_cast<void*>(this);
  }

  sockaddr_in* addr_in = reinterpret_cast<sockaddr_in *>(&addr_);

  const uint32_t port = info[1].ToNumber().Uint32Value();
  const char* ip = info[0].ToString().Utf8Value().c_str();

  int err = uv_ip4_addr(ip, port, addr_in);
  if (err != 0) {
    printf("IPv4 ADDR: %s - %s\n", uv_strerror(err), uv_err_name(err));
    assert(false);
  }

  server_->Listen(&addr_, cb_data, [](Socket* socket, void* cb_data) {
    if (cb_data != nullptr) {
      Server_Wrap* self = static_cast<Server_Wrap*>(cb_data);
      Napi::HandleScope scope(self->env_);

      Socket_Wrap* socket_wrap = Socket_Wrap::New(socket);

      size_t argc = 1;
      napi_value argv = socket_wrap->Value();

      self->cb_ref_.Value().MakeCallback(Napi::Object::New(self->env_), argc, &argv);
    }
  });

  return Napi::Value();
}

Napi::Value Server_Wrap::Close(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  // TODO: use a JS callback
  server_->Close(this, [](int status, void* data) {
    if (status > 0) {
      printf("Shutdown Error: %s - %s\n", uv_strerror(status), uv_err_name(status));
    }
  });

  cb_ref_.Unref();

  return Napi::Value();
}
