load("@rules_cc//cc:defs.bzl", "cc_library")

package(
  default_visibility = ["//visibility:public"],
)

cc_library(
  name = "Daedalus",
  srcs = glob(["src/**/*.c"]),
  hdrs = glob(["src/**/*.h"]),
  includes = ["src"],
  deps = select ({
    "@platforms//os:osx": ["@glfw//:glfw"],
    "@platforms//os:linux": [],
  }),
  linkopts = select ({
    "@platforms//os:osx": ["-undefined", "dynamic_lookup", "-framework Cocoa", "-framework IOKit", "-framework CoreFoundation"],
    "@platforms//os:linux": ["-lvulkan", "-lxcb", "-lX11", "-lX11-xcb", "-lxkbcommon", "-L/usr/lib", "-L/usr/X11R6/lib"],
  }),
  copts = select ({
    "@platforms//os:osx": [""],
    "@platforms//os:linux": ["-g -shared -fdeclspec -fPIC"],
  }),
  defines = ["D_DEBUG", "D_EXPORT"],
  target_compatible_with = select({
        "@platforms//os:osx": [],
        "@platforms//os:linux": [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
)