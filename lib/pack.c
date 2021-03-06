/* Copyright (c) 2011, onitake <onitake@gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *    1. Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright notice, this list
 *       of conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "pack.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/param.h>
#include <errno.h>

#if defined(__BYTE_ORDER)
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define SYSTEM_LITTLE 1
# elif __BYTE_ORDER == __BIG_ENDIAN
#  define SYSTEM_LITTLE 0
# endif
#elif defined(BYTE_ORDER)
# if BYTE_ORDER == LITTLE_ENDIAN
#  define SYSTEM_LITTLE 1
# elif BYTE_ORDER == BIG_ENDIAN
#  define SYSTEM_LITTLE 0
# endif
#endif /*__BYTE_ORDER*/

#if !defined(SYSTEM_LITTLE)
# error Unknown byte order!
#endif

typedef union {
	float f;
	uint32_t i;
} float_to_uint32_t;

enum {
	PACK_BYTE = 'c',
	PACK_UBYTE = 'C',
	PACK_SHORT = 's',
	PACK_USHORT = 'S',
	PACK_INT = 'i',
	PACK_UINT = 'I',
	PACK_LONG = 'l',
	PACK_ULONG = 'L',
	PACK_FLOAT = 'f',
	PACK_DOUBLE = 'd',
	PACK_SKIP = ' ',
	PACK_LITTLE = '<',
	PACK_BIG = '>',
	PACK_SYS = '=',
};

static size_t pack_length(const char *format) {
	size_t length = 0;
	const char *fmt;
	for (fmt = format; *fmt; fmt++) {
		switch (*fmt) {
			case PACK_BYTE:
			case PACK_UBYTE:
			case PACK_SKIP:
				length++;
				break;
			case PACK_SHORT:
			case PACK_USHORT:
				length += 2;
				break;
			case PACK_INT:
			case PACK_UINT:
			case PACK_FLOAT:
				length += 4;
				break;
			case PACK_LONG:
			case PACK_ULONG:
			case PACK_DOUBLE:
				length += 8;
				break;
		}
	}
	return length;
}

uint8_t *pack(const char *format, uint8_t *data, size_t *size, ...) {
	size_t length = pack_length(format);
	if (data && (!size || (length > *size))) {
		errno = EINVAL;
		return NULL;
	}
	uint8_t *ret;
	if (!data) {
		ret = malloc(length);
		if (!ret) {
			errno = ENOMEM;
			return NULL;
		}
	} else {
		ret = data;
	}
	
	int little = SYSTEM_LITTLE;
	va_list args;
	va_start(args, size);
	
	uint8_t *out = ret;
	const char *fmt;
	for (fmt = format; *fmt; fmt++) {
		switch (*fmt) {
			case PACK_SKIP: {
				out++;
			} break;
			case PACK_BYTE:
			case PACK_UBYTE: {
				unsigned int value = va_arg(args, unsigned int);
				out[0] = value & 0xff;
				out++;
			} break;
			case PACK_SHORT:
			case PACK_USHORT: {
				unsigned int value = va_arg(args, unsigned int);
				if (little) {
					out[0] = value & 0xff;
					out[1] = (value >> 8) & 0xff;
				} else {
					out[0] = (value >> 8) & 0xff;
					out[1] = value & 0xff;
				}
				out += 2;
			} break;
			case PACK_INT:
			case PACK_UINT: {
				uint32_t value = va_arg(args, uint32_t);
				if (little) {
					out[0] = value & 0xff;
					out[1] = (value >> 8) & 0xff;
					out[2] = (value >> 16) & 0xff;
					out[3] = (value >> 24) & 0xff;
				} else {
					out[0] = (value >> 24) & 0xff;
					out[1] = (value >> 16) & 0xff;
					out[2] = (value >> 8) & 0xff;
					out[3] = value & 0xff;
				}
				out += 4;
			} break;
			case PACK_FLOAT: {
				float_to_uint32_t value;
				value.f = (float) va_arg(args, double);
				if (little) {
					out[0] = value.i & 0xff;
					out[1] = (value.i >> 8) & 0xff;
					out[2] = (value.i >> 16) & 0xff;
					out[3] = (value.i >> 24) & 0xff;
				} else {
					out[0] = (value.i >> 24) & 0xff;
					out[1] = (value.i >> 16) & 0xff;
					out[2] = (value.i >> 8) & 0xff;
					out[3] = value.i & 0xff;
				}
				out += 4;
			} break;
			case PACK_LONG:
			case PACK_ULONG:
			case PACK_DOUBLE: {
				uint64_t value = va_arg(args, uint64_t);
				if (little) {
					out[0] = value & 0xff;
					out[1] = (value >> 8) & 0xff;
					out[2] = (value >> 16) & 0xff;
					out[3] = (value >> 24) & 0xff;
					out[4] = (value >> 32) & 0xff;
					out[5] = (value >> 40) & 0xff;
					out[6] = (value >> 48) & 0xff;
					out[7] = (value >> 56) & 0xff;
				} else {
					out[0] = (value >> 56) & 0xff;
					out[1] = (value >> 48) & 0xff;
					out[2] = (value >> 40) & 0xff;
					out[3] = (value >> 32) & 0xff;
					out[4] = (value >> 24) & 0xff;
					out[5] = (value >> 16) & 0xff;
					out[6] = (value >> 8) & 0xff;
					out[7] = value & 0xff;
				}
				out += 8;
			} break;
			case PACK_LITTLE: {
				little = 1;
			} break;
			case PACK_BIG: {
				little = 0;
			} break;
			case PACK_SYS: {
				little = SYSTEM_LITTLE;
			} break;
		}
	}
	
	va_end(args);
	if (size) {
		*size = length;
	}
	return ret;
}

