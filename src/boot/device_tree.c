#include "boot/early_printk.h"
#include "utils/types.h"

struct fdt_header {
  uint32_t magic;
  uint32_t total_size;
  uint32_t dt_struct_offset;
  uint32_t dt_strings_offset;
  uint32_t mem_resv_map_offset;
  uint32_t version;
  uint32_t last_compat_version;
  uint32_t boot_cpuid_phys;
  uint32_t dt_strings_size;
  uint32_t dt_struct_size;
};

const uint32_t fdt_magic = 0xd00dfeed;

const uint32_t fdt_expected_compat_version = 16;

bool validate_fdt_header(const struct fdt_header* fdt_header) {
  if (be_to_le(fdt_header->magic) != fdt_magic) {
    early_printk("Expected fdt_header.magic = %x, but was %x\n",
        fdt_magic, fdt_header->magic);
    return false;
  }

  if (be_to_le(fdt_header->last_compat_version) != fdt_expected_compat_version) {
    early_printk("Expected fdt_header.last_compat_version = %d, but was %d\n",
        fdt_expected_compat_version, fdt_header->last_compat_version);
    return false;
  }
  // TODO(brendan): We should probably also make sure the kernel did not
  // overwrite any of the reserved fields durring the initial loading.
  return true;
}

#define FDT_BEGIN_NODE (uint32_t) 0x00000001
#define FDT_END_NODE   (uint32_t) 0x00000002
#define FDT_PROP       (uint32_t) 0x00000003
#define FDT_NOP        (uint32_t) 0x00000004
#define FDT_END        (uint32_t) 0x00000009

struct fdt_prop {
  uint32_t len;
  uint32_t name_offset;
};

static uint8_t read_uint8(void** ptr) {
  uint8_t** uint8_ptr = (uint8_t**) ptr;
  uint8_t ret = **uint8_ptr;
  *uint8_ptr += 1;
  return ret;
}

static uint32_t read_uint32(void** ptr) {
  uint32_t** uint32_ptr = (uint32_t**) ptr;
  uint32_t ret = **uint32_ptr;
  *uint32_ptr += 1;
  return ret;
}

static uint8_t be_read_uint8(void** ptr) {
  return read_uint8(ptr);
}

static uint32_t be_read_uint32(void** ptr) {
  return be_to_le(read_uint32(ptr));
}

static struct fdt_prop* read_fdt_prop(void** ptr) {
  struct fdt_prop** fdt_prop_ptr = (struct fdt_prop**) ptr;
  struct fdt_prop* ret = *fdt_prop_ptr;
  *fdt_prop_ptr += 1;
  return ret;
}

static char* read_str(void** ptr) {
  char** str_ptr = (char**) ptr;
  char* ret = *str_ptr;
  *str_ptr += strlen(ret) + 1; // Include the null terminator.
  return ret;
}

static char* read_str_of_len(void** ptr, size_t len) {
  char** str_ptr = (char**) ptr;
  char* ret = *str_ptr;
  *str_ptr += len;
  return ret;
}

static void consume_padding(void** ptr) {
  const uint32_t padding_mask = 0x00000003;
  const uint32_t padding_inc  = 0x00000004;
  uint32_t raw_addr = (uint32_t) (long) *ptr;
  if (raw_addr & padding_mask) {
    *ptr = (void*) (long) ((raw_addr & ~padding_mask) + padding_inc);
  }
}

static void print_indent(int level) {
  for (int i = 0; i < level; i++) {
    early_puts("  ");
  }
}

static int parse_node(void** node_ptr, char* str_block, int level) {
  int ret = 0;
  char* node_name = read_str(node_ptr);
  consume_padding(node_ptr);
  print_indent(level - 1); early_printk("%s {\n", node_name);
  while (true) {
    uint32_t token = be_read_uint32(node_ptr);
    switch (token) {
      case FDT_BEGIN_NODE:
        ret = parse_node(node_ptr, str_block, level + 1);
        consume_padding(node_ptr);
        if (ret < 0) {
          return ret;
        }
        break;
      case FDT_END_NODE:
        print_indent(level - 1); early_printk("}\n");
        return 0;
      case FDT_PROP: { 
        struct fdt_prop* prop = read_fdt_prop(node_ptr);
        print_indent(level); early_printk("%s\n", str_block + be_to_le(prop->name_offset));
        read_str_of_len(node_ptr, be_to_le(prop->len));
        consume_padding(node_ptr);
        break;
        }
      case FDT_NOP:
        break;
      default:
        early_printk("Found illegal token %x at %p\n", token, ((uint32_t) (long) *node_ptr) - 4);
        return -1;
    }
  }
  return 0;
}

static int parse_struct_block(void* struct_block, char* str_block) {
  int ret = 0;
  early_printk("Parsing struct block at: %p\n", struct_block);
  uint32_t token = be_read_uint32(&struct_block);
  if (token == FDT_BEGIN_NODE) {
    ret = parse_node(&struct_block, str_block, 1);
    if (ret < 0) {
      return ret;
    }
    token = be_read_uint32(&struct_block);
    if (token != FDT_END) {
      return -1;
    }
    return 0;
  } else if (token == FDT_END) {
    return 0;
  }
  return -1;
}

int parse_device_tree(phys_addr_t device_tree_blob) {
  int ret = 0;
  struct fdt_header* fdt_header = (struct fdt_header*) device_tree_blob;
  if (!validate_fdt_header(fdt_header)) {
    return -1;
  }

  early_printk("Device tree is of version: %d\n", be_to_le(fdt_header->version));
  early_printk("Boot CPU ID is: %x\n", be_to_le(fdt_header->boot_cpuid_phys));

  void* struct_block = (void*) (long) (be_to_le(fdt_header->dt_struct_offset) + device_tree_blob);
  char* str_block = (char*) (long) (be_to_le(fdt_header->dt_strings_offset) + device_tree_blob);
  ret = parse_struct_block(struct_block, str_block);
  if (ret < 0) {
    return ret;
  }
  return 0;
}
