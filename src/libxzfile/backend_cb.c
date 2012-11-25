/*
 * Backend to call a callback on the data read or written
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "sysdefs.h"
#include "xzfile.h"


struct cb_state {
	xzf_stream *strm;

	void (*in_cb)(void *in_state, const unsigned char *buf, size_t size);
	void *in_state;
	const unsigned char *in_buf;

	void (*out_cb)(void *out_state, const unsigned char *buf, size_t size);
	void *out_state;
	const unsigned char *out_buf;
};


static int
cb_read(void *stateptr, unsigned char *buf, size_t *size)
{
	struct cb_state *state = stateptr;

	const size_t new_size = xzf_read(state->strm, buf, *size);
	const int ret = new_size < *size ? errno : 0;
	*size = new_size;

	if (state->in_cb != NULL)
		state->in_cb(state->in_state, buf, new_size);

	return ret;
}


static int
cb_peekin_start(void *stateptr, const unsigned char **buf, size_t *size)
{
	struct cb_state *state = stateptr;
	const size_t n = xzf_peekin_start(state->strm, buf, *size);
	state->in_buf = *buf;
	const int ret = n >= *size ? 0 : errno;
	*size = n;
	return ret;
}


static int
cb_peekin_end(void *stateptr, size_t bytes_used)
{
	struct cb_state *state = stateptr;
	if (state->in_cb != NULL)
		state->in_cb(state->in_state, state->in_buf, bytes_used);

	xzf_peekin_end(state->strm, bytes_used);
	return 0;
}


static int
cb_write(void *stateptr, const unsigned char *buf, size_t size)
{
	struct cb_state *state = stateptr;
	if (state->out_cb != NULL)
		state->out_cb(state->out_state, buf, size);

	return xzf_write(state->strm, buf, size) ? errno : 0;
}


static int
cb_peekout_start(void *stateptr, unsigned char **buf, size_t *size)
{
	struct cb_state *state = stateptr;
	const size_t n = xzf_peekout_start(state->strm, buf, *size);
	state->out_buf = *buf;
	const int ret = n >= *size ? 0 : errno;
	*size = n;
	return ret;
}


static int
cb_peekout_end(void *stateptr, size_t bytes_written)
{
	struct cb_state *state = stateptr;
	if (state->out_cb != NULL)
		state->out_cb(state->out_state, state->out_buf, bytes_written);

	return xzf_peekout_end(state->strm, bytes_written) ? errno : 0;
}

/*
static xzf_off
cb_seek(void *stateptr, xzf_off offset, enum xzf_whence whence)
{
	struct cb_state *state = stateptr;
	return xzf_seek(state->strm, offset, whence);
}
*/


static int
cb_flush(void *stateptr, int fl_flags)
{
	struct cb_state *state = stateptr;
	return xzf_flush(state->strm, fl_flags);
}


static int
cb_close(void *stateptr, int cl_flags)
{
	struct cb_state *state = stateptr;
	xzf_stream *strm = state->strm;
	free(state);
	return cl_flags & XZF_CL_DETACH ? 0 : xzf_close(strm, cl_flags);
}


static int
cb_getinfo(void *stateptr, int key, void *value)
{
	struct cb_state *state = stateptr;

/*
	FIXME Useless?
	if (key == XZF_KEY_TYPE) {
		int *i = value;
		*i = XZF_TYPE_CALLBACK;
		return 0;
	}
*/

	return xzf_getinfo(state->strm, key, value) ? errno : 0;
}


// FIXME
static const struct xzf_backend cb_backend = {
	.version = 0,
	.read = &cb_read,
	.peekin_start = &cb_peekin_start,
	.peekin_end = &cb_peekin_end,
	.write = &cb_write,
	.peekout_start = &cb_peekout_start,
	.peekout_end = &cb_peekout_end,
// 	.seek = &cb_seek,
	.flush = &cb_flush,
	.close = &cb_close,
	.getinfo = &cb_getinfo,
};


extern xzf_stream *
xzf_cb_inopen(xzf_stream *sub_strm,
		void (*in_cb)(void *in_state,
			const unsigned char *buf, size_t size),
		void *in_state)
{
	return xzf_cb_open(sub_strm, in_cb, in_state, NULL, NULL);
}


extern xzf_stream *
xzf_cb_outopen(xzf_stream *sub_strm,
		void (*out_cb)(void *out_state,
			const unsigned char *buf, size_t size),
		void *out_state)
{
	return xzf_cb_open(sub_strm, NULL, NULL, out_cb, out_state);
}


extern xzf_stream *
xzf_cb_open(xzf_stream *sub_strm,
		void (*in_cb)(void *in_state,
			const unsigned char *buf, size_t size),
		void *in_state,
		void (*out_cb)(void *in_state,
			const unsigned char *buf, size_t size),
		void *out_state)
{
	struct cb_state *state = malloc(sizeof(*state));
	if (state == NULL)
		return NULL;

	state->strm = sub_strm;
	state->in_cb = in_cb;
	state->in_state = in_state;
	state->out_cb = out_cb;
	state->out_state = out_state;

	const int flags = xzf_getflags(sub_strm);
	const size_t in_bufsize = (flags & XZF_READ)
			? xzf_getinbuf(sub_strm) : 0;
	const size_t out_bufsize = (flags & XZF_WRITE)
			? xzf_getoutbuf(sub_strm) : 0;

	xzf_stream *strm = xzf_stream_init(NULL, &cb_backend, state, flags,
			in_bufsize, out_bufsize);
	if (strm == NULL) {
		const int saved_errno = errno;
		free(state);
		errno = saved_errno;
		return NULL;
	}

	return strm;
}
