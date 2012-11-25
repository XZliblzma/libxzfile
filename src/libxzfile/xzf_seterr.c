/*
 * xzf_seterr()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_seterr(xzf_stream *strm, int errnum)
{
	internal_lock(strm);

	const int ret = strm->errnum;
	assert(ret != XZF_E_EOF);

	if (errnum == XZF_E_EOF) {
		strm->eof = true; // FIXME? Disallow XZF_E_EOF?
	} else {
		if (errnum == 0)
			strm->eof = false; // FIXME?

		strm->errnum = errnum;
	}

	internal_unlock(strm);
	return ret;
}
