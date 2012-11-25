#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

###############################################################################
#
# Author: Lasse Collin
#
# This file has been put into the public domain.
# You can do whatever you want with this file.
#
###############################################################################

AC_PREREQ([2.64])

dnl FIXME: Add the URL argument.
AC_INIT([libxzfile], [0.0.1], [lasse.collin@tukaani.org], [libxzfile])
AC_CONFIG_SRCDIR([src/libxzfile/xzfile.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([1.10 foreign tar-v7 filename-length-max=99])
AM_CFLAGS=

AC_PROG_CC_C99
if test x$ac_cv_prog_cc_c99 = xno ; then
	AC_MSG_ERROR([No C99 compiler was found.])
fi

AC_USE_SYSTEM_EXTENSIONS

dnl TODO: ax_pthreads.m4

LT_PREREQ([2.2])
LT_INIT

dnl TODO FIXME
dnl AM_GNU_GETTEXT_VERSION([0.18])
dnl AM_GNU_GETTEXT([external])

AC_HEADER_STDBOOL
AC_CHECK_SIZEOF([size_t])
AC_SYS_LARGEFILE

AC_FUNC_STRERROR_R

AC_MSG_CHECKING([if debugging code should be compiled])
AC_ARG_ENABLE([debug], AC_HELP_STRING([--enable-debug], [Enable debugging code.]),
	[], enable_debug=no)
if test "x$enable_debug" = xyes; then
	AC_MSG_RESULT([yes])
else
	AC_DEFINE([NDEBUG], [1], [Define to 1 to disable debugging code.])
	AC_MSG_RESULT([no])
fi

if test "$GCC" = yes ; then
	# Enable warnings.
	for NEW_FLAG in \
			-Wall \
			-Wextra \
			-Wformat=2 \
			-Winit-self \
			-Wmissing-include-dirs \
			-Wstrict-aliasing \
			-Wfloat-equal \
			-Wundef \
			-Wshadow \
			-Wpointer-arith \
			-Wbad-function-cast \
			-Wwrite-strings \
			-Wlogical-op \
			-Waggregate-return \
			-Wstrict-prototypes \
			-Wold-style-definition \
			-Wmissing-prototypes \
			-Wmissing-declarations \
			-Wmissing-noreturn \
			-Wredundant-decls \
			-Wcast-align
	do
		AC_MSG_CHECKING([if $CC accepts $NEW_FLAG])
		OLD_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $NEW_FLAG"
		AC_COMPILE_IFELSE([AC_LANG_SOURCE([void foo(void) { }])], [
			AM_CFLAGS="$AM_CFLAGS $NEW_FLAG"
			AC_MSG_RESULT([yes])
		], [
			AC_MSG_RESULT([no])
		])
		CFLAGS="$OLD_CFLAGS"
	done

	AC_ARG_ENABLE([werror],
		AC_HELP_STRING([--enable-werror], [Enable -Werror to abort
			compilation on all compiler warnings.]),
		[], [enable_werror=no])
	if test "x$enable_werror" = "xyes"; then
		AM_CFLAGS="$AM_CFLAGS -Werror"
	fi
fi

AC_SUBST([AM_CFLAGS])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/libxzfile/Makefile
	src/xzfcat/Makefile
	tests/Makefile
])

AC_OUTPUT
