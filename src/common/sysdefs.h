/*
 * Common includes and definitions
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#ifndef XZF_SYSDEFS_H
#define XZF_SYSDEFS_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifdef HAVE_STDINT_H
#	include <stdint.h>
#endif

#include <limits.h>

#if INT_MAX < 2147483647 || UINT_MAX < 4294967295U
#	error "int isn't at least 32 bits as required by POSIX.1-2001."
#endif

#if CHAR_BIT != 8
#	error CHAR_BIT != 8
#endif

// Incorrect(?) SIZE_MAX:
//   - Interix headers typedef size_t to unsigned long,
//     but a few lines later define SIZE_MAX to INT32_MAX.
//   - SCO OpenServer (x86) headers typedef size_t to unsigned int
//     but define SIZE_MAX to INT32_MAX.
//
// FIXME: Do we really care about this in libxzfile?
#if defined(__INTERIX) || defined(_SCO_DS)
#	undef SIZE_MAX
#endif

#ifndef SIZE_MAX
#	if SIZEOF_SIZE_T == 4
#		define SIZE_MAX UINT_MAX
#	elif SIZEOF_SIZE_T == 8
#		if ULONG_MAX == 18446744073709551615UL
#			define SIZE_MAX ULONG_MAX
#		elif ULLONG_MAX == 18446744073709551615ULL
#			define SIZE_MAX ULLONG_MAX
#		else
#			error "Cannot determine the value of SIZE_MAX."
#		endif
#	else
#		error "Cannot determine the value of SIZE_MAX."
#	endif
#endif

#ifndef PTRDIFF_MAX
#	if SIZEOF_PTRDIFF_T == 4
#		define PTRDIFF_MAX INT_MAX
#	elif SIZEOF_PTRDIFF_T == 8
#		if ULONG_MAX == 18446744073709551615UL
#			define PTRDIFF_MAX ULONG_MAX
#		elif ULLONG_MAX == 18446744073709551615ULL
#			define PTRDIFF_MAX ULLONG_MAX
#		else
#			error "Cannot determine the value of PTRDIFF_MAX."
#		endif
#	else
#		error "Cannot determine the value of PTRDIFF_MAX."
#	endif
#endif

#ifdef HAVE_STDBOOL_H
#	include <stdbool.h>
#else
#	if ! HAVE__BOOL
typedef unsigned char _Bool;
#	endif
#	define bool _Bool
#	define false 0
#	define true 1
#	define __bool_true_false_are_defined 1
#endif

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#ifndef ARRAY_SIZE
#	define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

#endif
