/*
 * File I/O library with compression and decompression support
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 *
 * libxzfile uses other Free Software and Open Source libraries
 * which are copyrighted. Even though libxzfile itself is in the
 * public domain, you must follow the licenses of the libraries that
 * are used by libxzfile even if your code only uses those libraries
 * indirectly via libxzfile. FIXME?
 */

#ifndef XZF_XZFILE_H
#define XZF_XZFILE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief       xzf_stream is open for reading
 */
#define XZF_READ        0x0001

/**
 * \brief       xzf_stream is open for writing
 */
#define XZF_WRITE       0x0002

/**
 * \brief       xzf_stream is open for reading and writing
 */
#define XZF_RW          (XZF_READ | XZF_WRITE)

/**
 * \brief       xzf_stream supports seeking
 */
#define XZF_SEEKABLE    0x0004

/**
 * \brief       xzf_flush() and xzf_close() will adjust the read position
 */
#define XZF_FIXREADPOS  0x0008

// FIXME
#define XZF_FAILFAST FIXME

/**
 * \brief       Writing to a xzf_stream is line buffered
 *
 * This is the default if the xzf_stream is connected to a terminal.
 */
#define XZF_LINEBUF     0x0020

/**
 * \brief       Writing to a xzf_stream is unbuffered
 *
 * This is the default for xzf_stderr.
 *
 * If you need unbuffered reading, use setinbuf(stream, 1). It makes
 * reading effectively unbuffered as long as xzf_peekc(), xzf_peekchar(),
 * or xzf_peekin_start() isn't used. FIXME OK?
 */
#define XZF_UNBUF       0x0040

/**
 * \brief       xzf_stream is thread-safe
 *
 * Excluding xzf_streams allocated on stack, all xzf_streams have an internal
 * mutex that can be used to ensure that only one thread at a time operates
 * on the xzf_stream. The locking can be done either manually using xzf_lock(),
 * xzf_trylock(), and xzf_unlock(), or automatically by specifying the flag
 * XZF_THRSAFE.
 */
#define XZF_THRSAFE     0x0080

/**
 * \brief       File is opened for writing in append mode
 *
 * FIXME: Is append mode specific to files or is it fine for all streams
 * (emulated if needed)?
 *
 * FIXME? Should XZF_WRITE be removed and be required to be used separately?
 *        Having two bits set may lead to easy bugs in backend code.
 */
#define XZF_APPEND      (0x0100 | XZF_WRITE)

/**
 * \brief       File is created if it doesn't already exist
 *
 * This requires that XZF_WRITE or XZF_APPEND was also used.
 * This maps to O_CREAT in open(2).
 */
#define XZF_CREAT       0x0200

/**
 * \brief       If the file exists, it is truncated to zero bytes
 *
 * This maps to O_TRUNC in open(2).
 */
#define XZF_TRUNC       0x0400

/**
 * \brief       If used together with XZF_CREATE, don't overwrite existing file
 *
 * This maps to O_EXCL in open(2).
 */
#define XZF_EXCL        0x0800

/**
 * \brief       If the filename is a symlink, don't follow it
 *
 * This maps to O_NOFOLLOW in open(2). If the operating system doesn't
 * support O_NOFOLLOW in open(2), it is emulated. The emulation is not
 * free of race conditions.
 */
#define XZF_NOFOLLOW    0x1000


#define XZF_CLOEXEC FIXME


/**
 * \brief       Require that the specified file is a regular file
 *
 * This is implemented by opening the file with O_NONBLOCK and then
 * calling fstat(2) on that file descriptor. If it isn't a regular file,
 * the file is closed and xzf_open() returns NULL. Otherwise fcntl(2) is
 * used to drop the O_NONBLOCK flag and xzf_open() returns successfully.
 *
 * FIXME: Does this work with XZF_WRITE?
 */
#define XZF_REGFILE     0x2000

/**
 * \brief       Try to make the file sparse when writing
 */
