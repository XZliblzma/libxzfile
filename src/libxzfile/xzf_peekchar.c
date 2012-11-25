/*
 * xzf_peekchar()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_peekchar(xzf_stream *strm)
{
	internal_lock(strm);
	const int ret = internal_peekc(strm);
	internal_unlock(strm);
	return ret;
}
