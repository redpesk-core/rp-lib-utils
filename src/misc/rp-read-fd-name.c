/*
 * Copyright (C) 2015-2022 IoT.bzh Company
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


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define NUMLEN 21 /* maximum number of digit for 64bits integers 3*ceil(64/10) */
static const char PROC_FD_FMT_STR[] = "/proc/%d/fd/%d";

ssize_t rp_read_fd_name(int fileno, char *buffer, size_t size)
{
	char path[NUMLEN + NUMLEN + sizeof PROC_FD_FMT_STR];
	ssize_t result;

	snprintf (path, sizeof path, PROC_FD_FMT_STR, getpid(), fileno);
	result = readlink(path, buffer, size);
	if (result >= 0) {
		result = result < size ? result : (ssize_t)(size - 1);
		buffer[result] = '\0';
	}
	return result;
}