#define XZF_SPARSE      0x4000

/**
 * \brief       Enable compression or decompression support
 */
#define XZF_COMP        0x8000

#define XZF_Z_NONE      0x0001
#define XZF_Z_GZ        0x0002
#define XZF_Z_BZ2       0x0004
#define XZF_Z_XZ        0x0008
#define XZF_Z_LZO       0x0010
#define XZF_Z_ANY       0x0FFF
#define XZF_Z_CUSTOM    0x1000

/**
 * \brief       When decompressing (XZF_READ), stop after the first stream
 *
 * Many stream compression formats allow concatenating multiple compressed
 * files as is, and the tools decompress such files as if they were a single
 * file. This library does so by default too, but if the compressed data
 * is stored inside some other file format, this isn't necessarly a good
 * thing to do. This flag makes the decompressor indicate end of file after
 * the first compressed stream has been decompressed.
 */
#define XZF_Z_SINGLE    0x2000


/* Flush flags */

#define XZF_FL_SYNC     0x10

/* Z_FULL_FLUSH, BZ_FLUSH, LZMA_FULL_FLUSH */
#define XZF_FL_BLOCK FIXME

/* Z_FINISH, BZ_FINISH, LZMA_FINISH */
#define XZF_FL_FINISH FIXME


/* Close flags */

#define XZF_CL_DETACH   0x01
#define XZF_CL_FORGET   0x02
#define XZF_CL_SYNC     0x10


/* Keys */

#define XZF_KEY_FD            (-1)
#define XZF_KEY_ISATTY        (-2)
#define XZF_KEY_SUBSTREAM     (-3)
#define XZF_KEY_ZTYPE         (-4)
#define XZF_KEY_ZOFFSET       (-5)
#define XZF_KEY_ZPROGRESS     (-6)
#define XZF_KEY_ZMEM          (-7)
#define XZF_KEY_NAME            1


#define XZF_BUFSIZE 8192

/*
 * errno values
 * TODO
 */
#define XZF_E_EOF (-1)
#define XZF_E_NOTFILE (-2)
#define XZF_E_NOTREADABLE (-3)
#define XZF_E_NOTWRITABLE (-4)
#define XZF_E_ZOPTNOTSUP (-5)
#define XZF_E_ZTRUNC (-6)
#define XZF_E_ZCORRUPT (-7)
#define XZF_E_BUG (-8)
#define XZF_E_CALLBACK (-9)
#define XZF_E_NOKEY (-10)


#ifndef XZF_INTERNAL_H
/*
 * This is defined here to let the macros access the internal buffers.
 * This is not the full struct. Never allocate this structure yourself.
 */
typedef struct xzf_stream {
	const unsigned char *in_next;
	const unsigned char *in_stop;
	const unsigned char *in_end;

	unsigned char *out_next;
	const unsigned char *out_stop;
	const unsigned char *out_end;
} xzf_stream;

#endif


/**
 * \brief       Offset in a file (compare to off_t in POSIX)
 *
 * This library always uses 63-bit (or bigger) file offsets, so
 * there is no mess with large file support.
 */
typedef long long xzf_off;
typedef unsigned long long xzf_u_off; // FIXME? In case someone needs unsigned?

// FIXME?
#define XZF_PRIiOFF "lli"
#define XZF_PRIdOFF "lld"
#define XZF_PRIoOFF "llo"
#define XZF_PRIxOFF "llx"
#define XZF_PRIXOFF "llX"


enum xzf_whence {
	XZF_SEEK_SET = 0,
	XZF_SEEK_CUR = 1,
	XZF_SEEK_END = 2
// FIXME? Also XZF_SEEK_HOLE and XZF_SEEK_DATA for sparse support?
// Solaris has those and on Linux they can nowadays be emulated with fiemap.
// FIXME? Also XZF_SEEK_BLOCK to locate .xz block boundaries?
};


/* Backend */

struct xzf_backend {
	unsigned int version;

