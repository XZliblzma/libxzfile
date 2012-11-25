/*
 * xzf_peekin_start() and xzf_peekin_end()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern size_t
xzf_peekin_start(xzf_stream *strm, const unsigned char **buf, size_t size)
{
	// NOTE: On success, the lock is left in locked state.
	// It will be unlocked in xzf_peekin_end().
	internal_lock(strm);

	assert(!strm->frontend_peekin);

	// FIXME? Test for XZF_READ?
	// Probably yes, in_buf_size might be garbage.

	// The caller must not request more than the stream buffer size.
	if (size == 0 || size > strm->in_buf_size) {
		// FIXME: Should this affect strm->errnum?
		errno = EINVAL;
		goto error;
	}

	// If there are enough bytes in strm->in_next, we can return quickly.
	const size_t avail = strm->in_end - strm->in_next;
	if (avail >= size) {
		strm->frontend_peekin = true;
		*buf = strm->in_next;
		return avail;
	}

	// We need to read more data from the backend.
	if (xzf_internal_fill(strm, size))
		goto error;

	assert(strm->in_next == strm->in_buf);
	assert(strm->in_next < strm->in_end);

	strm->frontend_peekin = true;
	*buf = strm->in_next;
	return strm->in_end - strm->in_next;

error:
	*buf = NULL;
	internal_unlock(strm);
	return 0;
}


extern void
xzf_peekin_end(xzf_stream *strm, size_t bytes_used)
{
	assert(strm->frontend_peekin);
	assert(bytes_used <= (size_t)(strm->in_end - strm->in_next));

	strm->in_next += bytes_used;
	strm->frontend_peekin = false;

	internal_unlock(strm);
}
