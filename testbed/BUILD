load("@rules_cc//cc:defs.bzl", "cc_binary")

package(
  default_visibility = ["//visibility:public"],
)

cc_library(
  name = "TestbedLibs",
  srcs = glob(["src/**/*.c"]),
  hdrs = glob(["src/**/*.h"]),
  deps = ["//engine:Engine"],
  copts = ["-Iexternal/engine"],
  includes = ["src"],
)

cc_binary(
  name = "Testbed",
  srcs = ["src/entry.c"],
  deps = ["//engine:Engine", ":TestbedLibs"],
  defines = ["D_DEBUG", "D_IMPORT"],
)