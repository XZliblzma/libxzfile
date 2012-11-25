##
## Author: Lasse Collin
##
## This file has been put into the public domain.
## You can do whatever you want with this file.
##

lib_LTLIBRARIES = libxzfile.la
libxzfile_la_SOURCES = \
	xzfile.h \
	internal.h \
	internal_fill.c \
	internal_fixreadpos.c \
	internal_flush.c \
	internal_seek.c \
	xzf_close.c \
	xzf_eof.c \
	xzf_fileno.c \
	xzf_flush.c \
	xzf_getchar.c \
	xzf_geterr.c \
	xzf_getflags.c \
	xzf_getinbuf.c \
	xzf_getinfo.c \
	xzf_getoutbuf.c \
	xzf_lock.c \
	xzf_peekchar.c \
	xzf_peekin.c \
	xzf_peekout.c \
	xzf_pending.c \
	xzf_purge.c \
	xzf_putchar.c \
	xzf_puts.c \
	xzf_read.c \
	xzf_seek.c \
	xzf_seterr.c \
	xzf_setflags.c \
	xzf_setinbuf.c \
	xzf_setoutbuf.c \
	xzf_stdio.c \
	xzf_stream.c \
	xzf_swap.c \
	xzf_write.c \
	backend_cb.c \
	backend_dummy.c \
	backend_fd.c \
	backend_gzin.c \
	callback_crc32.c
libxzfile_la_CPPFLAGS = -I$(top_srcdir)/src/common
libxzfile_la_LDFLAGS = -no-undefined -version-info 0:0:0
libxzfile_la_LIBADD = -lz -llzma

# TODO: pkg-config
