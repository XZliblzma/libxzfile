/*
 * xzf_putchar()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_putchar(xzf_stream *strm, unsigned char c)
{
	int ret = c;
	internal_lock(strm);

	assert(!strm->frontend_peekout);

	if (strm->out_next >= strm->out_end && xzf_internal_flush(strm, 1)) {
		ret = -1;
	} else {
		assert(strm->out_next < strm->out_end);
		*strm->out_next++ = c;

		// Handle unbuffered or line buffered stream.
		if (strm->flags & (XZF_UNBUF | XZF_LINEBUF))
			if ((strm->flags & XZF_UNBUF) || c == '\n')
				if (xzf_internal_flush(strm, 0))
					ret = -1;
	}

	internal_unlock(strm);
	return ret;
}
