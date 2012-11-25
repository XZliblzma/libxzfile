/*
 * Common internal definitions for xzf_* functions
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#ifndef XZF_INTERNAL_H
#define XZF_INTERNAL_H

#include "sysdefs.h"

#include <stdarg.h>

#ifdef HAVE_PTHREAD
#	include <pthread.h>
#endif

typedef struct xzf_stream xzf_stream;

#include "xzfile.h"


// FIXME !!!
#ifndef static_assert
#	ifdef NDEBUG
#		define static_assert(expr, msg) do { } while (0)
#	else
#		define static_assert(expr, msg) \
			do { \
				struct static_assert_struct_ { \
					int static_assertion_failed : \
							((expr) ? 1 : -1); \
				}; \
			} while (0)
#	endif
#endif


#define XZF_OFF_MAX LLONG_MAX

#define XZF_BUF_MAX PTRDIFF_MAX
#if XZF_BUF_MAX > XZF_OFF_MAX
#	undef XZF_BUF_MAX
#	define XZF_BUF_MAX XZF_OFF_MAX
#endif
#if XZF_BUF_MAX > SIZE_MAX
#	undef XZF_BUF_MAX
#	define XZF_BUF_MAX SIZE_MAX
#endif

/*

FIXME Useful for xzfile.h?

#define XZF_OFF_MAX \
	((((((xzf_off)1) << (sizeof(xzf_off) * 8 - 2)) - 1) << 1) + 1)

#define XZF_BUF_MAX (sizeof(size_t) < sizeof(ptrdiff_t) ? (size_t)-1 \
	: ((((((size_t)1) << (sizeof(ptrdiff_t) * 8 - 2)) - 1) << 1) + 1))

*/


/*
#define XZF_CAT_X(a, b) a ## b
#define XZF_CAT(a, b) XZF_CAT_X(a, b)
#define XZF_SYM(name) XZF_CAT(XZF_SYMPREFIX, name)

#ifndef XZF_SYMPREFIX
#	define XZF_SYMPREFIX
#endif

#ifndef XZF_IMPORT
#	define XZF_DECLSPEC
#endif

#ifndef XZF_CALL
#	define XZF_CALL
#endif

#ifndef XZF_VISIBILITY
#	define XZF_VISIBILITY(visibility)
#endif
// #define XZF_VISIBILITY(visibility) __attribute__((__visibility__(visibility)))

#define XZF_PUBLIC(type, name) \
	XZF_DECLSPEC type \
	XZF_VISIBILITY("default") XZF_CALL XZF_SYM(name)

// FIXME: What about private variables?
#define XZF_PRIVATE(type, name) \
	type XZF_VISIBILITY("hidden") XZF_CALL XZF_SYM(XZF_SYMPREFIX, name)
*/


struct xzf_stream {
	const unsigned char *in_next;
	const unsigned char *in_stop;
	const unsigned char *in_end;

	unsigned char *out_next;
	const unsigned char *out_stop;
	const unsigned char *out_end;

	unsigned char *in_buf;
	size_t in_buf_size;

	unsigned char *out_buf;
	size_t out_buf_size;

	const struct xzf_backend *backend;
	void *state;

	int flags;
	int errnum;

	unsigned int is_reading : 1;
	unsigned int is_writing : 1;
	unsigned int frontend_peekin : 1;
	unsigned int frontend_peekout : 1;
	unsigned int backend_peekin : 1;
	unsigned int backend_peekout : 1;
	unsigned int stream_is_external : 1;
	unsigned int eof : 1;
	unsigned int has_mutex : 1;

#ifdef HAVE_PTHREAD
	// NOTE: This must be the last member in the structure
	// to keep xzf_swap() working.
	pthread_mutex_t mutex;
#endif
};

// FIXME? Remove?
// #define SUPPORTED_FLAGS (XZF_READ | XZF_WRITE)


extern int xzf_internal_fill(xzf_stream *stream, size_t min_fill);
extern int xzf_internal_flush(xzf_stream *stream, size_t min_size);
// extern int xzf_internal_close(xzf_stream *stream, bool detach_only);
// extern int xzf_internal_halfclose(xzf_stream *stream, int cl_flags);
extern int xzf_internal_fixreadpos(xzf_stream *strm);
extern xzf_off xzf_internal_seek(
		xzf_stream *strm, xzf_off offset, enum xzf_whence whence);


static inline int
internal_getc(xzf_stream *strm)
{
	assert(!strm->frontend_peekin);

	// FIXME? Return -2 on EOF?

	int ret;

	if (strm->in_next >= strm->in_end && xzf_internal_fill(strm, 1)) {
		ret = -1;
	} else {
		assert(strm->in_next < strm->in_end);
		ret = *strm->in_next++;
	}

	return ret;
}


static inline int
internal_peekc(xzf_stream *strm)
{
	assert(!strm->frontend_peekin);

	// FIXME? Return -2 on EOF?

	int ret;

	if (strm->in_next >= strm->in_end && xzf_internal_fill(strm, 1)) {
		ret = -1;
	} else {
		assert(strm->in_next < strm->in_end);
		ret = *strm->in_next;
	}

	return ret;
}


#ifdef HAVE_PTHREAD
// NOTE: These functions must not modify errno.
static inline void
internal_lock(xzf_stream *strm)
{
	if (strm->flags & XZF_THRSAFE)
		pthread_mutex_lock(&strm->mutex);
}

static inline void
internal_unlock(xzf_stream *strm)
{
	if (strm->flags & XZF_THRSAFE)
		pthread_mutex_unlock(&strm->mutex);
}
#else
#	define internal_lock(strm) do { } while (0)
#	define internal_unlock(strm) do { } while (0)
#endif

#endif
