/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Kazuho Oku
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <errno.h>
#include <unistd.h>
#include "unco.h"

ssize_t unco_read_nosig(int fd, void *data, size_t len)
{
	ssize_t ret;
	while ((ret = read(fd, data, len)) == -1
		&& (errno == EAGAIN || errno == EWOULDBLOCK))
		;
	return ret;
}

int unco_full_write(int fd, const void *data, size_t len)
{
	ssize_t wret;
	size_t off = 0, chunksz;

	while (off != len) {
		chunksz = len - off;
		if (chunksz >= 10485760)
			chunksz = 10485760;
		wret = write(fd, (const char*)data + off, chunksz);
		if (wret == -1) {
			if (! (errno == EAGAIN || errno == EWOULDBLOCK)) {
				return -1;
			}
		} else {
			off += wret;
		}
	}

	return 0;
}

int unco_copyfd(int srcfd, int dstfd)
{
	char buf[4096];
	ssize_t rret;

	while ((rret = unco_read_nosig(srcfd, buf, sizeof(buf))) > 0) {
		if (unco_full_write(dstfd, buf, rret) != 0)
			return -1;
	}

	return rret;
}

int unco_utimes(int fd, const struct stat *st, int (*futimes)(int, const struct timeval times[2]))
{
	struct timeval times[2];

	times[0].tv_sec = st->st_atime;
	times[0].tv_usec = 0;
	times[1].tv_sec = st->st_mtime;
	times[1].tv_usec = 0;
	return futimes(fd, times);
}
