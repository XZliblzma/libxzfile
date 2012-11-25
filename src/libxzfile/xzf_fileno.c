/*
 * xzf_fileno()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_fileno(xzf_stream *strm)
{
	int fd;
	if (xzf_getinfo(strm, XZF_KEY_FD, &fd)) {
		errno = ENOTSUP;
		return -1;
	}

	assert(fd >= 0);
	return fd;
}
