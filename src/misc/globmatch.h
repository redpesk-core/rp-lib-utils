/*
 * Copyright (C) 2015-2021 IoT.bzh Company
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

#pragma once

#define GLOB    '*'

extern unsigned globmatch(const char *pat, const char *str);
extern unsigned globmatchi(const char *pat, const char *str);

#if WITH_FNMATCH

#include <fnmatch.h>

#else

#define	FNM_PATHNAME	(1 << 0)	/* No wildcard can ever match `/'.  */
#define	FNM_NOESCAPE	(1 << 1)	/* Backslashes don't quote special chars.  */
#define	FNM_PERIOD	(1 << 2)	/* Leading `.' is matched only explicitly.  */
#define FNM_FILE_NAME	FNM_PATHNAME	/* Preferred GNU name.  */
#define FNM_LEADING_DIR (1 << 3)	/* Ignore `/...' after a match.  */
#define FNM_CASEFOLD	(1 << 4)	/* Compare without regard to case.  */
#define FNM_EXTMATCH	(1 << 5)	/* Use ksh-like extended matching. */
#define	FNM_NOMATCH	1

extern int fnmatch(const char *pattern, const char *string, int flags);

#endif
