#pragma once

#ifndef __sgi
#include <endian.h>
#else 
#include <netinet/in.h>
#include <inttypes.h>

#define	__byte_swap_u64_constant(x) \
	(\
	 ((((x) & 0xff00000000000000ull) >> 56) | \
	  (((x) & 0x00ff000000000000ull) >> 40) | \
	  (((x) & 0x0000ff0000000000ull) >> 24) | \
	  (((x) & 0x000000ff00000000ull) >>  8) | \
	  (((x) & 0x00000000ff000000ull) <<  8) | \
	  (((x) & 0x0000000000ff0000ull) << 24) | \
	  (((x) & 0x000000000000ff00ull) << 40) | \
	  (((x) & 0x00000000000000ffull) << 56)))
#define	__byte_swap_u32_constant(x) \
	(\
	((((x) & 0xff000000) >> 24) | \
	 (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | \
	 (((x) & 0x000000ff) << 24)))
#define	__byte_swap_u16_constant(x) \
	(\
	((((x) & 0xff00) >> 8) | \
	 (((x) & 0x00ff) << 8)))
#define	bswap64(x) __byte_swap_u64_constant(x)
#define	bswap32(x) __byte_swap_u32_constant(x)
#define	bswap16(x) __byte_swap_u16_constant(x)

#define htobe16(x)	(x)
#define htobe32(x)	(x)
#define htobe64(x)	(x)
#define htole16(x)	bswap16((x))
#define htole32(x)	bswap32((x))
#define htole64(x)	bswap64((x))


#define be16toh(x)	htobe16(x)
#define be32toh(x)	htobe32(x)
#define be64toh(x)	htobe64(x)
#define le16toh(x)	htole16(x)
#define le32toh(x)	htole32(x)
#define le64toh(x)	htole64(x)
#endif

char *strsep(char **stringp, const char *delim);