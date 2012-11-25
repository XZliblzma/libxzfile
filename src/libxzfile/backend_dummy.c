/*
 * Backend that supports nothing
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "sysdefs.h"
#include "xzfile.h"


static const struct xzf_backend dummy_backend = {
	.version = 0,
};


extern xzf_stream *
xzf_dummy_open(void)
{
	return xzf_stream_init(NULL, &dummy_backend, NULL, 0, 0, 0);
}
