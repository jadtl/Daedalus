load("@rules_cc//cc:defs.bzl", "cc_binary")

package(
  default_visibility = ["//visibility:public"],
)

cc_binary(
  name = "Testbed",
  srcs = ["src/**/*.c"],
  textual_hdrs = ["src/**/*.h"],
  deps = ["//engine:Engine"],
  defines = ["-D_DEBUG -D_IMPORT"],
)