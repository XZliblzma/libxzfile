/*
 * xzf_getln()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


// FIXME TODO: return values:
//  - OK (0)
//  - EOF
//  - EOF after reading at least one byte
//  - buf too small
//  - '\0' found
//  - I/O error (-1)
extern int
xzf_getln(xzf_stream *strm, char *buf, size_t size)
{
	if (size == 0) {
		// FIXME: strm->errnum?
		errno = EINVAL;
		FIXME;
	}

	--size;
	size_t pos = 0;
	int ret = 0;

	internal_lock(strm);

	while (pos < size) {
		const int c = internal_getc(strm);

		if (c == '\n')
			break;

		if (c == '\0') {
			FIXME;
			break;
		}

		// FIXME: Will internal_getc() return -1 on EOF?
		if (c == -1) {
			ret = -1;
			break;
		}

		buf[pos++] = c;
	}

	internal_unlock(strm);

	buf[pos] = '\0';
	return ret;
}
