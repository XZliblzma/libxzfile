/*
 * xzf_internal_fixreadpos()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_internal_fixreadpos(xzf_stream *strm)
{
	if (strm->in_next < strm->in_end && (strm->flags & XZF_FIXREADPOS)) {
		if (xzf_internal_seek(strm, 0, XZF_SEEK_CUR) == -1)
			return errno;
	}

	return 0;
}
