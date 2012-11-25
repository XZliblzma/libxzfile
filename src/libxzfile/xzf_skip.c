/*
 * xzf_skip()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


static xzf_off
skip_by_reading(xzf_stream *strm, xzf_off left)
{
	assert(left >= 0);

	while (left > 0) {
		if (strm->in_next >= strm->in_end
				&& xzf_internal_fill(strm, 1))
			return left;

		const size_t avail = strm->in_end - strm->in_next;
		const size_t skip = avail < (unsigned long long)left
				? avail : left;
		strm->in_next += skip;
		left -= skip;
	}

	return 0;
}


extern xzf_off
xzf_skip(xzf_stream *strm, xzf_off amount)
{
	// FIXME TODO: Some backends could provide fast skipping even when
	// they cannot support seeking.

	if (amount < 0) {
		// FIXME: strm->errnum?
		errno = EINVAL;
		return -1;
	}

	internal_lock(strm);

	if (amount == 0) {
		if ((strm->flags & XZF_READ) == 0) {
			// FIXME: strm->errnum?
			errno = XZF_E_NOTREADABLE;
			amount = -1;
		}

	} else if (strm->flags & XZF_SEEKABLE) {
		// FIXME? Allow seeking past the end of the file?
		if (xzf_seek(strm, amount, XZF_SEEK_CUR)) // FIXME _internal?
			amount = -1;

	} else {
		amount -= skip_by_reading(strm, amount);
	}

	internal_unlock(strm);
	return 0;
}
