/*  multiboot.h - Multiboot header file.  */
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

#pragma once

#include "service/elf.h"

struct Multiboot {

  enum {
    // How many bytes from the start of the file we search for the header.  
    SEARCH			= 8192,
    HEADER_ALIGN			= 4,

    // The magic field should contain this.  
    HEADER_MAGIC			= 0x1BADB002,

    // This should be in %eax.  
    BOOTLOADER_MAGIC		= 0x2BADB002,

    // Alignment of multiboot modules.  
    MOD_ALIGN			= 0x00001000,

    // Alignment of the multiboot info structure.  
    INFO_ALIGN			= 0x00000004,

    // Flags set in the 'flags' member of the multiboot header.  

    // Align all boot modules on i386 page (4KB) boundaries.  
    PAGE_ALIGN			= 0x00000001,

    // Must pass memory information to OS.  
    MEMORY_INFO			= 0x00000002,

    // Must pass video information to OS.  
    VIDEO_MODE			= 0x00000004,

    // This flag indicates the use of the address fields in the header.  
    USE_LOAD_FIELDS		= 0x00010000,

    // Flags to be set in the 'flags' member of the multiboot info structure.  

    // is there basic lower/upper memory information? 
    INFO_MEMORY			= 0x00000001,
    // is there a boot device set? 
    INFO_BOOTDEV			= 0x00000002,
    // is the command-line defined? 
    INFO_CMDLINE			= 0x00000004,
    // are there modules to do something with? 
    INFO_MODS			= 0x00000008,

    // These next two are mutually exclusive 

    // is there a symbol table loaded? 
    INFO_AOUT_SYMS		= 0x00000010,
    // is there an ELF section header table? 
    INFO_ELF_SHDR			= 0x00000020,

    // is there a full memory map? 
    INFO_MEM_MAP			= 0x00000040,

    // Is there drive info?  
    INFO_DRIVE_INFO		= 0x00000080,

    // Is there a config table?  
    INFO_CONFIG_TABLE		= 0x00000100,

    // Is there a boot loader name?  
    INFO_BOOT_LOADER_NAME		= 0x00000200,

    // Is there a APM table?  
    INFO_APM_TABLE		= 0x00000400,

    // Is there video information?  
    INFO_VBE_INFO		        = 0x00000800,
    INFO_FRAMEBUFFER_INFO	        = 0x00001000,
  };

typedef unsigned char		multiboot_uint8_t;
typedef unsigned short		multiboot_uint16_t;
typedef unsigned int		multiboot_uint32_t;
typedef unsigned long long	multiboot_uint64_t;

struct Header
{
  /* Must be MULTIBOOT_MAGIC - see above.  */
  multiboot_uint32_t magic;

  /* Feature flags.  */
  multiboot_uint32_t flags;

  /* The above fields plus this one must equal 0 mod 2^32. */
  multiboot_uint32_t checksum;

  /* These are only valid if MULTIBOOT_AOUT_KLUDGE is set.  */
  multiboot_uint32_t header_addr;
  multiboot_uint32_t load_addr;
  multiboot_uint32_t load_end_addr;
  multiboot_uint32_t bss_end_addr;
  multiboot_uint32_t entry_addr;

  /* These are only valid if MULTIBOOT_VIDEO_MODE is set.  */
  multiboot_uint32_t mode_type;
  multiboot_uint32_t width;
  multiboot_uint32_t height;
  multiboot_uint32_t depth;
};

/* The symbol table for a.out.  */
struct multiboot_aout_symbol_table
{
  multiboot_uint32_t tabsize;
  multiboot_uint32_t strsize;
  multiboot_uint32_t addr;
  multiboot_uint32_t reserved;
};
typedef struct multiboot_aout_symbol_table multiboot_aout_symbol_table_t;

/* The section header table for ELF.  */
struct multiboot_elf_section_header_table
{
  multiboot_uint32_t num;
  multiboot_uint32_t size;
  multiboot_uint32_t addr;
  multiboot_uint32_t shndx;
};
typedef struct multiboot_elf_section_header_table multiboot_elf_section_header_table_t;

struct Info
{
  /* Multiboot info version number */
  multiboot_uint32_t flags;

  /* Available memory from BIOS */
  multiboot_uint32_t mem_lower;
  multiboot_uint32_t mem_upper;

  /* "root" partition */
  multiboot_uint32_t boot_device;

  /* Kernel command line */
  multiboot_uint32_t cmdline;

  /* Boot-Module list */
  multiboot_uint32_t mods_count;
  multiboot_uint32_t mods_addr;

  union
  {
    multiboot_aout_symbol_table_t aout_sym;
    multiboot_elf_section_header_table_t elf_sec;
  } u;

  /* Memory Mapping buffer */
  multiboot_uint32_t mmap_length;
  multiboot_uint32_t mmap_addr;

  /* Drive Info buffer */
  multiboot_uint32_t drives_length;
  multiboot_uint32_t drives_addr;

  /* ROM configuration table */
  multiboot_uint32_t config_table;

  /* Boot Loader Name */
  multiboot_uint32_t boot_loader_name;

  /* APM table */
  multiboot_uint32_t apm_table;

  /* Video */
  multiboot_uint32_t vbe_control_info;
  multiboot_uint32_t vbe_mode_info;
  multiboot_uint16_t vbe_mode;
  multiboot_uint16_t vbe_interface_seg;
  multiboot_uint16_t vbe_interface_off;
  multiboot_uint16_t vbe_interface_len;

