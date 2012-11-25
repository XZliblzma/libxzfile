/*
 * xzf_seek()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern xzf_off
xzf_seek(xzf_stream *strm, xzf_off offset, enum xzf_whence whence)
{
	internal_lock(strm);
	offset = xzf_internal_seek(strm, offset, whence);
	internal_unlock(strm);
	return offset;



/*
	internal_lock(strm);

	if ((unsigned int)whence > XZF_SEEK_END || offset < -XZF_OFF_MAX) {
		strm->errnum = errno = EINVAL;
		goto error;
	}

	if ((strm->flags & XZF_SEEKABLE) == 0) {
		strm->errnum = errno = ESPIPE;
		goto error;
	}

	assert(strm->backend->seek != NULL);

	if (strm->out_next > strm->out_buf || strm->backend_peekout)
		if (xzf_internal_flush(strm, 0))
			goto error;

	if (strm->backend_peekin && xzf_internal_fill(strm, 0))
		goto error;

	switch (whence) {
		case XZF_SEEK_SET:
			if (offset < 0) {
				strm->errnum = errno = EINVAL;
				goto error;
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
				goto error;
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
		goto error;
	}

	strm->in_next = strm->in_end;

	internal_unlock(strm);
	return offset;

error:
	internal_unlock(strm);
	return -1;
*/
}
