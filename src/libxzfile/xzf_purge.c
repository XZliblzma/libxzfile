/*
 * xzf_purge()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern void
xzf_purge(xzf_stream *strm, int flags)
{
	internal_lock(strm);

	if (flags & XZF_READ)
		strm->in_next = strm->in_end;

	if (flags & XZF_WRITE)
		strm->out_next = strm->out_buf;

	internal_unlock(strm);
}
