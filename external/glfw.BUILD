load("@rules_cc//cc:defs.bzl", "objc_library")

objc_library(
    name = "glfw",
    hdrs = glob(["**/GLFW/*.h"]),
    srcs = glob(["src/*.m", "src/*.c", "!src/win32*.c"]),
    textual_hdrs = glob(["src/*.h"])
    defines = [
        "_GLFW_COCOA",
    ],
    copts = [
        "-fno-objc-arc",
    ],
    visibility = [
        "//visibility:public",
    ],
)