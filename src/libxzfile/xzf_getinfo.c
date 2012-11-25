/*
 * xzf_getinfo()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_getinfo(xzf_stream *strm, int key, void *value)
{
	int ret;
	internal_lock(strm);

	assert(!strm->frontend_peekin);
	assert(!strm->frontend_peekout);

	if (strm->backend->getinfo != NULL) {
		ret = strm->backend->getinfo(strm->state, key, value);
		if (ret != 0) {
			errno = ret;
			ret = -1;
		}
	} else {
		errno = XZF_E_NOKEY;
		ret = -1;
	}

	internal_unlock(strm);
	return ret;
}
