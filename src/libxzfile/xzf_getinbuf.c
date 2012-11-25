/*
 * xzf_getinbuf()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern size_t
xzf_getinbuf(xzf_stream *strm)
{
	size_t size;
	internal_lock(strm);

	if ((strm->flags & XZF_READ) == 0) {
		errno = XZF_E_NOTREADABLE;
		size = 0;
	} else {
		size = strm->in_buf_size;
	}

	internal_unlock(strm);
	return size;
}
