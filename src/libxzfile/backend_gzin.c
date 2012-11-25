/*
 * Backend for reading .gz files
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "sysdefs.h"
#include "xzfile.h"

#include <zlib.h>


struct gzin_state {
	xzf_stream *in;
	bool concatenated;
	bool finished;
	z_stream s;
};


static int
gzin_errno(int zerrnum)
{
	int ret;

	switch (zerrnum) {
	case Z_NEED_DICT:
		ret = XZF_E_ZOPTNOTSUP;
		break;

	case Z_DATA_ERROR:
		ret = XZF_E_ZCORRUPT;
		break;

	case Z_MEM_ERROR:
		ret = ENOMEM;
		break;

	case Z_BUF_ERROR:
		ret = XZF_E_ZTRUNC;
		break;

	default:
		ret = XZF_E_BUG;
		break;
	}

	return ret;
}


static int
gzin_read(void *stateptr, unsigned char *out, size_t *out_size)
{
	struct gzin_state *state = stateptr;
	size_t remaining = *out_size;
	*out_size = 0;

	do {
		// Prepare the input buffer.
		const unsigned char *in;
		size_t in_size = xzf_peekin_start(state->in, &in, 1);
		if (in_size == 0 && (errno != XZF_E_EOF || state->finished))
			return errno;

		if (state->finished) {
			assert(in_size > 0);

			// The previous .gz stream was successfully
			// decompressed. Since we got more input,
			// it has to be due to concatenated .gz streams.
			const int ret = inflateReset(&state->s);
			if (ret != Z_OK) {
				xzf_peekin_end(state->in, 0);
				return gzin_errno(ret);
			}

			state->finished = false;
		}

#if UINT_MAX < SIZE_MAX
		if (in_size > UINT_MAX)
			in_size = UINT_MAX;
#endif

		state->s.next_in = (unsigned char *)in;
		state->s.avail_in = (unsigned int)in_size;

		// Prepare the output buffer.
		size_t out_limit = remaining;

#if UINT_MAX < SIZE_MAX
		if (out_limit > UINT_MAX)
			out_limit = UINT_MAX;
#endif

		state->s.next_out = out;
		state->s.avail_out = (unsigned int)out_limit;

		// Decompress
		const int ret = inflate(&state->s, Z_NO_FLUSH);

		// Update the input buffer position.
		if (in_size > 0)
			xzf_peekin_end(state->in,
					in_size - state->s.avail_in);

		// Update the output buffer position.
		const size_t out_used = out_limit - state->s.avail_out;
		out += out_used;
		*out_size += out_used;
		remaining -= out_used;

		// Handle end of file and errors.
		if (ret != Z_OK) {
			// If it didn't finish, it must be an error.
			if (ret != Z_STREAM_END)
				return gzin_errno(ret);

			// If we aren't decompressing concatenated
			// .gz streams, indicate the end of the file now.
			if (!state->concatenated)
				return XZF_E_EOF;

			// Mark that decompressing a stream was finished.
			// This way we know to reset the decompressor if
			// there is more input or we will know that
			// decompression was successful if there won't be
			// more input.
			state->finished = true;
		}
	} while (remaining > 0);

	return 0;
}


static int
gzin_close(void *stateptr, int cl_flags)
{
	struct gzin_state *state = stateptr;
	xzf_stream *in = state->in;

	(void)inflateEnd(&state->s);
	free(state);

	return cl_flags & XZF_CL_DETACH ? 0 : xzf_close(in, cl_flags);
}


static int
gzin_getinfo(void *stateptr, int key, void *value)
{
	struct gzin_state *state = stateptr;

	switch (key) {
		case XZF_KEY_SUBSTREAM: {
			xzf_stream **strm = value;
			*strm = state->in;
			return 0;
		}

		case XZF_KEY_ZTYPE: {
			int *type = value;
			*type = XZF_Z_GZ;
			return 0;
		}

/*
		case XZF_KEY_ZOFFSET: {
			// TODO
			xzf_off *offset = value;
			*offset = state->s.total_in;
			return 0;
		}

		case XZF_KEY_ZPROGRESS: {
			// TODO: Needs 64-bit?
			struct xzf_zprogress *p = value;
			p->comp = state->s.total_in;
			p->uncomp = state->s.total_out;
			return 0;
		}

		case XZF_KEY_ZMEM: {
			// TODO
			size_t *mem;
			*mem = 200 << 10; // FIXME !!! Check!!!
			return 0;
		}
*/
	}

	return xzf_getinfo(state->in, key, value) ? errno : 0;
}


static const struct xzf_backend gzin_backend = {
	.version = 0,
	.read = &gzin_read,
	.close = &gzin_close,
	.getinfo = &gzin_getinfo,
};


extern xzf_stream *
xzf_gzin_open(xzf_stream *in, int zflags)
{
	// TODO: Handle flags:
	// - Switch to zlib format (or use another function?)
	// - Switch to both .gz and zlib format support
	// - Preset dictionary

	static const int supported_flags = XZF_Z_SINGLE;
	if (zflags & ~supported_flags) {
		errno = EINVAL;
		return NULL;
	}

	struct gzin_state *state = malloc(sizeof(*state));
	if (state == NULL)
		return NULL;

	state->in = in;
	state->concatenated = (zflags & XZF_Z_SINGLE) == 0;
	state->finished = false;

	state->s.next_in = Z_NULL;
	state->s.avail_in = 0;
	state->s.zalloc = Z_NULL;
	state->s.zfree = Z_NULL;
	state->s.opaque = Z_NULL;

	const int ret = inflateInit2(&state->s, 31);
	if (ret != Z_OK) {
		free(state);
		errno = gzin_errno(ret);
		return NULL;
	}

	xzf_stream *strm = xzf_stream_init(NULL, &gzin_backend, state,
			XZF_READ, XZF_BUFSIZE, XZF_BUFSIZE);
	if (strm == NULL) {
		const int saved_errno = errno;
		gzin_close(state, XZF_CL_DETACH);
		errno = saved_errno;
		return NULL;
	}

	return strm;
}
