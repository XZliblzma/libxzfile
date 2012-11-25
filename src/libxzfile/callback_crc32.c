/*
 * Callback to calculate CRC32
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "sysdefs.h"
#include "xzfile.h"

#include <lzma.h>


extern void
xzf_cb_crc32(void *state, const unsigned char *buf, size_t size)
{
	uint32_t *crc = state;
	*crc = lzma_crc32(buf, size, *crc);
}
