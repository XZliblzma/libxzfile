/*
 * xzf_getoutbuf()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern size_t
xzf_getoutbuf(xzf_stream *strm)
{
	size_t size;
	internal_lock(strm);

	if ((strm->flags & XZF_WRITE) == 0) {
		errno = XZF_E_NOTWRITABLE;
		size = 0;
	} else {
		size = strm->out_buf_size;
	}

	internal_unlock(strm);
	return size;
}