int unpack(const char *format, const uint8_t *data, size_t *size, ...) {
	size_t length = pack_length(format);
	if (data && (!size || (length > *size))) {
		errno = EINVAL;
		return -1;
	}

	int little = SYSTEM_LITTLE;
	va_list args;
	va_start(args, size);
	
	const uint8_t *in = (const uint8_t *) data;
	const char *fmt;
	for (fmt = format; *fmt; fmt++) {
		switch (*fmt) {
			case PACK_SKIP: {
				in++;
			} break;
			case PACK_BYTE:
			case PACK_UBYTE: {
				uint8_t *value = va_arg(args, uint8_t *);
				*value = in[0];
				in++;
			} break;
			case PACK_SHORT:
			case PACK_USHORT: {
				uint16_t *value = va_arg(args, uint16_t *);
				if (little) {
					*value = (uint16_t) in[0] | ((uint16_t) in[1] << 8);
				} else {
					*value = (uint16_t) in[1] | ((uint16_t) in[0] << 8);
				}
				in += 2;
			} break;
			case PACK_INT:
			case PACK_UINT:
			case PACK_FLOAT: {
				uint32_t *value = va_arg(args, uint32_t *);
				if (little) {
					*value = (uint32_t) in[0] | ((uint32_t) in[1] << 8) | ((uint32_t) in[2] << 16) | ((uint32_t) in[3] << 24);
				} else {
					*value = (uint32_t) in[3] | ((uint32_t) in[2] << 8) | ((uint32_t) in[1] << 16) | ((uint32_t) in[0] << 24);
				}
				in += 4;
			} break;
			case PACK_LONG:
			case PACK_ULONG:
			case PACK_DOUBLE: {
				uint64_t *value = va_arg(args, uint64_t *);
				if (little) {
					*value = (uint64_t) in[0] | ((uint64_t) in[1] << 8) | ((uint64_t) in[2] << 16) | ((uint64_t) in[3] << 24) | ((uint64_t) in[4] << 32) | ((uint64_t) in[5] << 40) | ((uint64_t) in[6] << 48) | ((uint64_t) in[7] << 56);
				} else {
					*value = (uint64_t) in[7] | ((uint64_t) in[6] << 8) | ((uint64_t) in[5] << 16) | ((uint64_t) in[4] << 24) | ((uint64_t) in[3] << 32) | ((uint64_t) in[2] << 40) | ((uint64_t) in[1] << 48) | ((uint64_t) in[0] << 56);
				}
				in += 8;
			} break;
			case PACK_LITTLE: {
				little = 1;
			} break;
			case PACK_BIG: {
				little = 0;
			} break;
			case PACK_SYS: {
				little = SYSTEM_LITTLE;
			} break;
		}
	}
	
	va_end(args);
	return 0;
}
