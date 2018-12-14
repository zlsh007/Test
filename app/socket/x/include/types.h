/*
*   Copyright (C) 2002 - 2003 Ardis Technolgies <roman@ardistech.com>
 *  Copyright (C) 2007 - 2016 Vladislav Bolkhovitin
 *  Copyright (C) 2007 - 2016 SanDisk Corporation
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, version 2
 *  of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 */

#ifndef TYPES_H
#define TYPES_H

#include <sys/types.h>
#include <byteswap.h>
#include <endian.h>
#include <stdint.h>
#include <inttypes.h>
#include <types.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_le16(x)		bswap_16(x)
#define le16_to_cpu(x)		bswap_16(x)
#define cpu_to_le32(x)		bswap_32(x)
#define le32_to_cpu(x)		bswap_32(x)
#define cpu_to_be16(x)		(x)
#define be16_to_cpu(x)		(x)
#define cpu_to_be32(x)		(x)
#define be32_to_cpu(x)		(x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le16(x)		(x)
#define le16_to_cpu(x)		(x)
#define cpu_to_le32(x)		(x)
#define le32_to_cpu(x)		(x)
#define cpu_to_be16(x)		bswap_16(x)
#define be16_to_cpu(x)		bswap_16(x)
#define cpu_to_be32(x)		bswap_32(x)
#define be32_to_cpu(x)		bswap_32(x)
#else
#error "unknown endianess!"
#endif

void dumpdata(char *buf, int len)
{
	int i;
        if (len > 100)
                len = 96;
	for (i = 0; i < len; i++) {
		if (!(i%16) && i)
                        printf("\n");
		else if (!(i%8) && i)
                        printf("   ");
		printf("0x%02x ", buf[i]& 0xff);
	}
	printf("\n");
	return ;
}

typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;

#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif

#ifdef __CHECK_ENDIAN__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif

typedef u16 __bitwise __le16;
typedef u16 __bitwise __be16;
typedef u32 __bitwise __le32;
typedef u32 __bitwise __be32;
typedef u64 __bitwise __le64;
typedef u64 __bitwise __be64;

typedef u16 __bitwise __sum16;
typedef u32 __bitwise __wsum;
typedef int32_t s32;

#endif	/* TYPES_H */
