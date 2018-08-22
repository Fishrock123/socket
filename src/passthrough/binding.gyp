{
  "targets": [
    {
      "target_name": "addon",
      'include_dirs': [
        "<!(node -e \"require('bob-base')\")",
        '.'
      ],
      "sources": [
        "addon.cc",
        "js-passthrough.cc"
      ]
    }
  ]
}
