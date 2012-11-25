/*
 * xzf_read()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


static size_t
read_via_buf(xzf_stream *strm, unsigned char *buf, size_t pos, size_t size)
{
	assert(pos < size);

	do {
		if (strm->in_next >= strm->in_end
				&& xzf_internal_fill(strm, 1))
			return pos;

		size_t copy_size = strm->in_end - strm->in_next;
		if (copy_size > size - pos)
			copy_size = size - pos;

		memcpy(buf + pos, strm->in_next, copy_size);
		strm->in_next += copy_size;
		pos += copy_size;

	} while (pos < size);

	return pos;
}


static size_t
read_directly(xzf_stream *strm, unsigned char *buf, size_t pos, size_t size)
{
	assert(pos < size);
	assert(strm->backend->read != NULL);

	if (xzf_internal_fill(strm, 0))
		return pos;

	do {
		size_t n = size - pos;
		const int errnum = strm->backend->read(
				strm->state, buf + pos, &n);
		pos += n;

		if (errnum != 0) {
			if (errnum == XZF_E_EOF)
				strm->eof = true;
			else
				strm->errnum = errnum;

			errno = errnum;
			return pos;
		}
	} while (pos < size);

	return pos;
}


extern size_t
xzf_read(xzf_stream *strm, void *bufptr, size_t size)
{
	internal_lock(strm);

	assert(!strm->frontend_peekin);

	unsigned char *buf = bufptr;
	size_t pos = 0;

	// Copy from the input buffer first if it isn't empty.
	if (strm->in_next < strm->in_end) {
		pos = strm->in_end - strm->in_next;
		if (pos > size)
			pos = size;

		memcpy(buf, strm->in_next, pos);
		strm->in_next += pos;
	}

	if (pos < size) {
		// If the total amount requested is greater than or equal
		// to the internal input buffer size, guess that the
		// application will request more data using big buffer sizes
		// and read the data directly from the backend instead of
		// copying it via the internal input buffer.
		if (size >= strm->in_buf_size && strm->backend->read != NULL)
			pos = read_directly(strm, buf, pos, size);
		else
			pos = read_via_buf(strm, buf, pos, size);
	}

	internal_unlock(strm);
	return pos;
}
