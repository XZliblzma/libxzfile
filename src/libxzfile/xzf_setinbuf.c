/*
 * xzf_setinbuf()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_setinbuf(xzf_stream *strm, size_t size)
{
	int ret = 0;
	internal_lock(strm);

	if (size == 0 || size > XZF_BUF_MAX) {
		errno = EINVAL;
		ret = -1;

	} else if ((strm->flags & XZF_READ) == 0) {
		errno = XZF_E_NOTREADABLE;
		ret = -1;

	} else if (strm->backend->peekin_start != NULL) {
		errno = ENOTSUP;
		ret = -1;

	} else if (size < (size_t)(strm->in_end - strm->in_next)) {
		errno = EINVAL;
		ret = -1;

	} else if (xzf_internal_fill(strm, 0)) {
		ret = -1;

	} else if (size != strm->in_buf_size) {
		unsigned char *buf = malloc(size);

		if (buf == NULL) {
			ret = -1;
		} else {
			// If there is unread data in the internal buffer,
			// copy it into the new buffer and update the
			// pointers.
			assert(strm->in_next == strm->in_buf);
			const size_t unread = strm->in_end - strm->in_buf;
			if (unread > 0)
				memcpy(buf, strm->in_buf, unread);

			free(strm->in_buf);

			strm->in_buf = buf;
			strm->in_buf_size = size;

			strm->in_next = strm->in_buf;
			strm->in_end = strm->in_buf + unread;
			strm->in_stop = strm->flags & XZF_THRSAFE
					? strm->in_next : strm->in_end;
		}
	}

	internal_unlock(strm);
	return ret;
}
