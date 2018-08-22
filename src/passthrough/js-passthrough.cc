#include <node_api.h>
#include <assert.h>
#include <stdio.h>
#include "js-passthrough.h"
#include "../utils-inl.h"

napi_ref PassThrough::constructor;

PassThrough::PassThrough()
    : js_source_(nullptr),
      js_sink_(nullptr),
      env_(nullptr),
      wrapper_(nullptr),
      buf_ref_(nullptr),
      source_(nullptr),
      sink_(nullptr) {}

PassThrough::~PassThrough() {
  napi_delete_reference(env_, wrapper_);
  if (js_source_ != nullptr) napi_delete_reference(env_, js_source_);
  if (js_sink_ != nullptr) napi_delete_reference(env_, js_sink_);
}

// Called by N-API.
void PassThrough::Destructor(napi_env env, void* nativeObject, void* /*finalize_hint*/) {
  printf("### cleaned up\n");
  delete static_cast<PassThrough*>(nativeObject);
}

//
//  C++ only binding
//

BOB::Base* PassThrough::BindSource(BOB::Base* source) {
  if (js_source_ != nullptr) {
    printf("Should not bind from C++ when there is js_source_\n");
    return this;
  }

  source->BindSink(static_cast<BOB::Base*>(this));
  source_ = source;

  return this;
}

void PassThrough::BindSink(BOB::Base* sink) {
  if (js_sink_ != nullptr) {
    printf("Should not bind from C++ when there is js_sink_\n");
    return;
  }

  sink_ = sink;
}

//
// C++ API
//

void PassThrough::Next(int bob_status, void** error, char* data, size_t bytes) {
  // If we have bound to the C++ API
  if (sink_ != nullptr) {
    sink_->Next(bob_status, error, data, bytes);

  // If we have bound to the JS API
  } else if (js_sink_ != nullptr) {
    napi_status status;

    napi_handle_scope scope;
    status = napi_open_handle_scope(env_, &scope);
    NAPI_CHECK(status, env_);

    napi_value sink;
    status = napi_get_reference_value(env_, js_sink_, &sink);
    NAPI_CHECK(status, env_);

    napi_value sink_next;
    status = napi_get_named_property(env_, sink, "next", &sink_next);
    NAPI_CHECK(status, env_);

    napi_value buffer;
    status = napi_create_buffer_copy(env_, bytes, data, nullptr, &buffer);
    NAPI_CHECK(status, env_);

    // If we had data from a JS buffer previously (probably) we need to free it.
    if (buf_ref_ != nullptr) {
      status = napi_delete_reference(env_, buf_ref_);
      NAPI_CHECK(status, env_);
      buf_ref_ = nullptr;
    }

    napi_value js_status;
    status = napi_create_int32(env_, bob_status, &js_status);
    NAPI_CHECK(status, env_);

    napi_value js_bytes;
    status = napi_create_int32(env_, bytes, &js_bytes);
    NAPI_CHECK(status, env_);

    napi_value js_error;
    if (*error != nullptr) {
      js_error = reinterpret_cast<napi_value>(*error);
    } else {
      napi_get_undefined(env_, &js_error);
    }

    size_t argc = 4;
    const napi_value argv[] = {
      js_status,
      js_error,
      buffer,
      js_bytes
    };

    status = napi_make_callback(env_,
                                nullptr,
                                sink,
                                sink_next,
                                argc,
                                argv,
                                nullptr);
    NAPI_CHECK(status, env_);

    status = napi_close_handle_scope(env_, scope);
    NAPI_CHECK(status, env_);

  } else {
    printf("PassThrough::Next called with invalid {js_}sink_ pointers\n");
  }
}

void PassThrough::Pull(void** error, char* data, size_t size) {
  // If we have bound to the C++ API
  if (source_ != nullptr) {
    source_->Pull(error, data, size);

  // If we have bound to the JS API
  } else if (js_source_ != nullptr) {
    napi_status status;

    napi_handle_scope scope;
    status = napi_open_handle_scope(env_, &scope);
    NAPI_CHECK(status, env_);

    napi_value source;
    status = napi_get_reference_value(env_, js_source_, &source);
    NAPI_CHECK(status, env_);

    napi_value source_pull;
    status = napi_get_named_property(env_, source, "pull", &source_pull);
    NAPI_CHECK(status, env_);

    napi_value buffer;
    status = napi_create_buffer_copy(env_, size, data, nullptr, &buffer);
    NAPI_CHECK(status, env_);

    // If we had data from a JS buffer previously (probably) we need to free it.
    if (buf_ref_ != nullptr) {
      status = napi_delete_reference(env_, buf_ref_);
      NAPI_CHECK(status, env_);
      buf_ref_ = nullptr;
    }

    napi_value undef;
    status = napi_get_undefined(env_, &undef);
    NAPI_CHECK(status, env_);

    size_t argc = 2;
    const napi_value argv[] = {
      reinterpret_cast<napi_value>(undef),
      buffer
    };

    status = napi_make_callback(env_,
                                nullptr,
                                source,
                                source_pull,
                                argc,
                                argv,
                                nullptr);
    NAPI_CHECK(status, env_);

    status = napi_close_handle_scope(env_, scope);
    NAPI_CHECK(status, env_);

  } else {
    printf("PassThrough::Pull called with invalid {js_}sink_ pointers\n");
  }
}

