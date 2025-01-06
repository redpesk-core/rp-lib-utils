/*
 * Copyright (C) 2015-2025 IoT.bzh Company
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


#include "rp-systemd.h"

#include <stdlib.h>
#include "x-errno.h"

#if !defined(SD_LISTEN_FDS_START)
# define SD_LISTEN_FDS_START 3
#endif

int rp_systemd_fds_for(const char *name)
{
	int idx, fd;
	const char *fdnames = getenv("LISTEN_FDNAMES");
	if (fdnames != NULL) {
		for (fd = SD_LISTEN_FDS_START ; *fdnames != '\0' ; fd++) {
			idx = 0;
			while (fdnames[idx] != ':' && name[idx] != '\0' && fdnames[idx] == name[idx])
				idx++;
			if (name[idx] == '\0' && (fdnames[idx] == ':' || fdnames[idx] == '\0'))
				return fd;
			while (fdnames[idx] != ':' && fdnames[idx] != '\0')
				idx++;
			while (fdnames[idx] == ':')
				idx++;
			fdnames = &fdnames[idx];
		}
	}
	return X_ENOENT;
}
