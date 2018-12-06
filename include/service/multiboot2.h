/*  multiboot2.h - Multiboot 2 header file.  */
/*  Copyright (C) 1999,2003,2007,2008,2009,2010  Free Software Foundation, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 *  DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <base/log.h>

#pragma once

struct Multiboot2
{
  enum {
    // How many bytes from the start of the file we search for the header.
    MULTIBOOT_SEARCH			= 32768,
    MULTIBOOT_HEADER_ALIGN			= 8,

    // The magic field should contain this.
    MULTIBOOT2_HEADER_MAGIC			= 0xe85250d6,

    // This should be in %eax. 
    MULTIBOOT2_BOOTLOADER_MAGIC		= 0x36d76289,

    // Alignment of multiboot modules.
    MULTIBOOT_MOD_ALIGN			= 0x00001000,

    // Alignment of the multiboot info structure. 
    MULTIBOOT_INFO_ALIGN			= 0x00000008,

    // Flags set in the 'flags' member of the multiboot header. 

    MULTIBOOT_TAG_ALIGN                  = 8,
    MULTIBOOT_TAG_TYPE_END               = 0,
    MULTIBOOT_TAG_TYPE_CMDLINE           = 1,
    MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  = 2,
    MULTIBOOT_TAG_TYPE_MODULE            = 3,
    MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     = 4,
    MULTIBOOT_TAG_TYPE_BOOTDEV           = 5,
    MULTIBOOT_TAG_TYPE_MMAP              = 6,
    MULTIBOOT_TAG_TYPE_VBE               = 7,
    MULTIBOOT_TAG_TYPE_FRAMEBUFFER       = 8,
    MULTIBOOT_TAG_TYPE_ELF_SECTIONS      = 9,
    MULTIBOOT_TAG_TYPE_APM               = 10,
    MULTIBOOT_TAG_TYPE_EFI32             = 11,
    MULTIBOOT_TAG_TYPE_EFI64             = 12,
    MULTIBOOT_TAG_TYPE_SMBIOS            = 13,
    MULTIBOOT_TAG_TYPE_ACPI_OLD          = 14,
    MULTIBOOT_TAG_TYPE_ACPI_NEW          = 15,
    MULTIBOOT_TAG_TYPE_NETWORK           = 16,
    MULTIBOOT_TAG_TYPE_EFI_MMAP          = 17,
    MULTIBOOT_TAG_TYPE_EFI_BS            = 18,
    MULTIBOOT_TAG_TYPE_EFI32_IH          = 19,
    MULTIBOOT_TAG_TYPE_EFI64_IH          = 20,
    MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR    = 21,

    MULTIBOOT_HEADER_TAG_END  = 0,
    MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST  = 1,
    MULTIBOOT_HEADER_TAG_ADDRESS  = 2,
    MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS  = 3,
    MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS  = 4,
    MULTIBOOT_HEADER_TAG_FRAMEBUFFER  = 5,
    MULTIBOOT_HEADER_TAG_MODULE_ALIGN  = 6,
    MULTIBOOT_HEADER_TAG_EFI_BS        = 7,
    MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32  = 8,
    MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64  = 9,
    MULTIBOOT_HEADER_TAG_RELOCATABLE  = 10,

    MULTIBOOT_ARCHITECTURE_I386  = 0,
    MULTIBOOT_ARCHITECTURE_MIPS32  = 4,
    MULTIBOOT_HEADER_TAG_OPTIONAL = 1,

    MULTIBOOT_LOAD_PREFERENCE_NONE = 0,
    MULTIBOOT_LOAD_PREFERENCE_LOW = 1,
    MULTIBOOT_LOAD_PREFERENCE_HIGH = 2,

    MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED = 1,
    MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED = 2,
 };

typedef unsigned char		multiboot_uint8_t;
typedef unsigned short		multiboot_uint16_t;
typedef unsigned int		multiboot_uint32_t;
typedef unsigned long long	multiboot_uint64_t;

struct multiboot_header
{
  /* Must be MULTIBOOT_MAGIC - see above.  */
  multiboot_uint32_t magic;

  /* ISA */
  multiboot_uint32_t architecture;

  /* Total header length.  */
  multiboot_uint32_t header_length;

  /* The above fields plus this one must equal 0 mod 2^32. */
  multiboot_uint32_t checksum;
};

struct multiboot_header_tag
{
  multiboot_uint16_t type;
  multiboot_uint16_t flags;
  multiboot_uint32_t size;
};

struct multiboot_header_tag_information_request
{
  multiboot_uint16_t type;
  multiboot_uint16_t flags;
  multiboot_uint32_t size;
  multiboot_uint32_t requests[0];
};

