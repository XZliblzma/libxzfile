##
## Author: Lasse Collin
##
## This file has been put into the public domain.
## You can do whatever you want with this file.
##

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/libxzfile

LDADD = $(top_builddir)/src/libxzfile/libxzfile.la

check_PROGRAMS = \
	test_read

TESTS = \
	test_read