  multiboot_uint64_t framebuffer_addr;
  multiboot_uint32_t framebuffer_pitch;
  multiboot_uint32_t framebuffer_width;
  multiboot_uint32_t framebuffer_height;
  multiboot_uint8_t framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT	2
  multiboot_uint8_t framebuffer_type;
  union
  {
    struct
    {
      multiboot_uint32_t framebuffer_palette_addr;
      multiboot_uint16_t framebuffer_palette_num_colors;
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

struct multiboot_color
{
  multiboot_uint8_t red;
  multiboot_uint8_t green;
  multiboot_uint8_t blue;
};

struct Mmap_entry
{
  multiboot_uint32_t size;
  multiboot_uint64_t addr;
  multiboot_uint64_t len;
#define MULTIBOOT_MEMORY_AVAILABLE		1
#define MULTIBOOT_MEMORY_RESERVED		2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
  multiboot_uint32_t type;
} __attribute__((packed));
typedef struct multiboot_mmap_entry multiboot_memory_map_t;

struct multiboot_mod_list
{
  /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
  multiboot_uint32_t mod_start;
  multiboot_uint32_t mod_end;

  /* Module command line */
  multiboot_uint32_t cmdline;

  /* padding to take it to 16 bytes (must be zero) */
  multiboot_uint32_t pad;
};
typedef struct multiboot_mod_list multiboot_module_t;

/* APM BIOS info.  */
struct multiboot_apm_info
{
  uint16 version;
  uint16 cseg;
  uint32 offset;
  uint16 cseg_16;
  uint16 dseg;
  uint16 flags;
  uint16 cseg_len;
  uint16 cseg_16_len;
  uint16 dseg_len;
};

  static unsigned load_multiboot(char *module, size_t modsize, char *phys_mem, uintptr_t &rip,
                                 size_t &maxptr, size_t mem_size, size_t mem_offset,
                                 unsigned long long magic,
                                 multiboot_uint32_t flags_out)
  {
    int mbh_off = -1;
    for (int i = 0; i < SEARCH; i += HEADER_ALIGN) {
      if (*((uint32*)(module+i)) == HEADER_MAGIC) {
        mbh_off = i;
        break;
      }
    }
    check1(1, mbh_off < 0, "multiboot magic found\n");

    Multiboot::Header &mbh = *reinterpret_cast<Multiboot::Header *>(module+mbh_off);

    check1(2, (mbh.magic + mbh.flags + mbh.checksum) & 0xffffffff,
           "multiboot checksum is invalid\n");
    flags_out = mbh.flags;

    Logging::printf("multiboot:   header_addr = %x\n", mbh.header_addr);
    Logging::printf("multiboot:     load_addr = %x\n", mbh.load_addr);
    Logging::printf("multiboot: load_end_addr = %x\n", mbh.load_end_addr);
    Logging::printf("multiboot:  bss_end_addr = %x\n", mbh.bss_end_addr);


    if (mbh.flags & PAGE_ALIGN)
      Logging::printf("%s: Align all boot modules on i386 page (4KB) boundaries\n", __func__);
    if (mbh.flags & MEMORY_INFO)
      Logging::printf("%s: Must pass memory information to OS\n", __func__);
    if (mbh.flags & VIDEO_MODE)
      Logging::printf("%s: Must pass video information to OS\n", __func__);
    if (mbh.flags & USE_LOAD_FIELDS)
      Logging::printf("%s: Address fields are used in this header\n", __func__);

    if (!(mbh.flags & USE_LOAD_FIELDS)) {
      Logging::printf("%s: load image using ELF header\n", __func__);
      return Elf::decode_elf(module, modsize, phys_mem, rip,
                             maxptr, mem_size, mem_offset, magic);
    } else {
      Logging::printf("%s: load image using Multiboot fields\n", __func__);
      check1(3, mbh.load_addr > mbh.header_addr);
      
      size_t const load_off = mbh_off - (mbh.header_addr - mbh.load_addr);

      size_t const load_size = (mbh.load_end_addr == 0)
        ? modsize - load_off : mbh.load_end_addr - mbh.load_addr;

      size_t const code_size = (mbh.bss_end_addr == 0)
      	? load_size : mbh.bss_end_addr - mbh.load_addr;

      check1(4, load_size > mem_size);
      check1(5, code_size > mem_size);

      char *load_dst = phys_mem + mbh.load_addr - mem_offset;

	  bool overlap = (load_dst <= module && module < load_dst + load_size);
	  check1(6, overlap, " a.out overlap");

      Logging::printf("loading multiboot kernel (%zx bytes) at %x\n",
                      load_size, mbh.load_addr);


      memcpy(load_dst, module+load_off, load_size);
      if (mbh.bss_end_addr != 0) {
        memset(load_dst+load_size, 0x00,
               mbh.bss_end_addr - mbh.load_addr - load_size);
      }

      rip = mbh.entry_addr;
      maxptr = mbh.load_addr + code_size - mem_offset;

      if (maxptr < mbh.load_addr + code_size - mem_offset) {
        maxptr = mbh.load_addr + code_size - mem_offset;
      }

      // final mapping check
      check1(7, (*((uint32*)(phys_mem+mbh.header_addr-mem_offset))) != HEADER_MAGIC);
      return 0;
    }
  }
};
