/*
 * xzf_setflags()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_setflags(xzf_stream *strm, int new_flags)
{
	int ret;

	// This might change the thread-safety flag so use xzf_lock()
	// to force locking.
	xzf_lock(strm);

	// FIXME? Other flags?
	const int cannot_change = ~(XZF_LINEBUF | XZF_UNBUF | XZF_THRSAFE);
	const int diff = new_flags ^ strm->flags;

	if (diff & cannot_change) {
		ret = -1;
		// FIXME? strm->errnum?
		errno = EINVAL;
	} else {
		strm->flags = new_flags;
		ret = 0;
	}

	// FIXME? Does this need to flush if buffering mode has been changed?
	// xzf_write() assumes that unbuffered stream has been flushed.

	xzf_unlock(strm);
	return ret;
}
