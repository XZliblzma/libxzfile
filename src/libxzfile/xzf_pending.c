/*
 * xzf_pending()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern size_t
xzf_pending(xzf_stream *strm)
{
	internal_lock(strm);
	const size_t size = strm->out_next - strm->out_buf;
	internal_unlock(strm);
	return size;
}
