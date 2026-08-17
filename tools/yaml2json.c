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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "rp-yaml.h"

#ifndef JSON_C_TO_STRING_COLOR
#define JSON_C_TO_STRING_COLOR 0
#endif
#ifndef	JSON_C_TO_STRING_NOSLASHESCAPE
#define	JSON_C_TO_STRING_NOSLASHESCAPE 0
#endif
#ifndef	JSON_C_TO_STRING_NOZERO
#define	JSON_C_TO_STRING_NOZERO 0
#endif
#ifndef	JSON_C_TO_STRING_PRETTY
#define	JSON_C_TO_STRING_PRETTY 0
#endif

#define FLAGS \
	  JSON_C_TO_STRING_NOSLASHESCAPE \
	| JSON_C_TO_STRING_NOZERO \
	| JSON_C_TO_STRING_PRETTY

int main(int ac, char **av)
{
	int rc;
	json_object *obj;
	const char *fname;
	const char *arg;
	FILE *file;
	int ok = 0;
	int flags = FLAGS;
	int colorize = isatty(1);

	/* check coloring option */
	while (!ok) {
		arg = *++av;
#define ISOPT(short,long) (strcmp(arg,short)==0 || strcmp(arg,long)==0)
		if (arg == NULL)
			ok = 1;
		else if (ISOPT("-C","--color"))
			colorize = 1;
		else if (ISOPT("-c","--no-color"))
			colorize = 0;
		else
			ok = 1;
	}
	if (colorize)
		flags |= JSON_C_TO_STRING_COLOR;

	/* get file to process */
	fname = arg;
	if (fname == NULL) {
		fname = "<stdin>";
		file = stdin;
	}
	else {
		file = fopen(fname, "rb");
		if (file == NULL) {
			fprintf(stderr, "can't open %s: %m\n", fname);
			exit(1);
		}
	}

	/* process now */
	rc = rp_yaml_file_to_json_c(&obj, file, fname);
	if (rc < 0)
		exit(1);
	json_object_to_fd(1, obj, flags);
	return !rc;
}
