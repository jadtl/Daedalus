#include "platform.h"

#if DDLS_PLATFORM_APPLE

#include "core/logger.h"

#include "include/GLFW/glfw3.h"
#include <sys/time.h>

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct internal_state {
  GLFWwindow* window;
} internal_state;

b8 platform_startup(
  platform_state* plat_state, 
  const char* app_name,
  i32 x, 
  i32 y, 
  i32 width, 
  i32 height
) {
  plat_state->internal_state = malloc(sizeof(internal_state));
  internal_state* state = (internal_state*)plat_state->internal_state;

  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  
  state->window = glfwCreateWindow(width, height, app_name, NULL, NULL);
  glfwSetWindowPos(state->window, x, y);

  return TRUE;
}

void platform_shutdown(platform_state* plat_state) {
  internal_state* state = (internal_state*)plat_state->internal_state;

  glfwDestroyWindow(state->window);

  glfwTerminate();
}

b8 platform_pump_messages(platform_state* plat_state) {
  internal_state* state = (internal_state*)plat_state->internal_state;

  while (!glfwWindowShouldClose(state->window)) {
    glfwPollEvents();
  }

  return TRUE;
}

void* platform_allocate(u64 size, b8 aligned) {
  return malloc(size);
}
void platform_free(void* block, b8 aligned) {
  free(block);
}
void* platform_zero_memory(void* block, u64 size) {
  return memset(block, 0, size);
}
void* platform_copy_memory(void* dest, const void* source, u64 size) {
  return memcpy(dest, source, size);
}
void* platform_set_memory(void* dest, i32 value, u64 size) {
  return memset(dest, value, size);
}

void platform_console_write(const char* message, u8 color) {
  const char* color_strings[] = { "0;41", "1;31", "1;33", "1;32", "1;34", "1;30" };
  printf("\033[%sm%s\033[0m", color_strings[color], message);
}
void platform_console_write_error(const char* message, u8 color) {
  const char* color_strings[] = { "0;41", "1;31", "1;33", "1;32", "1;34", "1;30" };
  printf("\033[%sm%s\033[0m", color_strings[color], message);
}

f64 platform_get_absolute_time() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec + now.tv_nsec * 0.000000001;
}

void platform_sleep(u64 ms) {
#if _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000 * 1000;
  nanosleep(&ts, 0);
#else
  if (ms >= 1000) {
    sleep(ms / 1000);
  }
  usleep((ms % 1000) * 1000);
#endif
}

#endif