/*
 * xzf_setoutbuf()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_setoutbuf(xzf_stream *strm, size_t size)
{
	int ret = 0;
	internal_lock(strm);

	if (size == 0 || size > XZF_BUF_MAX) {
		errno = EINVAL;
		ret = -1;

	} else if ((strm->flags & XZF_WRITE) == 0) {
		errno = XZF_E_NOTWRITABLE;
		ret = -1;

	} else if (strm->backend->peekout_start != NULL) {
		errno = ENOTSUP;
		ret = -1;

	} else if (xzf_internal_flush(strm, 0)) {
		ret = -1;

	} else if (size != strm->out_buf_size) {
		unsigned char *buf = malloc(size);

		if (buf == NULL) {
			ret = -1;
		} else {
			free(strm->out_buf);

			strm->out_buf = buf;
			strm->out_buf_size = size;

			strm->out_next = NULL;
			strm->out_stop = NULL;
			strm->out_end = NULL;
		}
	}

	internal_unlock(strm);
	return ret;
}