//
// JavaScript API
//

#define DECLARE_NAPI_METHOD(name, func)                          \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

napi_value PassThrough::CreateClass(napi_env env) {
  napi_status status;
  napi_property_descriptor properties[] = {
      // { "value", 0, 0, GetValue, SetValue, 0, napi_default, 0 },
      DECLARE_NAPI_METHOD("bindSource", BindSource),
      DECLARE_NAPI_METHOD("bindSink", BindSink),
      DECLARE_NAPI_METHOD("next", Next),
      DECLARE_NAPI_METHOD("pull", Pull),
  };

  napi_value cons;
  status =
      napi_define_class(env, "PassThrough", NAPI_AUTO_LENGTH, New, nullptr, 4, properties, &cons);
  if (status != napi_ok) return nullptr;

  status = napi_create_reference(env, cons, 1, &constructor);
  if (status != napi_ok) return nullptr;

  NAPI_CHECK(status, env);
  return cons;
}

napi_value PassThrough::New(napi_env env, napi_callback_info info) {
  napi_status status;

  napi_value target;
  status = napi_get_new_target(env, info, &target);
  NAPI_CHECK(status, env);
  bool is_constructor = target != nullptr;

  if (is_constructor) {
    // Invoked as constructor: `new PassThrough(...)`
    napi_value jsthis;
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    NAPI_CHECK(status, env);

    PassThrough* obj = new PassThrough();

    obj->env_ = env;
    status = napi_wrap(env,
                       jsthis,
                       reinterpret_cast<void*>(obj),
                       PassThrough::Destructor,
                       nullptr,  // finalize_hint
                       &obj->wrapper_);
    NAPI_CHECK(status, env);

    return jsthis;
  } else {
    // Invoked as plain function `PassThrough(...)`, turn into construct call.
    size_t argc_ = 1;
    napi_value args[1];
    status = napi_get_cb_info(env, info, &argc_, args, nullptr, nullptr);
    NAPI_CHECK(status, env);

    const size_t argc = 1;
    napi_value argv[argc] = {args[0]};

    napi_value cons;
    status = napi_get_reference_value(env, constructor, &cons);
    NAPI_CHECK(status, env);

    napi_value instance;
    status = napi_new_instance(env, cons, argc, argv, &instance);
    NAPI_CHECK(status, env);

    return instance;
  }
}

napi_value PassThrough::BindSource(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 1;
  napi_value argv[argc];
  napi_value jsthis;
  status = napi_get_cb_info(env, info, &argc, argv, &jsthis, nullptr);
  NAPI_CHECK(status, env);

  napi_value source = argv[0];

  napi_value js_bindsink;
  status = napi_get_named_property(env, source, "bindSink", &js_bindsink);
  NAPI_CHECK(status, env);

  napi_value argv_[1] = { jsthis };
  status = napi_call_function(env,
                              source,
                              js_bindsink,
                              1,
                              argv_,
                              nullptr);
  NAPI_CHECK(status, env);

  PassThrough* obj;
  status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
  NAPI_CHECK(status, env);

  // Bind to the C++ API if it is available.
  BOB::Base* source_obj;
  status = napi_unwrap(env, source, reinterpret_cast<void**>(&source_obj));
  if (source_obj != nullptr && status == napi_ok) {
    printf("bound c++ source!\n");
    obj->source_ = source_obj;
  }

  // Set a JS property to hold the reference from being GC'd but in JS so it is
  // still visible to the GC, then keep the C++ reference weakly.
  status = napi_set_named_property(env, jsthis, "source", source);
  NAPI_CHECK(status, env);

  napi_ref js_source_ref;
  status = napi_create_reference(env, source, 0, &js_source_ref);
  NAPI_CHECK(status, env);

  obj->js_source_ = js_source_ref;

  return jsthis;
}

