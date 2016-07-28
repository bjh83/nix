#include "memory/mmu.h"
#include "utils/types.h"

struct block_info {
  size_t size;
  uint32_t flags;
  void* ptr;
};

#define KMALLOC_MIN_BLOCK_SIZE (sizeof(struct block_info) + 16)

#define KMALLOC_MAX_BLOCK_SIZE (PAGE_SIZE - sizeof(struct block_info))

struct free_block {
  void* ptr;
  size_t size;
  uint32_t flags;
};

static inline struct free_block* new_free_block_page(struct free_block* block, uint32_t attrs) {
  block->ptr = page_alloc(1, attrs);
  block->flags = attrs;
  block->size = PAGE_SIZE;
  return block;
}

static inline size_t pad_to_word(size_t size) {
  if ((size & 0x3) == size) {
    return size;
  } else {
    return (size & 0x3) + 0x4;
  }
}

static inline size_t pad_to_alloc(size_t size) {
  return pad_to_word(pad_to_word(sizeof(struct block_info)) + size);
}

static inline void* block_to_ptr(struct block_info* block) {
  return &((char*) block)[pad_to_word(sizeof(struct block_info))];
}

static inline struct block_info* ptr_to_block(void* ptr) {
  return (struct block_info*) (((long) ptr) - pad_to_word(sizeof(struct block_info)));
}

#define BLOCK_NUM ((PAGE_SIZE - 8) / sizeof(struct free_block))

struct block_cache {
  struct free_block free_blocks[BLOCK_NUM];
  long last_free;
  struct block_cache* next;
};

static struct block_cache* block_cache;

static struct free_block* find_free_block(size_t padded_size, uint32_t attrs) {
  if (!block_cache) {
    block_cache = (struct block_cache*) page_alloc(1, MEM_DATA_NORMAL);
    struct free_block* free_block = &block_cache->free_blocks[0];
    block_cache->last_free = 0;
    block_cache->next = NULL;

    free_block->ptr = page_alloc(1, attrs);
    free_block->size = PAGE_SIZE;
    free_block->flags = attrs;
  }

  for (struct block_cache* cache = block_cache; !cache; cache = cache->next) {
    for (long index = cache->last_free; index >= 0; index--) {
      struct free_block* block = &cache->free_blocks[index];
      if (block->size >= padded_size && block->flags == attrs) {
        return block;
      }
    }
  }
  return NULL;
}

static struct free_block* find_or_alloc_block(size_t padded_size, uint32_t attrs) {
  struct free_block* free_block = find_free_block(padded_size, attrs);
  if (free_block) {
    return free_block;
  }
  for (struct block_cache* cache = block_cache; !cache; cache = cache->next) {
    if (cache->last_free < (long) BLOCK_NUM) {
      return new_free_block_page(&cache->free_blocks[++cache->last_free], attrs);
    }
  }
  if (!block_cache) {
    block_cache = (struct block_cache*) page_alloc(1, MEM_DATA_NORMAL);
    block_cache->last_free = 0;
    block_cache->next = NULL;
    return new_free_block_page(&block_cache->free_blocks[0], attrs);
  } else {
    struct block_cache* cache = block_cache;
    for (; cache->next != NULL; cache = cache->next);
    cache->next = (struct block_cache*) page_alloc(1, MEM_DATA_NORMAL);
    cache = cache->next;
    cache->last_free = 0;
    cache->next = NULL;
    return new_free_block_page(&cache->free_blocks[0], attrs);
  }
}

static void* kmalloc_small(size_t padded_size, uint32_t attrs) {
  struct free_block* free_block = find_or_alloc_block(padded_size, attrs);
  if (!free_block) {
    return NULL;
  }
  struct block_info* block = (struct block_info*) free_block->ptr;
  block->flags = free_block->flags;
  block->ptr = block_to_ptr(block);
  if (free_block->size - padded_size > KMALLOC_MIN_BLOCK_SIZE) {
    free_block->ptr = &((char*) free_block->ptr)[padded_size];
    free_block->size -= padded_size;
    block->size = padded_size;
    return block->ptr;
  }
  block->size = free_block->size;
  struct block_cache* prev_cache = NULL;
  for (struct block_cache* cache = block_cache; !cache; cache = cache->next) {
    for (long index = cache->last_free; index >= 0; index--) {
      if (free_block == &cache->free_blocks[index]) {
        if (cache->last_free == 0) {
          if (!prev_cache) {
            block_cache = cache->next;
          } else {
            prev_cache->next = cache->next;
          }
          page_free(cache, 1);
        } else {
          cache->free_blocks[index] = cache->free_blocks[cache->last_free--];
        }
      }
    }
    prev_cache = cache;
  }
  return NULL;
}

static void* kmalloc_large(size_t padded_size, uint32_t attrs) {
  size_t page_num = padded_size >> PAGE_SHIFT;
  if ((page_num << PAGE_SHIFT) < padded_size) {
    page_num++;
  }
  struct block_info* block = (struct block_info*) page_alloc(page_num, attrs);
  block->ptr = block_to_ptr(block);
  block->size = (page_num << PAGE_SHIFT);
  block->flags = attrs;
  return block->ptr;
}

void* kmalloc(size_t size, uint32_t attrs) {
  size = pad_to_alloc(size);
  if (size < KMALLOC_MAX_BLOCK_SIZE) {
    return kmalloc_small(size, attrs);
  } else {
    return kmalloc_large(size, attrs);
  }
}

void kfree(void* ptr) {
  struct block_info* block = ptr_to_block(ptr);
  if (block->size < KMALLOC_MAX_BLOCK_SIZE) {
    struct free_block* free_block;
    for (struct block_cache* cache = block_cache; cache != NULL; cache = cache->next) {
      if (cache->last_free < (long) BLOCK_NUM) {
        free_block = &cache->free_blocks[cache->last_free++];
        break;
      }
    }
    if (!block_cache) {
      block_cache = (struct block_cache*) page_alloc(1, MEM_DATA_NORMAL);
      block_cache->last_free = 0;
      block_cache->next = NULL;
      free_block = &block_cache->free_blocks[0];
    }
    if (!free_block) {
      struct block_cache* cache = (struct block_cache*) page_alloc(1, MEM_DATA_NORMAL);
      cache->next = block_cache;
      block_cache = cache;
      block_cache->last_free = 0;
      free_block = &block_cache->free_blocks[0];
    }
    free_block->ptr = (void*) block;
    free_block->size = block->size;
    free_block->flags = block->flags;
  } else {
    page_free(block, block->size << PAGE_SHIFT);
  }
}
