#pragma once

#include "defines.h"

typedef enum memory_tag {
  MEMORY_TAG_UNKNOWN,
  MEMORY_TAG_ARRAY,
  MEMORY_TAG_DARRAY,
  MEMORY_TAG_DICT,
  MEMORY_TAG_RING_QUEUE,
  MEMORY_TAG_BST,
  MEMORY_TAG_STRING,
  MEMORY_TAG_APPLICATION,
  MEMORY_TAG_JOB,
  MEMORY_TAG_TEXTURE,
  MEMORY_TAG_MATERIAL_INSTANCE,
  MEMORY_TAG_RENDERER,
  MEMORY_TAG_GAME,
  MEMORY_TAG_TRANSFORM,
  MEMORY_TAG_ENTITY,
  MEMORY_TAG_ENTITY_NODE,
  MEMORY_TAG_SCENE,
  
  MEMORY_TAG_MAX_TAGS
} memory_tag;

DDLS_API void initialize_memory();
DDLS_API void shutdown_memory();

DDLS_API void* ddls_allocate(u64 size, memory_tag tag);

DDLS_API void ddls_free(void* block, u64 size, memory_tag tag);

DDLS_API void* ddls_zero_memory(void* block, u64 size);

DDLS_API void* ddls_copy_memory(void* dest, const void* source, u64 size);

DDLS_API void* ddls_set_memory(void* dest, i32 value, u64 size);

DDLS_API char* get_memory_usage_str();