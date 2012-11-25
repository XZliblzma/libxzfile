/*
 * xzf_getchar()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_getchar(xzf_stream *strm)
{
	internal_lock(strm);
	assert(!strm->frontend_peekin);
	const int ret = internal_getc(strm);
	internal_unlock(strm);
	return ret;
}
