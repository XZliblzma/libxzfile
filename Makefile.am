##
## Author: Lasse Collin
##
## This file has been put into the public domain.
## You can do whatever you want with this file.
##

SUBDIRS = src tests

ACLOCAL_AMFLAGS = -I m4

# This works with GNU tar.
mydist:
	VERSION=$(VERSION); \
	if test -d "$(srcdir)/.git" && type git > /dev/null 2>&1; then \
		SNAPSHOT=`cd "$(srcdir)" && git describe --abbrev=4 | cut -b2-`; \
		test -n "$$SNAPSHOT" && VERSION=$$SNAPSHOT; \
	fi; \
	TAR_OPTIONS='--owner=0 --group=0 --numeric-owner --mode=u+rw,go+r-w' \
		$(MAKE) VERSION="$$VERSION" dist-gzip
