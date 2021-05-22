load("@rules_cc//cc:defs.bzl", "objc_library")

objc_library(
    name = "glfw",
    hdrs = glob(["**/GLFW/*.h"]),
    includes = ["include"],
    srcs = [
        "src/cocoa_joystick.m",
        "src/nsgl_context.m",
        "src/cocoa_monitor.m",
        "src/cocoa_window.m",
        "src/cocoa_init.m",
        "src/posix_thread.c",
        "src/cocoa_time.c",
        "src/egl_context.c",
        "src/osmesa_context.c",
        "src/context.c",
        "src/init.c",
        "src/input.c",
        "src/vulkan.c",
        "src/window.c",
        "src/monitor.c",
    ],
    textual_hdrs = glob(["src/*.h"]),
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