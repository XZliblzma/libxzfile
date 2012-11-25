/*
 * xzf_lock(), xzf_trylock(), and xzf_unlock()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern void
xzf_lock(xzf_stream *strm)
{
#ifdef HAVE_PTHREAD
	if (strm->has_mutex) {
		const int ret = pthread_mutex_lock(&strm->mutex);
		assert(ret == 0);
		(void)ret;
	}
#else
	(void)strm;
#endif
}


extern int
xzf_trylock(xzf_stream *strm)
{
#ifdef HAVE_PTHREAD
	return strm->has_mutex ? pthread_mutex_trylock(&strm->mutex) : ENOTSUP;
#else
	(void)strm;
	return 0;
#endif
}


extern void
xzf_unlock(xzf_stream *strm)
{
#ifdef HAVE_PTHREAD
	if (strm->has_mutex) {
		const int ret = pthread_mutex_unlock(&strm->mutex);
		assert(ret == 0);
		(void)ret;
	}
#else
	(void)strm;
#endif
}