napi_value PassThrough::BindSink(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 1;
  napi_value argv[argc];
  napi_value jsthis;
  status = napi_get_cb_info(env, info, &argc, argv, &jsthis, nullptr);
  NAPI_CHECK(status, env);

  PassThrough* obj;
  status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
  NAPI_CHECK(status, env);

  // Bind to the C++ API if it is available.
  BOB::Base* sink_obj;
  status = napi_unwrap(env, argv[0], reinterpret_cast<void**>(&sink_obj));
  if (sink_obj != nullptr && status == napi_ok) {
    printf("bound c++ sink!\n");
    obj->sink_ = sink_obj;
  }

  napi_value sink = argv[0];

  // Set a JS property to hold the reference from being GC'd but in JS so it is
  // still visible to the GC, then keep the C++ reference weakly.
  status = napi_set_named_property(env, jsthis, "sink", sink);
  NAPI_CHECK(status, env);

  napi_ref sink_ref;
  status = napi_create_reference(env, sink, 0, &sink_ref);
  NAPI_CHECK(status, env);

  obj->js_sink_ = sink_ref;

  return nullptr;
}

napi_value PassThrough::Next(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 4;
  napi_value argv[argc];
  napi_value jsthis;
  status = napi_get_cb_info(env, info, &argc, argv, &jsthis, nullptr);
  NAPI_CHECK(status, env);

  PassThrough* obj;
  status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
  NAPI_CHECK(status, env);

  // If we have bound to the C++ API
  if (obj->sink_ != nullptr) {
    int bob_status;
    status = napi_get_value_int32(env, argv[0], &bob_status);
    NAPI_CHECK(status, env);

    status = napi_create_reference(env, argv[2], 1, &obj->buf_ref_);
    NAPI_CHECK(status, env);

    char* data = nullptr;
    size_t buffsize;
    status = napi_get_buffer_info(env,
                                  argv[2],
                                  reinterpret_cast<void**>(&data),
                                  &buffsize);
    NAPI_CHECK(status, env);

    int bytes;
    status = napi_get_value_int32(env, argv[3], &bytes);
    NAPI_CHECK(status, env);

    printf("C++ -> C++ next!\n");

    obj->sink_->Next(bob_status, reinterpret_cast<void**>(&argv[1]), data, bytes);

  // If we have bound to the JS API
  } else if (obj->js_sink_ != nullptr) {
    napi_value sink;

    status = napi_get_reference_value(env, obj->js_sink_, &sink);
    NAPI_CHECK(status, env);

    napi_value sink_next;
    status = napi_get_named_property(env, sink, "next", &sink_next);
    NAPI_CHECK(status, env);

    status = napi_make_callback(env,
                                nullptr,
                                sink,
                                sink_next,
                                4,
                                argv,
                                nullptr);
    NAPI_CHECK(status, env);

  } else {
    printf("PassThrough::Next (JS) called with invalid {js_}sink_ pointers\n");
  }
  return nullptr;
}

napi_value PassThrough::Pull(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 2;
  napi_value argv[argc];
  napi_value jsthis;
  status = napi_get_cb_info(env, info, &argc, argv, &jsthis, nullptr);
  NAPI_CHECK(status, env);

  PassThrough* obj;
  status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
  NAPI_CHECK(status, env);

  // If we have bound to the C++ API
  if (obj->source_ != nullptr) {
    status = napi_create_reference(env, argv[1], 1, &obj->buf_ref_);
    NAPI_CHECK(status, env);

    char* data = nullptr;
    size_t size;
    status = napi_get_buffer_info(env,
                                  argv[1],
                                  reinterpret_cast<void**>(&data),
                                  &size);
    NAPI_CHECK(status, env);

    printf("C++ -> C++ pull!\n");

    obj->source_->Pull(reinterpret_cast<void**>(&argv[0]), data, size);

  // If we have bound to the JS API
  } else if (obj->js_source_ != nullptr) {
    napi_value source;

    status = napi_get_reference_value(env, obj->js_source_, &source);
    NAPI_CHECK(status, env);

    napi_value js_pull;
    status = napi_get_named_property(env, source, "pull", &js_pull);
    NAPI_CHECK(status, env);

    status = napi_make_callback(env,
                                nullptr,
                                source,
                                js_pull,
                                2,
                                argv,
                                nullptr);
    NAPI_CHECK(status, env);

  } else {
    printf("PassThrough::Pull (JS) called with invalid {js_}sink_ pointers\n");
  }
  return nullptr;
}

napi_value PassThrough::GetJSObject(napi_env env) {
  napi_status status;
  napi_value jsObject;
  status = napi_get_reference_value(env, wrapper_, &jsObject);
  NAPI_CHECK(status, env);
  return jsObject;
}
