/*
 * xzf_internal_flush()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


static int
flush_with_write(xzf_stream *strm)
{
	// Call backend->write() only if there is something to write.
	if (strm->out_next > strm->out_buf) {
		const size_t write_size = strm->out_next - strm->out_buf;
		strm->out_next = strm->out_buf;

		const int errnum = strm->backend->write(
				strm->state, strm->out_buf, write_size);
		if (errnum != 0) {
			assert(errnum != XZF_E_EOF);
			errno = strm->errnum = errnum;
			strm->out_end = strm->out_buf;
			return -1;
		}
	}

	strm->out_end = strm->out_buf + strm->out_buf_size;
	return 0;
}


static int
flush_with_peekout(xzf_stream *strm, size_t min_size)
{
	if (strm->backend_peekout) {
		const size_t bytes_written = strm->out_next - strm->out_buf;
		const int errnum = strm->backend->peekout_end(
				strm->state, bytes_written);

		strm->out_end = NULL;
		strm->out_buf = NULL;
		strm->backend_peekout = false;

		if (errnum != 0) {
			assert(errnum != XZF_E_EOF);
			errno = strm->errnum = errnum;
			return -1;
		}
	}

	if (min_size == 0)
		return 0;

	size_t size = min_size;
	const int errnum = strm->backend->peekout_start(
			strm->state, &strm->out_buf, &size);

	// If we get less output than requested, the backend
	// must have returned the reason.
	assert(size >= min_size || errnum != 0);

	// The size of the buffer has to fit into xzf_off and ptrdiff_t.
	if (size > XZF_BUF_MAX)
		size = XZF_BUF_MAX;

	if (errnum != 0) {
		assert(errnum != XZF_E_EOF);
		errno = strm->errnum = errnum;
		if (size == 0) {
			strm->out_buf = NULL;
			return -1;
		}
	}

	assert(strm->out_buf != NULL);
	strm->out_end = strm->out_buf + strm->out_buf_size;
	strm->backend_peekout = true;
	return 0;
}


extern int
xzf_internal_flush(xzf_stream *strm, size_t min_size)
{
	// If an error has already occurred, return immediately.
	if (strm->errnum != 0) {
		errno = strm->errnum;
		return -1;
	}

	if (!strm->is_writing) {
		// Check that the stream is open for writing.
		if ((strm->flags & XZF_WRITE) == 0) {
			errno = strm->errnum = XZF_E_NOTWRITABLE;
			return -1;
		}

		// If it is a seekable read-write stream, we need to fix
		// the position before writing anything.
		if (strm->is_reading && (strm->flags & XZF_SEEKABLE)) {
			const int ret = xzf_internal_seek(
					strm, 0, XZF_SEEK_CUR);
			if (ret != 0)
				return ret;

			strm->is_reading = false;
		}

		strm->is_writing = true;
	}

	assert(min_size <= strm->out_buf_size);

	const int ret = strm->backend->peekout_start != NULL
			? flush_with_peekout(strm, min_size)
			: flush_with_write(strm);

	// Don't allow direct access to the buffer via API macros
	// if the output is line-buffered or unbuffered or if
	// the thread-safety flag has been set.
	strm->out_next = strm->out_buf;
	strm->out_stop = strm->flags & (XZF_LINEBUF | XZF_UNBUF | XZF_THRSAFE)
			? strm->out_next : strm->out_end;

	return ret;
}
