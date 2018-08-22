#include <napi.h>
#include <node_api.h>
#include "socket_wrap.h"
#include "socket.h"
#include "utils-inl.h"

Napi::FunctionReference Socket_Wrap::constructor;

Napi::Object Socket_Wrap::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Socket", {
    InstanceMethod("connect", &Socket_Wrap::Connect),
    InstanceMethod("start", &Socket_Wrap::Start),
    InstanceMethod("stop", &Socket_Wrap::Stop)
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Socket", func);
  return exports;
}

Socket_Wrap::Socket_Wrap(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Socket_Wrap>(info)  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  uv_loop_t* loop;
  napi_status status;
  status = napi_get_uv_event_loop(env, &loop);
  NAPI_CHECK(status, env);

  this->socket_ = new Socket(loop, 1024);

  Napi::Object self = info.This().ToObject();

  Napi::Function PassTCtr = Napi::Value::From(env, PassThrough::CreateClass(env)).As<Napi::Function>();

  Napi::Value source = Napi::Value::From(env, PassTCtr.New(0, {}));
  source_pt_ = Napi::ObjectWrap<PassThrough>::Unwrap(source.ToObject());
  self.DefineProperty(Napi::PropertyDescriptor::Value("source", source_pt_->GetJSObject(env)));

  Napi::Value sink = Napi::Value::From(env, PassTCtr.New(0, {}));
  sink_pt_ = Napi::ObjectWrap<PassThrough>::Unwrap(sink.ToObject());
  self.DefineProperty(Napi::PropertyDescriptor::Value("sink", sink_pt_->GetJSObject(env)));

  this->socket_->sink_->BindSource(sink_pt_);
  source_pt_->BindSource(this->socket_->source_);
}

Napi::Value Socket_Wrap::Connect(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length();

  if (length <= 0) {
    Napi::TypeError::New(env, "Socket.Connect() expected two arguments").ThrowAsJavaScriptException();
  } else if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Socket.Connect() expected argument 0 to be String").ThrowAsJavaScriptException();
  } if (!info[1].IsNumber()) {
    Napi::TypeError::New(env, "Socket.Connect() expected argument 1 to be Number").ThrowAsJavaScriptException();
  }

  this->socket_->Connect(info[0].ToString().Utf8Value().c_str(), info[1].ToNumber().Uint32Value());

  return Napi::Value();
}

Napi::Value Socket_Wrap::Start(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  this->socket_->Start();

  return Napi::Value();
}

Napi::Value Socket_Wrap::Stop(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  this->socket_->Stop();

  return Napi::Value();
}