	// FIXME: Check all backends: Should the return value be -1 or errno
	// when something goes wrong?

	/* NOTE: read() and write() are never called with *size == 0. */
	/* NOTE: read() may read less than requested, write must
	   write everything? */
	// FIXME: Should write take size as a pointer for future compatibility?
	int (*read)(void *state, unsigned char *buf, size_t *size);
	int (*write)(void *state, const unsigned char *buf, size_t size);
	int (*seek)(void *state, xzf_off *offset, enum xzf_whence whence);
	int (*flush)(void *state, int fl_flags);
	int (*close)(void *state, int cl_flags);

	int (*peekin_start)(void *state,
			const unsigned char **buf, size_t *size);
	int (*peekin_end)(void *state, size_t bytes_used);
	int (*peekout_start)(void *state, unsigned char **buf, size_t *size);
	int (*peekout_end)(void *state, size_t bytes_written);

	int (*getinfo)(void *state, int key, void *value);

	// FIXME TODO? int (*setflags)(void *state, int flags);

	int (*reserved[21])(void *);
};

typedef struct xzf_stream_mem xzf_stream_mem;

extern xzf_stream_mem *xzf_stream_prealloc(
		size_t in_buf_size, size_t out_buf_size);

extern void xzf_stream_free(xzf_stream_mem *mem);

extern xzf_stream *xzf_stream_init(xzf_stream_mem *mem,
		const struct xzf_backend *backend, void *state, int flags,
		size_t in_buf_size, size_t out_buf_size);

/* For allocation on stack */
typedef union xzf_stream_mem_st { // FIXME name?
	xzf_off xzf_stackmem_off;
	size_t xzf_stackmem_size[64];
	void *xzf_stackmem_ptr[64]; // FIXME?
} xzf_stream_mem_st;

extern xzf_stream *xzf_stream_init_st(xzf_stream_mem_st *mem,
		const struct xzf_backend *backend, void *state, int flags,
		unsigned char *in_buf, size_t in_buf_size,
		unsigned char *out_buf, size_t out_buf_size);



typedef struct xzf_errbuf {
	char buf[512];
} xzf_errbuf;


extern xzf_stream *xzf_stdin;
extern xzf_stream *xzf_stdout;
extern xzf_stream *xzf_stderr;


extern int xzf_stdio_open(void);
extern int xzf_stdio_close(void);

extern xzf_stream *xzf_open(const char *filename, int flags, ...);
extern xzf_stream *xzf_xzfopen(xzf_stream *stream, int flags, ...);
// extern xzf_stream *xzf_fopen(FILE *file_stream, int flags, ...);
extern xzf_stream *xzf_fdopen(int fd, int flags, ...);
extern xzf_stream *xzf_copen(const struct xzf_backend *backend, void *state,
		int flags, size_t in_bufsize, size_t out_bufsize);

extern void xzf_swap(xzf_stream *stream1, xzf_stream *stream2);


extern size_t xzf_read(xzf_stream *stream, void *buf, size_t size);
extern int xzf_write(xzf_stream *stream, const void *buf, size_t size);

extern xzf_off xzf_seek(xzf_stream *stream, xzf_off offset,
		enum xzf_whence whence);
// extern xzf_off xzf_getpos(xzf_stream *stream);

extern int xzf_flush(xzf_stream *stream, int fl_flags);
extern int xzf_close(xzf_stream *stream, int cl_flags);

extern int xzf_eof(xzf_stream *stream);
extern int xzf_geterr(xzf_stream *stream);
extern int xzf_seterr(xzf_stream *stream, int errnum);
extern const char *xzf_strerr(int errnum, xzf_errbuf *buf);

extern int xzf_getinfo(xzf_stream *stream, int key, void *value);

extern int xzf_getflags(xzf_stream *stream);
extern int xzf_setflags(xzf_stream *stream, int flags);

extern int xzf_getchar(xzf_stream *stream);
extern int xzf_peekchar(xzf_stream *stream);
extern int xzf_putchar(xzf_stream *stream, unsigned char c);

