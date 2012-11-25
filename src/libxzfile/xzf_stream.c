/*
 * Allocate and initialize a xzf_stream
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"


#ifdef HAVE_PTHREAD
static int
init_mutex(pthread_mutex_t *mutex)
{
	// It must be OK to lock the stream multiple times in the same
	// thread so we need a recursive mutex.
	pthread_mutexattr_t attr;
	int ret = pthread_mutexattr_init(&attr);
	if (ret != 0) {
		errno = ret;
		return -1;
	}

	ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	if (ret == 0)
		ret = pthread_mutex_init(mutex, &attr);

	pthread_mutexattr_destroy(&attr);

	if (ret != 0) {
		errno = ret;
		return -1;
	}

	return 0;
}
#else
#	define init_mutex(mutex) 0
#endif


extern xzf_stream_mem *
xzf_stream_prealloc(size_t in_buf_size, size_t out_buf_size)
{
	if (in_buf_size > XZF_BUF_MAX || out_buf_size > XZF_BUF_MAX) {
		errno = EINVAL;
		return NULL;
	}

	xzf_stream *strm = malloc(sizeof(*strm));
	if (strm == NULL)
		return NULL;

	memset(strm, 0, sizeof(*strm));

	unsigned char *in_buf = NULL;
	unsigned char *out_buf = NULL;

	if (in_buf_size > 0 && (in_buf = malloc(in_buf_size)) == NULL)
		goto error;

	strm->in_buf = in_buf;
	strm->in_buf_size = in_buf_size;

	if (out_buf_size > 0 && (out_buf = malloc(out_buf_size)) == NULL)
		goto error;

	strm->out_buf = out_buf;
	strm->out_buf_size = out_buf_size;

	if (init_mutex(&strm->mutex))
		goto error;

	return (xzf_stream_mem *)strm;

error:
	{
		const int saved_errno = errno;
		free(in_buf);
		free(out_buf);
		free(strm);
		errno = saved_errno;
		return NULL;
	}
}


extern void
xzf_stream_free(xzf_stream_mem *mem)
{
	xzf_stream *strm = (xzf_stream *)mem;
	if (strm == NULL)
		return;

	// Saving of errno is a special convenience feature of this function.
	const int saved_errno = errno;

	assert(strm->backend == NULL);

#ifdef HAVE_PTHREAD
	const int mutex_ret = pthread_mutex_destroy(&strm->mutex);
	assert(mutex_ret == 0);
	(void)mutex_ret;
#endif

	free(strm->in_buf);
	free(strm->out_buf);
	free(strm);

	errno = saved_errno;
}


static bool
are_args_valid(const struct xzf_backend *backend, int flags,
		size_t in_buf_size, size_t out_buf_size)
{
	// TODO: Pass thru for XZF_APPEND etc.
	const int supported_flags
			= XZF_RW | XZF_SEEKABLE | XZF_FIXREADPOS
			| XZF_LINEBUF | XZF_UNBUF | XZF_THRSAFE;

	if (flags & ~supported_flags)
		return false;

	if (flags & XZF_READ) {
		if (backend->read == NULL && backend->peekin_start == NULL)
			return false;

		if (backend->peekin_start != NULL
				&& backend->peekin_end == NULL)
			return false;

		if (in_buf_size == 0 || in_buf_size > XZF_BUF_MAX)
			return false;
	}

	if (flags & XZF_WRITE) {
		if (backend->write == NULL && backend->peekout_start == NULL)
			return false;

		if (backend->peekout_start != NULL
				&& backend->peekout_end == NULL)
			return false;

		if (out_buf_size == 0 || out_buf_size > XZF_BUF_MAX)
			return false;
	}

	return true;
}


extern xzf_stream *
xzf_stream_init(xzf_stream_mem *mem,
		const struct xzf_backend *backend, void *state, int flags,
		size_t in_buf_size, size_t out_buf_size)
{
	// xzf_stream_mem is an alias for xzf_stream. It is done this way
	// to prevent callers from accidentally using xzf_stream_mem with
	// functions other than xzf_stream_init() and xzf_stream_free().
	xzf_stream *strm = (xzf_stream *)mem;

	// If preallocation was used, use the preallocation sizes unless
	// the backend uses peeking.
	if (strm != NULL) {
		if (backend->peekin_start == NULL)
			in_buf_size = strm->in_buf_size;

		if (backend->peekout_start == NULL)
			out_buf_size = strm->out_buf_size;
	}

	if (!are_args_valid(backend, flags, in_buf_size, out_buf_size))
		goto invalid_arg;

	// Unreadable or unseekable streams cannot fix the read position
	// when flushing or closing.
	const int f = XZF_READ | XZF_SEEKABLE;
	if ((flags & f) != f)
		flags &= ~XZF_FIXREADPOS;

	// Input buffer
	if ((flags & XZF_READ) == 0 || backend->peekin_start != NULL) {
		if ((flags & XZF_READ) == 0)
			in_buf_size = 0;

		if (strm != NULL) {
			free(strm->in_buf);
			strm->in_buf = NULL;
		}
	}

	// Output buffer
	if ((flags & XZF_WRITE) == 0 || backend->peekout_start != NULL) {
		if ((flags & XZF_WRITE) == 0)
			out_buf_size = 0;

		if (strm != NULL) {
			free(strm->out_buf);
			strm->out_buf = NULL;
		}
	}

	// If preallocation wasn't used, allocate the required memory now.
	if (strm == NULL) {
		mem = xzf_stream_prealloc(in_buf_size, out_buf_size);
		strm = (xzf_stream *)mem;
		if (strm == NULL)
			return NULL;
	}

	strm->backend = backend;
	strm->state = state;
	strm->flags = flags;

	strm->in_buf_size = in_buf_size;
	strm->out_buf_size = out_buf_size;

	return strm;

invalid_arg:
	errno = EINVAL;

	if (mem != NULL)
		xzf_stream_free(mem);

	return NULL;
}


extern xzf_stream *
xzf_stream_init_st(xzf_stream_mem_st *mem,
		const struct xzf_backend *backend, void *state, int flags,
		unsigned char *in_buf, size_t in_buf_size,
		unsigned char *out_buf, size_t out_buf_size)
{
	static_assert(sizeof(xzf_stream) <= sizeof(xzf_stream_mem_st),
			"xzf_stream_mem_st is too small");

	xzf_stream *strm = (xzf_stream *)mem;

	if (!are_args_valid(backend, flags, in_buf_size, out_buf_size)
			|| ((flags & XZF_READ)
				&& backend->peekin_start == NULL
				&& in_buf == NULL)
			|| ((flags & XZF_WRITE)
				&& backend->peekout_start == NULL
				&& out_buf == NULL)
			|| strm == NULL) {
		errno = EINVAL;
		return NULL;
	}

	if ((flags & XZF_READ) == 0) {
		in_buf = NULL;
		in_buf_size = 0;
	} else if (backend->peekin_start != NULL) {
		in_buf = NULL;
	}

	if ((flags & XZF_WRITE) == 0) {
		out_buf = NULL;
		out_buf_size = 0;
	} else if (backend->peekout_start != NULL) {
		out_buf = NULL;
	}

	memset(strm, 0, sizeof(*strm));

	strm->in_buf = in_buf;
	strm->in_buf_size = in_buf_size;

	strm->out_buf = out_buf;
	strm->out_buf_size = out_buf_size;

	strm->backend = backend;
	strm->state = state;
// 	mem->strm.flags = flags | XZF_NOMUTEX; // FIXME?
	strm->stream_is_external = true;
	strm->has_mutex = false;

	return strm;
}
