/*
 * Copyright (C) 2015-2021 IoT.bzh Company
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


#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>

#include "x-errno.h"

#include "process-name.h"

int process_name_set_name(const char *name)
{
	return prctl(PR_SET_NAME, name, 0, 0, 0) < 0 ? -errno : 0;
}

int process_name_replace_cmdline(char **argv, const char *name)
{
	char *beg, *end, **av, c;

	/* update the command line */
	av = argv;
	if (!av)
		return X_EINVAL; /* no command line update required */

	/* longest prefix */
	end = beg = *av;
	while (*av)
		if (*av++ == end)
			while(*end++)
				;
	if (end == beg)
		return X_EINVAL; /* nothing to change */

	/* patch the command line */
	av = &argv[1];
	end--;
	while (beg != end && (c = *name++)) {
		if (c != ' ' || !*av)
			*beg++ = c;
		else {
			*beg++ = 0;
			*av++ = beg;
		}
	}
	/* terminate last arg */
	if (beg != end)
		*beg++ = 0;
	/* inform system */
	prctl(PR_SET_MM, PR_SET_MM_ARG_END, (unsigned long)(intptr_t)beg, 0, 0);
	/* update remaining args (for keeping initial length correct) */
	while (*av)
		*av++ = beg;
	/* fulfill last arg with spaces */
	while (beg != end)
		*beg++ = ' ';
	*beg = 0;

	return 0;
}

