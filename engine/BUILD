load("@rules_cc//cc:defs.bzl", "cc_library")

package(
  default_visibility = ["//visibility:public"],
)

cc_library(
  name = "Engine",
  srcs = ["src/**/*.c"],
  textual_hdrs = ["src/**/*.h"],
  deps = select ({
    "@bazel_tools//src/conditions:darwin": ["@glfw//:glfw"],
  }),
  linkopts = select ({
    "@bazel_tools//src/conditions:darwin": ["-undefined", "dynamic_lookup", "-framework Cocoa", "-framework IOKit", "-framework CoreFoundation"],
  }),
  defines = ["-D_DEBUG", "-D_EXPORT"],
)