struct multiboot_header_tag_address
{
  multiboot_uint16_t type;
  multiboot_uint16_t flags;
  multiboot_uint32_t size;
  multiboot_uint32_t header_addr;
  multiboot_uint32_t load_addr;
  multiboot_uint32_t load_end_addr;
  multiboot_uint32_t bss_end_addr;
};

struct multiboot_header_tag_entry_address
{
  multiboot_uint16_t type;
  multiboot_uint16_t flags;
  multiboot_uint32_t size;
  multiboot_uint32_t entry_addr;
};

struct multiboot_header_tag_console_flags
{
  multiboot_uint16_t type;
  multiboot_uint16_t flags;
  multiboot_uint32_t size;
  multiboot_uint32_t console_flags;
};

struct multiboot_header_tag_framebuffer
{
  multiboot_uint16_t type;
  multiboot_uint16_t flags;
  multiboot_uint32_t size;
  multiboot_uint32_t width;
  multiboot_uint32_t height;
  multiboot_uint32_t depth;
};

struct multiboot_header_tag_module_align
{
  multiboot_uint16_t type;
  multiboot_uint16_t flags;
  multiboot_uint32_t size;
};

struct multiboot_header_tag_relocatable
{
  multiboot_uint16_t type;
  multiboot_uint16_t flags;
  multiboot_uint32_t size;
  multiboot_uint32_t min_addr;
  multiboot_uint32_t max_addr;
  multiboot_uint32_t align;
  multiboot_uint32_t preference;
};

struct multiboot_color
{
  multiboot_uint8_t red;
  multiboot_uint8_t green;
  multiboot_uint8_t blue;
};

struct multiboot_mmap_entry
{
  multiboot_uint64_t addr;
  multiboot_uint64_t len;
#define MULTIBOOT_MEMORY_AVAILABLE		1
#define MULTIBOOT_MEMORY_RESERVED		2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
  multiboot_uint32_t type;
  multiboot_uint32_t zero;
};
typedef struct multiboot_mmap_entry multiboot_memory_map_t;

struct multiboot_tag
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
};

struct multiboot_tag_string
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  char string[0];
};

struct multiboot_tag_module
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint32_t mod_start;
  multiboot_uint32_t mod_end;
  char cmdline[0];
};

struct multiboot_tag_basic_meminfo
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint32_t mem_lower;
  multiboot_uint32_t mem_upper;
};

struct multiboot_tag_bootdev
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint32_t biosdev;
  multiboot_uint32_t slice;
  multiboot_uint32_t part;
};

struct multiboot_tag_mmap
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint32_t entry_size;
  multiboot_uint32_t entry_version;
  struct multiboot_mmap_entry entries[0];  
};

struct multiboot_vbe_info_block
{
  multiboot_uint8_t external_specification[512];
};

struct multiboot_vbe_mode_info_block
{
  multiboot_uint8_t external_specification[256];
};

struct multiboot_tag_vbe
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;

  multiboot_uint16_t vbe_mode;
  multiboot_uint16_t vbe_interface_seg;
  multiboot_uint16_t vbe_interface_off;
  multiboot_uint16_t vbe_interface_len;

  struct multiboot_vbe_info_block vbe_control_info;
  struct multiboot_vbe_mode_info_block vbe_mode_info;
};

struct multiboot_tag_framebuffer_common
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;

  multiboot_uint64_t framebuffer_addr;
  multiboot_uint32_t framebuffer_pitch;
  multiboot_uint32_t framebuffer_width;
  multiboot_uint32_t framebuffer_height;
  multiboot_uint8_t framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT	2
  multiboot_uint8_t framebuffer_type;
  multiboot_uint16_t reserved;
};

struct multiboot_tag_framebuffer
{
  struct multiboot_tag_framebuffer_common common;

  union
  {
    struct
    {
      multiboot_uint16_t framebuffer_palette_num_colors;
      struct multiboot_color framebuffer_palette[0];
    };
    struct
    {
      multiboot_uint8_t framebuffer_red_field_position;
      multiboot_uint8_t framebuffer_red_mask_size;
      multiboot_uint8_t framebuffer_green_field_position;
      multiboot_uint8_t framebuffer_green_mask_size;
      multiboot_uint8_t framebuffer_blue_field_position;
      multiboot_uint8_t framebuffer_blue_mask_size;
    };
  };
};

struct multiboot_tag_elf_sections
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint32_t num;
  multiboot_uint32_t entsize;
  multiboot_uint32_t shndx;
  char sections[0];
};