// FIXME !!! Unlocked versions are broken for unbuffered
// and line-buffered streams.

#define xzf_getc(stream) \
	((stream)->in_next < (stream)->in_stop \
			? *(stream)->in_next++ \
			: xzf_getchar(stream))

#define xzf_getc_ul(stream) \
	((stream)->in_next < (stream)->in_end \
			? *(stream)->in_next++ \
			: xzf_getchar(stream))

#define xzf_peekc(stream) \
	((stream)->in_next < (stream)->in_stop \
			? *(stream)->in_next \
			: xzf_peekchar(stream))

#define xzf_peekc_ul(stream) \
	((stream)->in_next < (stream)->in_end \
			? *(stream)->in_next \
			: xzf_peekchar(stream))

#define xzf_putc(stream, c) \
	((stream)->out_next < (stream)->out_stop \
			? *(stream)->out_next++ = (unsigned char)(c) \
			: xzf_putchar(stream, c))

#define xzf_putc_ul(stream, c) \
	((stream)->out_next < (stream)->out_end \
			? *(stream)->out_next++ = (unsigned char)(c) \
			: xzf_putchar(stream, c))


/**
 * FIXME desc
 *
 * This may return less or more than the requested size. Returning more
 * is useful if the application doesn't strictly need more than a byte
 * or a few but it can use more to be more efficient. By requesting only
 * one byte, the caller can be sure that this library doesn't need to
 * do any memmove() calls or similar things to provide long enough
 * contiguous buffer to the application.
 */
extern size_t xzf_peekin_start(
		xzf_stream *stream, const unsigned char **buf, size_t size);

extern void xzf_peekin_end(xzf_stream *stream, size_t bytes_used);

extern size_t xzf_peekout_start(
		xzf_stream *stream, unsigned char **buf, size_t size);
extern int xzf_peekout_end(xzf_stream *stream, size_t bytes_written);

extern int xzf_puts(xzf_stream *stream, const char *str);

extern void xzf_lock(xzf_stream *stream);
extern int xzf_trylock(xzf_stream *stream);
extern void xzf_unlock(xzf_stream *stream);

extern int xzf_fileno(xzf_stream *stream);

extern int xzf_setinbuf(xzf_stream *stream, size_t size);
extern size_t xzf_getinbuf(xzf_stream *stream);

extern int xzf_setoutbuf(xzf_stream *stream, size_t size);
extern size_t xzf_getoutbuf(xzf_stream *stream);

extern size_t xzf_pending(xzf_stream *stream);
extern void xzf_purge(xzf_stream *stream, int flags);

/*
TODO:
gzoffset()
gzdirect()
getline
*/

extern xzf_stream *xzf_dummy_open(void);

extern xzf_stream *xzf_fd_open(const char *filename, int flags, int mode);
extern xzf_stream *xzf_fd_fdopen(int fd, int xflags);

// FIXME: Needs only XZF_C_SINGLE or similar as a flag?
extern xzf_stream *xzf_gzin_open(xzf_stream *stream, int flags);

// FIXME? Use a struct instead to pass the params?
extern xzf_stream *xzf_gzout_open(xzf_stream *stream, int level, int strategy);


extern xzf_stream *xzf_cb_open(xzf_stream *stream,
		void (*in_cb)(void *in_state, const unsigned char *buf,
			size_t size),
		void *in_state,
		void (*out_cb)(void *in_state, const unsigned char *buf,
			size_t size),
		void *out_state);

extern xzf_stream *xzf_cb_inopen(xzf_stream *stream,
		void (*in_cb)(void *in_state, const unsigned char *buf,
			size_t size),
		void *in_state);

extern xzf_stream *xzf_cb_outopen(xzf_stream *stream,
		void (*out_cb)(void *out_state, const unsigned char *buf,
			size_t size),
		void *out_state);


extern void xzf_cb_crc32(void *crc, const unsigned char *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
