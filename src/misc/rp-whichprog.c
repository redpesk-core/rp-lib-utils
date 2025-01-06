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

#define _GNU_SOURCE

#include "rp-whichprog.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

char *rp_whichprog(const char *name, const char *evar, const char *dflt)
{
    size_t off, lename = 1 + strlen(name);
    char buffer[PATH_MAX];
    const char *path, *next, *result = NULL;

    /* if environment variable exist it takes precedence */
    if (evar != NULL)
        result = getenv(evar);
    /* search in path */
    for(path = secure_getenv("PATH") ; path != NULL && result == NULL ; path = next) {
        /* extract part between colons */
        for( ; *path == ':' ; path++);
        for(off = 1 ; path[off] && path[off] != ':' ; off++);
        /* prepare iteration (because off can change) */
        next = path[off] ? &path[off + 1] : NULL;
        if (off + lename < sizeof buffer) {
            /* enougth space, makes PATH/NAME in buffer */
            memcpy(buffer, path, off);
            if (off == 0 || path[off - 1] != '/')
                buffer[off++] = '/';
            memcpy(&buffer[off], name, lename);
            /* if matching an executable, its done */
            if (access(buffer, X_OK) == 0)
                result = buffer;
        }
    }
    /* return the found result or els the default value or else the name  */
    return strdup(result ?: dflt ?: name);
}

