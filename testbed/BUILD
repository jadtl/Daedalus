load("@rules_cc//cc:defs.bzl", "cc_binary")

package(
  default_visibility = ["//visibility:public"],
)

cc_library(
  name = "Game",
  srcs = ["src/game.c"],
  hdrs = ["src/game.h"],
  deps = ["//engine:Daedalus"],
  copts = ["-Iexternal/engine"],
  includes = ["src"],
)

cc_binary(
  name = "Testbed",
  srcs = ["src/entry.c"],
  deps = ["//engine:Daedalus", ":Game"],
  linkopts = select ({
    "@platforms//os:osx": [""],
    "@platforms//os:linux": ["-Wl,-rpath,."],
  }),
  defines = ["D_DEBUG", "D_IMPORT"],
)