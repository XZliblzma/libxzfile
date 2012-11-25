##
## Author: Lasse Collin
##
## This file has been put into the public domain.
## You can do whatever you want with this file.
##

bin_PROGRAMS = xzfcat

xzfcat_SOURCES = xzfcat.c
xzfcat_CPPFLAGS = \
	-I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/libxzfile
xzfcat_LDADD = $(top_builddir)/src/libxzfile/libxzfile.la
