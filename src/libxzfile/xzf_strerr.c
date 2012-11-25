/*
 * xzf_strerr()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


// FIXME: Fix after gettext is configured for this package.
#define _(s) s
#define N_(s) s


// FIXME TODO: Just in case, use some less simple range for error codes
// e.g. [-8192, -4097] so that if something else uses negative errnos,
// we don't clash so easily hopefully.

static const char *err_msgs[] = {
	[-XZF_E_EOF]          = N_("End of input successfully reached"),
// 	[-XZF_E_UEEOF]        = N_("Unexpected end of input"),
	[-XZF_E_NOTREADABLE]  = N_("Not open for reading"),
	[-XZF_E_NOTWRITABLE]  = N_("Not open for writing"),
// 	[-XZF_E_FORMAT]       = N_("File format not recognized"),
	[-XZF_E_ZCORRUPT]     = N_("Compressed data is corrupt"),
	[-XZF_E_ZTRUNC]       = N_("Compressed data is truncated "
	                           "or otherwise corrupt"),
// 	[-XZF_E_ZOPNOTSUP]    = N_("Unsupported compression options"),
// 	[-XZF_E_ZMEMLIMIT]    = N_("Decompressor memory usage limit reached"),
	[-XZF_E_BUG]          = N_("Internal error (bug)"),
};


extern const char *
xzf_strerr(int errnum, xzf_errbuf *buf)
{
	if (errnum >= 0) {
		// System error number
#if defined(_WIN32) && !defined(__CYGWIN__)
		if (errnum < _sys_nerr)
			return _sys_errlist[errnum];

#elif defined(STRERROR_R_CHAR_P)
		return strerror_r(errnum, buf->buf, sizeof(buf->buf));

#else
		if (strerror_r(errnum, buf->buf, sizeof(buf->buf)) == 0)
			return buf->buf;
#endif

	} else {
		// xzfile error number
		int e = -errnum;
		if (e > 0 && (size_t)e < ARRAY_SIZE(err_msgs))
			return _(err_msgs[e]);
	}

	// Unknown error number
	xzf_sprintf(buf->buf, sizeof(buf->buf),
			_("Unknown error number %d"), errnum);
	return buf->buf;
}
