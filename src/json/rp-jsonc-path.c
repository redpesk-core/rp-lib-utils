/*
 * Copyright (C) 2015-2023 IoT.bzh Company
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
#include <stdio.h>
#include <limits.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

#include "rp-jsonc-path.h"

/**
 * Structure recording the path path of the expansion
 */
struct path
{
	/** previous, aka parent, path */
	struct path *previous;

	/** key of expanded child if object is an object */
	const char *key;

	/** index of expanded child if object is an array */
	size_t index;
};

/**
 * Compute the length of the string representation of path,
 * add it to the given value and return the sum.
 *
 * @param path the path to compute
 * @param got  the length to be add
 *
 * @return the sum of got and length of the string representation of path
 */
static size_t pathlen(struct path *path, size_t got)
{
	size_t v;

	/* terminal */
	if (path == NULL)
		return got;

	/* add length of current path item */
	if (path->key)
		/* length of ".keyname" */
		got += 1 + strlen(path->key);
	else {
		/* length of "[index]" */
		v = path->index;
		got += 3;
		while (v >= 100) {
			v /= 100;
			got += 2;
		}
		got += v > 9;
	}

	/* recursive computation of the result */
	return pathlen(path->previous, got);
}

/**
 * Put in base the string representation of path and returns a pointer
 * to the end.
 *
 * @param path the path whose string representation is to be computed
 * @param base where to put the string
 *
 * @return a pointer to the character after the end
 */
static char *pathset(struct path *path, char *base)
{
	size_t v;
	char buffer[30]; /* enough for 70 bits (3 * (70 / 10) = 21) */
	int i;

	if (path != NULL) {
		base = pathset(path->previous, base);

		if (path->key) {
			/* put ".key" */
			*base++ = '.';
			base = stpcpy(base,path->key);
		}
		else {
			/* compute reverse string of index in buffer */
			v = path->index;
			i = 0;
			do {
				buffer[i++] = (char)('0' + v % 10);
				v /= 10;
			} while(v);

			/* put "[index]" */
			*base++ = '[';
			while (i)
				*base++ = buffer[--i];
			*base++ = ']';
		}
	}
	return base;
}

/**
 * Search the path to the object 'jso' starting from 'root' that is at path 'previous'
 *
 * @param root current root of the search
 * @param jso  the object whose path is searched
 * @param previous path of root
 *
 * @return an allocated string representation of the path found or NULL
 */
static char *search(struct json_object *root, struct json_object *jso, struct path *previous)
{
#if JSON_C_VERSION_NUM >= 0x000d00
	size_t idx, len;
#else
	int idx, len;
#endif
	struct json_object_iterator it, end;
	struct path path;
	char *result = NULL;

	if (root == jso) {
		result = malloc(pathlen(previous, 1));
		if (result)
			*pathset(previous, result) = 0;
	}
	else if (json_object_is_type(root, json_type_object)) {
		path.index = 0;
		path.previous = previous;
		it = json_object_iter_begin(root);
		end = json_object_iter_end(root);
		while (result == NULL && !json_object_iter_equal(&it, &end)) {
			path.key = json_object_iter_peek_name(&it);
			result = search(json_object_iter_peek_value(&it), jso, &path);
			json_object_iter_next(&it);
		}
	}
	else if (json_object_is_type(root, json_type_array)) {
		path.key = 0;
		path.previous = previous;
		len = json_object_array_length(root);
		for (idx = 0 ; result == NULL && idx < len ; idx++) {
			path.index = (size_t)idx;
			result = search(json_object_array_get_idx(root, idx), jso, &path);
		}
	}
	return result;
}

/* get the path  from root to json or NULL if none exists */
char *rp_jsonc_path(struct json_object *root, struct json_object *jso)
{
	return search(root, jso, NULL);
}
