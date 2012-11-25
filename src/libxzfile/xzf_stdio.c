/*
 * xzf_stdio_open() and xzf_stdio_close()
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "internal.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



/**
 * \brief       Call fflush(NULL) before closing stdio streams
 */
#define XZF_STDIO_FFLUSH        0x01

/**
 * \brief       Handle errno == EPIPE as if no error has occurred
 *
 * This affects xzf_stdout and xzf_stderr.
 */
#define XZF_STDIO_IGNPIPE       0x02

/**
 * \brief       Force xzf_stdout to line buffered mode
 */
#define XZF_STDIO_LINEBUF       0x20

/**
 * \brief       Force xzf_stdout to unbuffered mode
 */
#define XZF_STDIO_UNBUF         0x40


/*
// FIXME
#define XZF_STDIO_INIT \
	{ \
		.stream_is_external = true, \
		.mutex = PTHREAD_MUTEX_INITIALIZER, \
	}

static xzf_stream stdin_stream = XZF_STDIO_INIT;
static xzf_stream stdout_stream = XZF_STDIO_INIT;
static xzf_stream stderr_stream = XZF_STDIO_INIT;

xzf_stream *xzf_stdin = &stdin_stream;
xzf_stream *xzf_stdout = &stdout_stream;
xzf_stream *xzf_stderr = &stderr_stream;
*/


xzf_stream *xzf_stdin;
xzf_stream *xzf_stdout;
xzf_stream *xzf_stderr;


static int flags = 0;
static int err_status = EXIT_FAILURE;


/*
static int
close_c_stdio(void)
{
	int ret = 0;

	// Flush stdout. If something goes wrong, print an error message
	// to xzf_stderr.
	const int ferror_err = ferror(stdout);
	const int fflush_err = fflush(stdout);

	if (ferror_err || fflush_err) {
		ret = -1;

		if (show_error) {
			// If it was fflush() that failed, we have the reason
			// in errno. If only ferror() indicated an error,
			// we have no idea what the reason was.
			xzf_errmsg buf;
			xzf_perror(_("Writing to standard output failed: %s"),
					fflush_err ? xzf_strerr(errno, &buf)
						: _("Unknown error"));
		}
	}



	return ret;
}
*/


static void
xzf_stdio_exit(void)
{
	// FIXME TODO: Optionally close also the C stdio streams before
	// closing the xzfile stdio streams.
	//
	// FIXME: What if stdin would fix the read position and then we
	// try to fix it again for xzf_stdin?
// 	if (flags & XZF_STDIO_FFLUSH)
// 		(void)fflush(NULL);

	// FIXME: Keep things working also if the streams have already
	// been closed.

	// FIXME TODO: Static mutex to prevent running this multiple times?


	// FIXME TODO
	xzf_lock(xzf_stdin);
	const int ret_stdin = xzf_close(xzf_stdin, 0);

	xzf_lock(xzf_stdout);
	int ret_stdout = xzf_close(xzf_stdout, 0);
	if (ret_stdout == EPIPE && (flags & XZF_STDIO_IGNPIPE))
		ret_stdout = 0;

	xzf_lock(xzf_stderr);
	int ret_stderr = xzf_close(xzf_stderr, 0);
	if (ret_stderr == EPIPE && (flags & XZF_STDIO_IGNPIPE))
		ret_stderr = 0;

	if (err_status != 0)
		if (ret_stdin || ret_stdout || ret_stderr)
			_Exit(err_status);
}


