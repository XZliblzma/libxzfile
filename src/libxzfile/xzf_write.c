/*
 * xzf_write()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


static int
write_via_buf(xzf_stream *strm, const unsigned char *buf, size_t size)
{
	while (size > 0) {
		if (strm->out_next >= strm->out_end
				&& xzf_internal_flush(strm, 1))
			return -1;

		assert(strm->out_next < strm->out_end);

		size_t copy_size = strm->out_end - strm->out_next;
		if (copy_size > size)
			copy_size = size;

		memcpy(strm->out_next, buf, copy_size);
		strm->out_next += copy_size;
		buf += copy_size;
		size -= copy_size;
	}

	return 0;
}


static int
write_linebuf(xzf_stream *strm, const unsigned char *buf, size_t size)
{
	// FIXME? This could be optimized to avoid copying the data
	// to the internal buffer in some cases.

	while (size > 0) {
		const unsigned char *p = memchr(buf, '\n', size);
		size_t linesize = p == NULL ? size : (size_t)(p - buf) + 1;

		if (write_via_buf(strm, buf, linesize))
			return -1;

		buf += linesize;
		size -= linesize;

		// FIXME? Avoid useless flush when write_via_buf() has
		// flushed it as the last step (requires that the buffer
		// size matches)? Maybe not worth it.
		//
		// Don't request for more output space if we don't have more
		// to write. The request could give ENOSPC which we don't
		// want when we don't have anything left to write.
		if (p != NULL && xzf_internal_flush(strm, size > 0))
			return -1;
	}

	return 0;
}


static int
write_unbuf(xzf_stream *strm, const unsigned char *buf, size_t size)
{
	const int errnum = strm->backend->write(strm->state, buf, size);
	if (errnum != 0) {
		assert(errnum != XZF_E_EOF);
		errno = strm->errnum = errnum;
		return -1;
	}

	return 0;
}


static int
write_fullybuf(xzf_stream *strm, const unsigned char *buf, size_t size)
{
	if (size < strm->out_buf_size || strm->backend->write == NULL) {
		// The total amount to write is less than the size of the
		// internal buffer. Copy the new data to the internal buffer,
		// possibly flushing to make more space.
		return write_via_buf(strm, buf, size);
	}

	// The total amount to write is at least buf_size bytes.
	// Flush the data already in the buffer and write the
	// new data directly to the backend. Flushing is required
	// also to check that the stream is open for writing.
	if (xzf_internal_flush(strm, 0)
			|| write_unbuf(strm, buf, size))
		return -1;

	return 0;
}


extern int
xzf_write(xzf_stream *strm, const void *buf, size_t size)
{
	int ret;
	internal_lock(strm);

	assert(!strm->frontend_peekout);

/* FIXME?
	const int ret = strm->write_func(strm, buf, size);
*/

	if ((strm->flags & (XZF_LINEBUF | XZF_UNBUF)) == 0)
		ret = write_fullybuf(strm, buf, size);
	else if (strm->flags & XZF_LINEBUF)
		ret = write_linebuf(strm, buf, size);
	else if (strm->backend->write == NULL)
		ret = write_via_buf(strm, buf, size);
	else
		ret = write_unbuf(strm, buf, size);

	internal_unlock(strm);
	return ret;
}
