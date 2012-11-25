/*
 * xzf_flush()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_flush(xzf_stream *strm, int fl_flags)
{
	int ret = 0;
	internal_lock(strm);

	if ((strm->out_next > strm->out_buf || strm->backend_peekout)
			&& xzf_internal_flush(strm, 0)) {
		ret = -1;

	} else if (strm->backend_peekin && xzf_internal_fill(strm, 0)) {
		ret = -1;

	} else if (xzf_internal_fixreadpos(strm)) {
		ret = -1;

	} else if (strm->backend->flush != NULL) {
		const int errnum = strm->backend->flush(strm->state, fl_flags);
		if (errnum != 0) {
			strm->errnum = errno = errnum;
			ret = -1;
		}
	}

	internal_unlock(strm);
	return ret;
}
