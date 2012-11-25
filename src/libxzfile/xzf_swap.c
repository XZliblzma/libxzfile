/*
 * xzf_swap()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern void
xzf_swap(xzf_stream *s1, xzf_stream *s2)
{
	if (s1 == s2)
		return;

	// Use xzf_lock() because it is possible that only one of the two
	// streams has XZF_THRSAFE set, which would break unlocking after
	// the structure contents have been swapped.
	xzf_lock(s1);
	xzf_lock(s2);

#ifdef HAVE_PTHREAD
	// Don't copy the mutex.
	const size_t size = offsetof(xzf_stream, mutex);
#else
	const size_t size = sizeof(xzf_stream);
#endif

	unsigned char buf[size];
	memcpy(buf, s1, size);
	memcpy(s1, s2, size);
	memcpy(s2, buf, size);

	xzf_unlock(s2);
	xzf_unlock(s1);
}
