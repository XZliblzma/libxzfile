/*
 * xzf_eof()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_eof(xzf_stream *strm)
{
	internal_lock(strm);
	const int ret = strm->eof && strm->in_next >= strm->in_end;
	internal_unlock(strm);
	return ret;
}
