/*
 * xzfcat
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#include "sysdefs.h"
#include "xzfile.h"


// FIXME DEBUG
#include <stdio.h>
#include <unistd.h>


static void
cat_file(xzf_stream *file)
{
/*
	// Test xzf_getc().
	int c;
	while ((c = xzf_getc(file)) >= 0)
		putchar(c);
*/

/*
	// Test xzf_read() with small buffers.
	while (true) {
		char buf[5];
		const size_t size = xzf_read(file, buf, sizeof(buf));
		if (size == 0)
			break;

		fwrite(buf, 1, size, stdout);
	}
*/

/*
	// Test xzf_read() with big buffers.
	while (true) {
		char buf[XZF_BUFSIZE];
		const size_t size = xzf_read(file, buf, sizeof(buf));
		if (size == 0)
			break;

		fwrite(buf, 1, size, stdout);
	}
*/

/*
	// Test xzf_read() with mixed buffer sizes.
	while (true) {
		char buf[XZF_BUFSIZE];
		size_t size = xzf_read(file, buf, sizeof(buf));
		if (size == 0)
			break;

		fwrite(buf, 1, size, stdout);

		size = xzf_read(file, buf, 3);
		if (size == 0)
			break;

		fwrite(buf, 1, size, stdout);
	}
*/

/*
	int c;
	while ((c = xzf_getc(file)) >= 0)
		xzf_putc(xzf_stdout, c);
*/

	const unsigned char *b;
	size_t size;
	while ((size = xzf_peekin_start(file, &b, 1)) > 0) {
		xzf_write(xzf_stdout, b, size);
		xzf_peekin_end(file, size);
	}
}


extern int
main(int argc, char **argv)
{
	xzf_stdio_open();

	if (argc > 1) {
		for (int i = 1; i < argc; ++i) {
			xzf_stream *file = xzf_fd_open(
					argv[i], XZF_READ | XZF_REGFILE, 0);
			file = xzf_gzin_open(file, 0);
			cat_file(file);
			xzf_close(file, 0);
		}
	} else {
		xzf_stream *file = xzf_gzin_open(xzf_stdin, 0);
		cat_file(file);
		xzf_close(file, XZF_CL_DETACH);
// 		xzf_putc(xzf_stdout, xzf_getc(xzf_stdin));
	}

	return 0;
}
