/*
 * Backend for file descriptors
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "sysdefs.h"
#include "xzfile.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#ifndef O_BINARY
#	define O_BINARY 0
#endif

#ifndef O_NOCTTY
#	define O_NOCTTY 0
#endif


struct fd_state {
	int fd;
	bool writing;
};


static int
fd_read(void *stateptr, unsigned char *buf, size_t *size)
{
	struct fd_state *state = stateptr;

	while (true) {
		const size_t limit = *size <= SSIZE_MAX ? *size : SSIZE_MAX;
		const ssize_t ret = read(state->fd, buf, limit);

		if (ret <= 0) {
			if (ret < 0 && errno == EINTR)
				continue;

			*size = 0;
			return ret == 0 ? XZF_E_EOF : errno;
		}

		*size = ret;
		return 0;
	}
}


static int
fd_write(void *stateptr, const unsigned char *buf, size_t size)
{
	// TODO: Sparse file support

	struct fd_state *state = stateptr;

	do {
		const size_t limit = size <= SSIZE_MAX
				? size : SSIZE_MAX;
		const ssize_t ret = write(state->fd, buf, limit);

		if (ret < 0)
			return errno;

		// FIXME: ENOSPC like in gnulib if ret == 0?

		buf += ret;
		size -= ret;

	} while (size > 0);

	return 0;
}


static int
fd_flush(void *stateptr, int fl_flags)
{
	struct fd_state *state = stateptr;

	if ((fl_flags & XZF_FL_SYNC) && state->writing)
		while (fsync(state->fd) && errno != EINVAL)
			if (errno != EINTR)
				return errno;

	return 0;
}


static int
fd_seek(void *stateptr, xzf_off *offset, enum xzf_whence whence)
{
	static const int convert[3] = {
		SEEK_SET,
		SEEK_CUR,
		SEEK_END,
	};

	// FIXME: This is broken if large file support isn't available.

	struct fd_state *state = stateptr;
	*offset = lseek(state->fd, *offset, convert[whence]);
	return *offset == -1 ? errno : 0;
}


static int
fd_close(void *stateptr, int cl_flags)
{
	struct fd_state *state = stateptr;

	// XZF_CL_SYNC == XZF_FL_SYNC so we can just call fd_flush().
	const int fsync_errnum = fd_flush(state, cl_flags);

	const int close_ret = cl_flags & XZF_CL_DETACH ? 0 : close(state->fd);
	const int close_errnum = errno;

	free(state);

	if (fsync_errnum != 0)
		return fsync_errnum;

	if (close_ret)
		return close_errnum;

	return 0;
}


static int
fd_getinfo(void *stateptr, int key, void *value)
{
	struct fd_state *state = stateptr;

	switch (key) {
		case XZF_KEY_FD: {
			int *fd = value;
			*fd = state->fd;
			return 0;
		}

		case XZF_KEY_ISATTY: {
			int *result = value;
			*result = isatty(state->fd);
			return 0;
		}
	}

	return XZF_E_NOKEY;
}


static const struct xzf_backend fd_backend = {
	.version = 0,
	.read = &fd_read,
	.write = &fd_write,
	.seek = &fd_seek,
	.flush = &fd_flush,
	.close = &fd_close,
	.getinfo = &fd_getinfo,
};


extern xzf_stream *
xzf_fd_open(const char *filename, int xflags, int mode /* FIXME unsigned? */)
{
	static const int supported_xflags
			= XZF_RW | XZF_APPEND | XZF_CREAT | XZF_TRUNC
			| XZF_EXCL | XZF_NOFOLLOW | XZF_REGFILE;
			// FIXME: THRSAFE, LINEBUF etc. etc.

	if (xflags & ~supported_xflags) {
		errno = EINVAL;
		return NULL;
	}

	// TODO: Validate the flags and convert them to be usable with open().
	int oflags;

	if ((xflags & XZF_RW) == XZF_RW) {
		oflags = O_RDWR;
	} else if (xflags & XZF_READ) {
		oflags = O_RDONLY;
	} else if (xflags & XZF_WRITE) {
		oflags = O_WRONLY;
	} else {
		errno = EINVAL;
		return NULL;
	}

	if ((xflags & XZF_APPEND) != 0) {
		if ((xflags & XZF_WRITE) == 0) {
			// XZF_WRITE bit wasn't included but the append
			// bit was. XZF_APPEND includes both so the caller
			// is doing something wrong.
			errno = EINVAL;
			return NULL;
		}

		oflags |= O_APPEND;
	}

	if (xflags & XZF_CREAT)
		oflags |= O_CREAT;

	if (xflags & XZF_TRUNC)
		oflags |= O_TRUNC;

	if (xflags & XZF_EXCL)
		oflags |= O_EXCL;

	// TODO: Support systems that don't have O_NOFOLLOW.
	if (xflags & XZF_NOFOLLOW)
		oflags |= O_NOFOLLOW;

	// FIXME: Does this work with XZF_WRITE?
	if (xflags & XZF_REGFILE)
		oflags |= O_NONBLOCK;

	// Flags that we always want.
	oflags |= O_NOCTTY | O_BINARY;

	// Allocate memory.
	xzf_stream_mem *strm_mem = xzf_stream_prealloc(
			xflags & XZF_READ ? XZF_BUFSIZE : 0,
			xflags & XZF_WRITE ? XZF_BUFSIZE : 0);
	if (strm_mem == NULL)
		return NULL;

	struct fd_state *state = malloc(sizeof(*state));
	if (state == NULL) {
		xzf_stream_free(strm_mem);
		return NULL;
	}

	state->fd = -1;
	state->writing = (xflags & XZF_WRITE) != 0;

	// FIXME: Use custom error code for O_NOFOLLOW failure.
	state->fd = open(filename, oflags, (mode_t)mode);
	if (state->fd == -1)
		goto error;

	if (xflags & XZF_REGFILE) {
		struct stat st;
		if (fstat(state->fd, &st))
			goto error;

		if (!S_ISREG(st.st_mode)) {
			errno = XZF_E_NOTFILE;
			goto error;
		}

		oflags = fcntl(state->fd, F_GETFL);
		if (oflags == -1)
			goto error;

		oflags &= ~O_NONBLOCK;

		if (fcntl(state->fd, F_SETFL, oflags))
			goto error;
	}

	// FIXME!
	xflags &= XZF_RW;

	if (lseek(state->fd, 0, SEEK_CUR) != -1)
		xflags |= XZF_SEEKABLE | XZF_FIXREADPOS;

	xzf_stream *strm = xzf_stream_init(
			strm_mem, &fd_backend, state, xflags, 0, 0);
	assert(strm != NULL);
	if (strm == NULL) {
		strm_mem = NULL;
		goto error;
	}

	return strm;

error:
	{
		const int saved_errno = errno;

		if (state->fd != -1)
			(void)close(state->fd);

		free(state);
		xzf_stream_free(strm_mem);

		errno = saved_errno;
		return NULL;
	}
}


extern xzf_stream *
xzf_fd_fdopen(int fd, int xflags)
{
	static const int supported_xflags
			= XZF_RW | XZF_LINEBUF | XZF_UNBUF;
			// FIXME: THRSAFE, LINEBUF etc. etc.

	if (xflags & ~supported_xflags) {
		errno = EINVAL;
		return NULL;
	}

	struct fd_state *state = malloc(sizeof(*state));
	if (state == NULL)
		return NULL;

	state->fd = fd;
	state->writing = (xflags & XZF_WRITE) != 0;

	if (lseek(fd, 0, SEEK_CUR) != -1)
		xflags |= XZF_SEEKABLE | XZF_FIXREADPOS;

	xzf_stream *strm = xzf_stream_init(NULL, &fd_backend, state,
			xflags, XZF_BUFSIZE, XZF_BUFSIZE);
	if (strm == NULL) {
		const int saved_errno = errno;
		free(state);
		errno = saved_errno;
		return NULL;
	}

	return strm;
}
