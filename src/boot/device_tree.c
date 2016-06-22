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

int parse_device_tree(phys_addr_t device_tree_blob) {
  struct fdt_header* fdt_header = (struct fdt_header*) (long) device_tree_blob;
  if (!validate_fdt_header(fdt_header)) {
    return -1;
  }

  early_printk("Device tree is of version: %d\n", be_to_le(fdt_header->version));
  early_printk("Boot CPU ID is: %x\n", be_to_le(fdt_header->boot_cpuid_phys));
  return 0;
}