static int
open_std_stream(xzf_stream **strm, int fd, int xflags)
{
	const int fdflags = fcntl(fd, F_GETFL);

	if (fdflags != -1) {
		// The stream will be created with the same access mode
		// that the file descriptor has.
		if ((fdflags & O_ACCMODE) == O_RDONLY)
			xflags |= XZF_READ;
		else if ((fdflags & O_ACCMODE) == O_WRONLY)
			xflags |= XZF_WRITE;
		else if ((fdflags & O_ACCMODE) == O_RDWR)
			xflags |= XZF_RW;

	} else if (errno == EBADF) {
		// The file descriptor isn't open. Open it to prevent later
		// open() calls from using the file descriptor numbers of
		// the stdio streams.
		//
		// With stdin, we could use /dev/full so that
		// writing to stdin would fail. However, /dev/full
		// is Linux specific, and if the program tries to
		// write to stdin, there's already a problem anyway.
		//
		// FIXME: Set FD_CLOEXEC with fcntl() or use
		// POSIX.1-2008 O_CLOEXEC to avoid a syscall?
		const int new_fd = open("/dev/null", O_NOCTTY
				| (fd == 0 ? O_WRONLY : O_RDONLY));

		if (new_fd != fd) {
			// Something went wrong. Exit with the
			// exit status we were given. Don't try
			// to print an error message, since stderr
			// may very well be non-existent. This
			// error should be extremely rare.
			//
			// TODO FIXME: Use a flag
			(void)close(new_fd);
			exit(err_status);
		}
	} else {
		// TODO?
	}

	*strm = xflags & XZF_RW
			? xzf_fd_fdopen(fd, xflags) : xzf_dummy_open();
	if (*strm == NULL)
		return -1;

	return 0;
}


extern int
xzf_stdio_open(void /*int flags*/)
{
/*
	// Ensure that the file descriptors 0, 1, and 2 are open.
	for (int i = 0; i <= 2; ++i) {
		// We use fcntl() to check if the file descriptor is open.
		if (fcntl(i, F_GETFD) == -1 && errno == EBADF) {
			// With stdin, we could use /dev/full so that
			// writing to stdin would fail. However, /dev/full
			// is Linux specific, and if the program tries to
			// write to stdin, there's already a problem anyway.
			const int fd = open("/dev/null", O_NOCTTY
					| (i == 0 ? O_WRONLY : O_RDONLY));

			if (fd != i) {
				// Something went wrong. Exit with the
				// exit status we were given. Don't try
				// to print an error message, since stderr
				// may very well be non-existent. This
				// error should be extremely rare.
				//
				// TODO FIXME: Use a flag
				(void)close(fd);
				exit(err_status);
			}
		}
	}

	// Connect the file descriptors 0, 1, and 2 to xzf_stdin,
	// xzf_stdout, and xzf_stderr.

	// FIXME? POSIX says that stderr is open for reading *and* writing. Why?


	xzf_stdin = xzf_fd_fdopen(0, XZF_READ);
	xzf_stdout = xzf_fd_fdopen(1,
			XZF_WRITE | (isatty(1) ? XZF_LINEBUF : 0));
	xzf_stderr = xzf_fd_fdopen(2, XZF_WRITE | XZF_UNBUF);
*/


	open_std_stream(&xzf_stdin, 0, 0);
	open_std_stream(&xzf_stdout, 1, isatty(1) ? XZF_LINEBUF : 0);
	open_std_stream(&xzf_stderr, 2, XZF_UNBUF);


	if (atexit(&xzf_stdio_exit)) {
		// FIXME TODO
	}

	return 0;
}


/*
static int
half_close(xzf_stream *strm)
{
	// Lock the mutex and close the stream as usual but don't unlock
	// the mutex or free the base structure, which contains the mutex.
	// This way there is no race condition when exit() is called in
	// a multithreaded program whose other threads use the stdio streams.
	xzf_lock(strm);

	// TODO: Close it.


}


extern void
xzf_stdio_close(void)
{
	const int ret_stdin = xzf_close(xzf_stdin);
	const int ret_stdout = xzf_close(xzf_stdout);

	if (ret_stdin != 0) {
		TODO
	}

	if (ret_stdout != 0) {
		TODO
	}

	const int ret_stderr = xzf_close(xzf_stderr);

	if (ret_stdin || ret_stdout || ret_stderr)
		if (err_status != 0)
			_Exit(err_status);

	// TODO: Support both _Exit() and _exit() to exit on error?
}
*/
