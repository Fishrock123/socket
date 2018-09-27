#include <assert.h>
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

Socket_Wrap::Socket_Wrap(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<Socket_Wrap>(info),
      socket_(nullptr),
      env_(info.Env()) {
  Napi::Env env = env_;
  Napi::HandleScope scope(env);

  self_ = Napi::Persistent(info.This());
}

Socket_Wrap* Socket_Wrap::New(Socket* socket) {
  Napi::Object js_this = constructor.Value().New(0, {});

  Socket_Wrap* self = Napi::ObjectWrap<Socket_Wrap>::Unwrap(js_this);

  self->InitSocket(socket);

  return self;
}

Napi::Value Socket_Wrap::Connect(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length();

  if (length != 2 && length != 3) {
    Napi::TypeError::New(env, "Socket.Connect() expects two or three arguments").ThrowAsJavaScriptException();
  } else if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Socket.Connect() expected argument [0] to be String").ThrowAsJavaScriptException();
  } if (!info[1].IsNumber()) {
    Napi::TypeError::New(env, "Socket.Connect() expected argument [1] to be Number").ThrowAsJavaScriptException();
  }

  void* cb_data = nullptr;
  if (length == 3) {
    if (!info[2].IsFunction()) {
      Napi::TypeError::New(env, "Socket.Connect() expected argument [2] to be Function").ThrowAsJavaScriptException();
    } else {
      cb_ref_ = Napi::Persistent(info[2].As<Napi::Function>());

      cb_data = static_cast<void*>(this);
    }
  }

  sockaddr_in addr_in;

  const uint32_t port = info[1].ToNumber().Uint32Value();
  const char* ip = info[0].ToString().Utf8Value().c_str();

  int err = uv_ip4_addr(ip, port, &addr_in);
  if (err != 0) {
    printf("IPv4 ADDR: %s - %s\n", uv_strerror(err), uv_err_name(err));
    assert(false);
  }
  sockaddr* addr = reinterpret_cast<sockaddr *>(&addr_in);

  InitSocket();

  socket_->Connect(addr, cb_data, [](int status, void* cb_data) {
    if (cb_data != nullptr) {
      Socket_Wrap* self = static_cast<Socket_Wrap*>(cb_data);
      Napi::HandleScope scope(self->env_);

      size_t argc = 1;
      napi_value argv = Napi::Number::New(self->env_, status);
      self->cb_ref_.Value().MakeCallback(Napi::Object::New(self->env_), argc, &argv);
      self->cb_ref_.Unref();
    }
  });

  return Napi::Value();
}

void Socket_Wrap::InitSocket() {
  uv_loop_t* loop;
  napi_status status;
  status = napi_get_uv_event_loop(env_, &loop);
  NAPI_CHECK(status, env_);

  InitSocket(new Socket(loop, 1024));
}

void Socket_Wrap::InitSocket(Socket* socket) {
  assert(socket_ == nullptr);

  socket_ = socket;

  Napi::Object self = self_.Value().ToObject();

  Napi::Function PassTCtr = Napi::Value::From(env_, PassThrough::CreateClass(env_)).As<Napi::Function>();

  Napi::Value source = Napi::Value::From(env_, PassTCtr.New(0, {}));
  source_pt_ = Napi::ObjectWrap<PassThrough>::Unwrap(source.ToObject());
  self.DefineProperty(Napi::PropertyDescriptor::Value("source", source_pt_->GetJSObject(env_)));

  Napi::Value sink = Napi::Value::From(env_, PassTCtr.New(0, {}));
  sink_pt_ = Napi::ObjectWrap<PassThrough>::Unwrap(sink.ToObject());
  self.DefineProperty(Napi::PropertyDescriptor::Value("sink", sink_pt_->GetJSObject(env_)));

  socket_->sink_->BindSource(sink_pt_);
  source_pt_->BindSource(socket_->source_);
}

Napi::Value Socket_Wrap::Start(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  socket_->Start();

  return Napi::Value();
}

Napi::Value Socket_Wrap::Stop(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  socket_->Stop();

  return Napi::Value();
}

Napi::Value Socket_Wrap::Value() {
  return self_.Value();
}
