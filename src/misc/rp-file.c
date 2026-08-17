/*
 * Copyright (C) 2015-2026 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _GNU_SOURCE

#include "rp-file.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int rp_file_get(const char *file, char **content, size_t *size)
{
	return rp_file_get_at(AT_FDCWD, file, content, size);
}

int rp_file_get_at(int dfd, const char *file, char **content, size_t *size)
{
	int rc, f;
	struct stat s;
	char *r;
	ssize_t rsz;
	size_t sz, i;

	if (content)
		*content = NULL;
	rc = openat(dfd, file, O_RDONLY);
	if (rc < 0)
		rc = -errno;
	else {
		f = rc;
		rc = fstat(f, &s);
		if (rc != 0) {
			rc = -errno;
		} else if (!S_ISREG(s.st_mode)) {
			rc = -EBADF;
		} else {
			sz = (size_t)s.st_size;
			if (content) {
				r = malloc(sz + 1);
				if (!r) {
					rc = -ENOMEM;
				} else {
					i = 0;
					while (rc == 0 && i < sz) {
						rsz = read(f, r + i, sz - i);
						if (rsz == 0)
							sz = i;
						else if (rsz > 0)
							i += (size_t)rsz;
						else if (errno != EINTR && errno != EAGAIN) {
							free(r);
							rc = -errno;
						}
					}
					if (rc == 0) {
						r[sz] = 0;
						*content = r;
					}
				}
			}
			if (size)
				*size = sz;
		}
		close(f);
	}
	return rc;
}

int rp_file_put(const char *file, const void *content, size_t size)
{
	return rp_file_put_at(AT_FDCWD, file, content, size);
}

int rp_file_put_at(int dfd, const char *file, const void *content, size_t size)
{
	int rc, f;
	ssize_t wsz;
	size_t i;

	rc = openat(dfd, file, O_WRONLY|O_CREAT|O_TRUNC, 0666);
	if (rc < 0)
		rc = -errno;
	else {
		if (size == (size_t)(ssize_t)-1)
			size = strlen(content);
		f = rc;
		i = 0;
		rc = 0;
		while (rc == 0 && i < size) {
			wsz = write(f, content + i, size - i);
			if (wsz >= 0)
				i += (size_t)wsz;
			else if (errno != EINTR && errno != EAGAIN) {
				unlinkat(dfd, file, 0);
				rc = -errno;
			}
		}
		close(f);
	}
	return rc;
}

