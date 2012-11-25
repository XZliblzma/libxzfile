/*
 * xzf_puts()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_puts(xzf_stream *strm, const char *str)
{
	return xzf_write(strm, str, strlen(str));
}
