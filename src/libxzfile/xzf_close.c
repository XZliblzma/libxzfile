/*
 * xzf_close()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


extern int
xzf_close(xzf_stream *strm, int cl_flags)
{
	// Notes:
	//   - If the stream isn't seekable, input and output are handled
	//     separately. Trying to fix the read position does nothing.
	//   - If the stream is seekable, input and output buffers aren't
	//     in use at the same time.

// TODO: Check all cases where similar flushing-like operation is needed:
// This function, xzf_flush, xzf_seek, xzf_internal_fill, xzf_internal_flush

	if (cl_flags & XZF_CL_FORGET) {
		// XZF_CL_SYNC is ignored if XZF_CL_FORGET
		// has been specified too.
		cl_flags &= ~XZF_CL_SYNC;

		// Purge the input and output buffers.
		strm->in_next = strm->in_end;
		strm->out_next = strm->out_buf;
	}

	if (strm->is_writing) {
		if (strm->frontend_peekout)
			(void)xzf_peekout_end(strm, 0);

		if (strm->out_next > strm->out_buf || strm->backend_peekout)
			(void)xzf_internal_flush(strm, 0);
	}

	if (strm->is_reading) {
		if (strm->frontend_peekin)
			xzf_peekin_end(strm, 0);

		if (strm->backend_peekin)
			(void)xzf_internal_fill(strm, 0);

		(void)xzf_internal_fixreadpos(strm);
	}

	int errnum = 0;
	if (strm->backend->close != NULL)
		errnum = strm->backend->close(strm->state, cl_flags);

	if (errnum == 0)
		errnum = strm->errnum;

	if (!strm->stream_is_external) {
#ifdef HAVE_PTHREAD
		const int mutex_ret = pthread_mutex_destroy(&strm->mutex);
		assert(mutex_ret == 0);
		(void)mutex_ret;
#endif

		free(strm->in_buf);
		free(strm->out_buf);
		free(strm);
	}

	if (errnum != 0)
		errno = errnum;

	return errnum;
}
