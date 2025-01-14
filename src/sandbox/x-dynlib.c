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

#define _GNU_SOURCE /* for secure_getenv */

#include <stdlib.h>
#include <dlfcn.h>

#include "x-dynlib.h"
#include "x-errno.h"

int x_dynlib_open(const char *filename, x_dynlib_t *dynlib, int global, int lazy)
{
	int flags;
	char *notdeep;

	/* compute the dlopen flags */
	flags = lazy ? RTLD_LAZY : RTLD_NOW;
	flags |= global ? RTLD_GLOBAL : RTLD_LOCAL;

	/* For ASan mode, export AFB_NO_RTLD_DEEPBIND=1, to disable RTLD_DEEPBIND */
	notdeep = secure_getenv("AFB_NO_RTLD_DEEPBIND");
	if (!notdeep || notdeep[0] != '1' || notdeep[1])
		flags |= RTLD_DEEPBIND;

	/* open the library now */
	dynlib->handle = dlopen(filename, flags);
	return dynlib->handle ? 0 : X_ENODATA;
}

void x_dynlib_close(x_dynlib_t *dynlib)
{
	dlclose(dynlib->handle);
}

int x_dynlib_symbol(x_dynlib_t *dynlib, const char* name, void** ptr)
{
	void *p = dlsym(dynlib->handle, name);
	*ptr = p;
	return p ? 0 : X_ENOENT;
}

const char* x_dynlib_error(const x_dynlib_t *dynlib)
{
	return dlerror();
}
