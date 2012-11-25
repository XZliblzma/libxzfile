/*
 * xzf_off_abs()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern xzf_off
xzf_off_abs(xzf_off pos, xzf_off end, xzf_off offset, enum xzf_whence whence)
{
	if (pos < 0 || offset < -XZF_OFF_MAX)
		return -1;

	xzf_off ret = -1;

	switch (whence) {
	case XZF_SEEK_SET:
		if (offset >= 0)
			ret = offset;

		break;

	case XZF_SEEK_END:
		// Negative end indicates that XZF_SEEK_END isn't supported.
		if (end < 0)
			break;

		pos = end;

	// Fall through

	case XZF_SEEK_CUR:
		if (offset < 0) {
			offset = -offset;
			if (pos >= offset)
				ret = pos - offset;
		} else {
			if (pos <= XZF_OFF_MAX - offset)
				ret = pos + offset;
		}

		break;
	}

	return ret;
}