struct multiboot_tag_apm
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint16_t version;
  multiboot_uint16_t cseg;
  multiboot_uint32_t offset;
  multiboot_uint16_t cseg_16;
  multiboot_uint16_t dseg;
  multiboot_uint16_t flags;
  multiboot_uint16_t cseg_len;
  multiboot_uint16_t cseg_16_len;
  multiboot_uint16_t dseg_len;
};

struct multiboot_tag_efi32
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint32_t pointer;
};

struct multiboot_tag_efi64
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint64_t pointer;
};

struct multiboot_tag_smbios
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint8_t major;
  multiboot_uint8_t minor;
  multiboot_uint8_t reserved[6];
  multiboot_uint8_t tables[0];
};

struct multiboot_tag_old_acpi
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint8_t rsdp[0];
};

struct multiboot_tag_new_acpi
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint8_t rsdp[0];
};

struct multiboot_tag_network
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint8_t dhcpack[0];
};

struct multiboot_tag_efi_mmap
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint32_t descr_size;
  multiboot_uint32_t descr_vers;
  multiboot_uint8_t efi_mmap[0];
}; 

struct multiboot_tag_efi32_ih
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint32_t pointer;
};

struct multiboot_tag_efi64_ih
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint64_t pointer;
};

struct multiboot_tag_load_base_addr
{
  multiboot_uint32_t type;
  multiboot_uint32_t size;
  multiboot_uint32_t load_base_addr;
};

  static unsigned load_multiboot2(char *module, size_t modsize, char *phys_mem,
                                  struct multiboot_header &mbh, uintptr_t &rip,
                                  size_t &maxptr, size_t mem_size, size_t mem_offset,
                                  unsigned long long magic)
  {
    int mbh_off = -1;
    for (size_t i = 0; i < MULTIBOOT_SEARCH; i += MULTIBOOT_HEADER_ALIGN) {
      if (*((uint32*)(module+i)) == MULTIBOOT2_HEADER_MAGIC) {
        mbh_off = i;
        break;
      }
    }

    if (mbh_off < 0) {
      Genode::error("multiboot magic not found");
      return ~0U;
    }

    mbh = *reinterpret_cast<struct multiboot_header *>(module+mbh_off);

    if ((mbh.magic + mbh.flags + mbh.checksum) & 0xffffffff) {
      Genode::error("multiboot checksum is invalid");
      return ~0U;
    }
    warning("multiboot magic found at ", (Hex)mbh_off);

    if (mbh.flags & MULTIBOOT_USE_LOAD_FIELDS) {
      warning("loading multiboot");
      if (mbh.load_addr > mbh.header_addr)
        return ~0U;

      size_t const load_off = mbh_off - (mbh.header_addr - mbh.load_addr);
      warning("multiboot module load offset is ", (Hex)load_off);

      size_t const load_size = mbh.load_end_addr
        ? mbh.load_end_addr - mbh.load_addr
        : modsize - load_off;
      warning("multiboot load size is ", (Hex)load_size);

      size_t const code_size = mbh.bss_end_addr
      	? mbh.bss_end_addr - mbh.load_addr
      	: load_size;
      warning("multiboot code size is ", (Hex)code_size);

      if (code_size > mem_size)
        return ~0U;

      char *load_dst = phys_mem + mbh.load_addr - mem_offset;
      warning("load at ", (Hex)mbh.load_addr, " (", (Hex)(mbh.load_addr - mem_offset),")");

	  bool overlap = (load_dst <= module && module < load_dst + load_size);
	  check1(10, overlap, " a.out overlap");

      memcpy(load_dst, module+load_off, load_size);
      if (mbh.bss_end_addr) {
        memset(load_dst+load_size, 0x00,
               mbh.bss_end_addr - mbh.load_addr - load_size);
      }

      rip = mbh.entry_addr;
      maxptr = mbh.load_addr + code_size - mem_offset;

/*
      warning("multiboot entry is ", (Hex)rip);
      warning((Hex)maxptr, " < ", Hex(mbh.load_addr + code_size - mem_offset));
      if (maxptr < mbh.load_addr + code_size - mem_offset) {
        warning("maxptr increased from ", (Hex)maxptr, " to ", (Hex)(code_size - mem_offset));
        maxptr = mbh.load_addr + code_size - mem_offset;
      }
 */

      // final mapping check
      if (*((uint32*)(phys_mem+mbh.header_addr-mem_offset)) != MULTIBOOT_HEADER_MAGIC)
        return ~0U;

      warning("multiboot loading complete");
      return 0;
    }
    return ~0U;
  }
};
