// -*- Mode: C++ -*-
/** @file
 * Endianness conversion
 *
 * Copyright (C) 2010, Julian Stecklina <jsteckli@os.inf.tu-dresden.de>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of Vancouver.
 *
 * Vancouver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Vancouver is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#pragma once

#include <nul/types.h>

namespace Endian {

#if defined(__i386__) || defined(__x86_64__)
  static inline uint16 hton16(uint16 value) { asm ("xchg %b0, %h0" : "+Q"(value)); return value; }
  static inline uint16 ntoh16(uint16 value) { asm ("xchg %b0, %h0" : "+Q"(value)); return value; }
  static inline uint32 hton32(uint32 value) { return __builtin_bswap32(value); }
  static inline uint32 ntoh32(uint32 value) { return __builtin_bswap32(value); }
  static inline uint64 hton64(uint64 value) { return __builtin_bswap64(value); }
#else
  #error Port me!
#endif

}

// EOF
