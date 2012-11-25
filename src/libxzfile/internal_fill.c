/*
 * xzf_internal_fill()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


static int
fill_with_read(xzf_stream *strm, size_t min_fill)
{
	size_t pos = 0;
	if (strm->in_end > strm->in_next) {
		// There is some unused data in the buffer.
		// Move it to the beginning of the buffer.
		pos = strm->in_end - strm->in_next;
		memmove(strm->in_buf, strm->in_next, pos);
	}

	// Try to fill the buffer, but stop trying after there is at least
	// min_fill bytes, end of input is reached, or an error occurs.
	while (pos < min_fill) {
		size_t size = strm->in_buf_size - pos;
		const int errnum = strm->backend->read(strm->state,
				strm->in_buf + pos, &size);
		pos += size;

		if (errnum != 0) {
			strm->in_end = strm->in_buf + pos;

			if (errnum == XZF_E_EOF)
				strm->eof = true;
			else
				strm->errnum = errnum;

			errno = errnum;
			return pos == 0 ? -1 : 0; // FIXME?
		}
	}

	strm->in_end = strm->in_buf + pos;
	return 0;
}


static int
fill_with_peekin(xzf_stream *strm, size_t min_fill)
{
	// If previous peeking of input is active, end it first.
	if (strm->backend_peekin) {
		const size_t bytes_used = strm->in_next - strm->in_buf;
		const int errnum = strm->backend->peekin_end(
				strm->state, bytes_used);

		strm->in_end = NULL;
		strm->in_buf = NULL;
		strm->backend_peekin = false;

		if (errnum != 0) {
			assert(errnum != XZF_E_EOF);
			errno = strm->errnum = errnum;
			return -1;
		}
	}

	// Don't start a new peeking of input if
	// no input buffer was requested.
	if (min_fill == 0)
		return 0;

	size_t size = min_fill;
	const int errnum = strm->backend->peekin_start(strm->state,
			(const unsigned char **)&strm->in_buf, &size);

	// If we get less input than requested, the backend
	// must have returned the reason.
	assert(size >= min_fill || errnum != 0);

	// The size of the buffer has to fit into xzf_off and ptrdiff_t.
	if (size > XZF_BUF_MAX)
		size = XZF_BUF_MAX;

	if (errnum != 0) {
		if (errnum == XZF_E_EOF)
			strm->eof = true;
		else
			strm->errnum = errnum;

		errno = errnum;

		if (size == 0) {
			strm->in_buf = NULL;
			return -1;
		}
	}

	assert(strm->in_buf != NULL);
	strm->in_end = strm->in_buf + size;
	strm->backend_peekin = true;
	return 0;
}


extern int
xzf_internal_fill(xzf_stream *strm, size_t min_fill)
{
	// If an error has already occurred, return immediately.
	// FIXME: This isn't so simple. E.g. what about EOF flag?
	if (strm->errnum != 0
			|| (strm->eof && strm->in_next >= strm->in_end)) {
		errno = strm->errnum != 0 ? strm->errnum : XZF_E_EOF;
		return -1;
	}

	if (!strm->is_reading) {
		// Check that the stream is open for reading.
		if ((strm->flags & XZF_READ) == 0) {
			errno = strm->errnum = XZF_E_NOTREADABLE;
			return -1;
		}

		// If it is a seekable read-write stream, we need to fix
		// the position before reading anything.
		//
		// FIXME: Is seeking better than flushing?
		if (strm->is_writing && (strm->flags & XZF_SEEKABLE)) {
			const int ret = xzf_internal_seek(
					strm, 0, XZF_SEEK_CUR);
			if (ret != 0)
				return ret;

			strm->is_writing = false;
		}

		strm->is_reading = true;
	}

	assert(min_fill <= strm->in_buf_size);

	// Get data from the backend.
	const int ret = strm->backend->peekin_start != NULL
			? fill_with_peekin(strm, min_fill)
			: fill_with_read(strm, min_fill);

	// Don't allow direct access to the buffer via API macros
	// if the thread-safety flag has been set.
	strm->in_next = strm->in_buf;
	strm->in_stop = strm->flags & XZF_THRSAFE
			? strm->in_next : strm->in_end;

	return ret;
}
