/*
 * xzf_geterr()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_geterr(xzf_stream *strm)
{
	internal_lock(strm);

	const int ret = strm->errnum;
	assert(ret != XZF_E_EOF);

	internal_unlock(strm);
	return ret;
}
