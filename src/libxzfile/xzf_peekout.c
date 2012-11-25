/*
 * xzf_peekout_start() and xzf_peekout_end()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern size_t
xzf_peekout_start(xzf_stream *strm, unsigned char **buf, size_t size)
{
	// NOTE: On success, the lock is left in locked state.
	// It will be unlocked in xzf_peekout_end().
	internal_lock(strm);

	assert(!strm->frontend_peekout);

	// FIXME? Test for XZF_WRITE?

	// The caller must not request more than the stream buffer size.
	if (size == 0 || size > strm->out_buf_size) {
		// FIXME: Should this affect strm->errnum?
		strm->errnum = errno = EINVAL;
		goto error;
	}

	// If there are enough bytes in strm->out_next, we can return quickly.
	size_t avail = strm->out_end - strm->out_next;
	if (avail >= size) {
		strm->frontend_peekout = true;
		*buf = strm->out_next;
		return avail;
	}

	// Flush the internal buffer to make more space in it.
	if (xzf_internal_flush(strm, size))
		goto error;

	assert(strm->out_next == strm->out_buf);
	assert(strm->out_next < strm->out_end);

	strm->frontend_peekout = true;
	*buf = strm->out_next;
	return strm->out_end - strm->out_next;

error:
	*buf = NULL;
	internal_unlock(strm);
	return 0;
}


extern int
xzf_peekout_end(xzf_stream *strm, size_t size)
{
	assert(strm->frontend_peekout);

	int ret;

	if (size > (size_t)(strm->out_end - strm->out_next)) {
		strm->errnum = errno = EINVAL;
		ret = -1;

	} else if ((strm->flags & (XZF_LINEBUF | XZF_UNBUF)) == 0) {
		// Never flush with fully buffered streams.
		strm->out_next += size;
		ret = 0;

	} else if (strm->flags & XZF_UNBUF) {
		// Always flush with unbuffered streams.
		strm->out_next += size;
		ret = xzf_internal_flush(strm, 0);

	} else if (memchr(strm->out_next, '\n', size) != NULL) {
		assert(strm->flags & XZF_LINEBUF);

		// With line-buffered streams, flushing is done if a newline
		// character was included in the buffer. This isn't what
		// people expect line-buffering to mean, but it keeps the
		// implementation simple also when the backend uses peeking.
		// The use of xzf_peekout_start() and _end() is probably
		// rare on line-buffered streams anyway.
		ret = xzf_internal_flush(strm, 0);
	}

	strm->frontend_peekout = false;
	internal_unlock(strm);
	return ret;
}
