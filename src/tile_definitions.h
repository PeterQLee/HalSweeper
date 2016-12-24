#ifndef TILE_DEFINITIONS_H
#define TILE_DEFINITIONS_H

//Taken from http://gareus.org/wiki/embedding_resources_in_executables
#ifdef __APPLE__
#include <mach-o/getsect.h>

#define EXTLD(NAME) \
  extern const unsigned char _section$__DATA__ ## NAME [];
#define IMAGE(NAME) _section$__DATA__ ## NAME
#define LDLEN(NAME) (getsectbyname("__DATA", "__" #NAME)->size)

#elif (defined __WIN32__)  /* mingw */

#define EXTLD(NAME) \
  extern const unsigned char binary_ ## NAME ## _start[]; \
  extern const unsigned char binary_ ## NAME ## _end[];

#define IMAGE(NAME) \
  binary_ ## NAME ## _start
#define LDLEN(NAME) \
  ((binary_ ## NAME ## _end) - (binary_ ## NAME ## _start))

#else /* gnu/linux ld */

#define EXTLD(NAME) \
  extern const unsigned char _binary_data_ ## NAME ## _data_start[]; \
  extern const unsigned char _binary_data_ ## NAME ## _data_end[];
#define IMAGE(NAME) \
  _binary_data_ ## NAME ## _data_start
#define LDLEN(NAME) \
  ((_binary_data_ ## NAME ## _data_end) - (_binary_data_ ## NAME ## _data_start))
#endif

EXTLD(1)
EXTLD(2)
EXTLD(3)
EXTLD(4)
EXTLD(5)
EXTLD(6)
EXTLD(7)
EXTLD(8)

EXTLD(empty)
EXTLD(unclick)
EXTLD(mine)
#endif
