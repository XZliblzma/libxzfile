/*
 * xzf_internal_seek()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern xzf_off
xzf_internal_seek(xzf_stream *strm, xzf_off offset, enum xzf_whence whence)
{
	if ((unsigned int)whence > XZF_SEEK_END || offset < -XZF_OFF_MAX) {
		strm->errnum = errno = EINVAL;
		return -1;
	}

	if ((strm->flags & XZF_SEEKABLE) == 0) {
		strm->errnum = errno = ESPIPE;
		return -1;
	}

	assert(strm->backend->seek != NULL);

	if (strm->out_next > strm->out_buf || strm->backend_peekout)
		if (xzf_internal_flush(strm, 0))
			return -1;

	if (strm->backend_peekin && xzf_internal_fill(strm, 0))
		return -1;

	switch (whence) {
		case XZF_SEEK_SET:
			if (offset < 0) {
				strm->errnum = errno = EINVAL;
				return -1;
			}

			break;

		case XZF_SEEK_CUR: {
			const size_t adjust = strm->in_end - strm->in_next;
			assert(adjust <= XZF_OFF_MAX);

			// This was checked earlier.
			assert(offset >= -XZF_OFF_MAX);

			// Catch integer overflows.
			if (offset < 0 && XZF_OFF_MAX + offset
					< (xzf_off)adjust) {
				strm->errnum = errno = EINVAL;
				return -1;
			}

			offset -= (xzf_off)adjust;
			break;
		}

		case XZF_SEEK_END:
			break;
	}

	// FIXME !!! Does the backend need a pointer to offset?
	const int errnum = strm->backend->seek(
			strm->state, &offset, whence);
	if (errnum != 0) {
		strm->errnum = errno = errnum;
		return -1;
	}

	strm->in_next = strm->in_end;

	return offset;
}
