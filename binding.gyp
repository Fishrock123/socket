{
  "targets": [
    {
      "target_name": "addon",
      "include_dirs": [
        "<!(node -e \"require('bob-base')\")",
        "<!@(node -p \"require('node-addon-api').include\")",
        # Use the passthough as a dependency
        "./src/passthrough",
        "./src"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "sources": [
        "src/addon.cc",
        "src/sink.cc",
        "src/socket_wrap.cc",
        "src/socket.cc",
        "src/source.cc",
        "src/passthrough/js-passthrough.cc"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.7",
      },
      "msvs_settings": {
        "VCCLCompilerTool": { "ExceptionHandling": 1 },
      },
    }
  ]
}
