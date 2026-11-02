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

#include "rp-jsonc-path.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

/* internal of computing JSON pointer length */
static size_t pointer_length(const rp_jsonc_path_t *path, size_t length)
{
	if (path == NULL || path->parent == NULL)
		return length;
	else {
		if (path->key != NULL) {
			size_t len = strlen(path->key);
			length += 1 + len;
			while (len > 0) {
				char c = path->key[--len];
				if (c == '~' || c == '/')
					length++;
			}
		}
		else {
			size_t idx = path->index;
			while (idx >= 100) {
				idx /= 100;
				length += 2;
			}
			length += idx > 9 ? 3 : 2;
		}
		return pointer_length(path->parent, length);
	}
}

/*
 * Internal of computing JSON pointer.
 *
 * The build is done recurssively starting by the end of
 * the resulting path.
 */
static char *pointer_get(const rp_jsonc_path_t *path, char *end)
{
	if (path == NULL || path->parent == NULL)
		return end;
	else {
		if (path->key != NULL) {
			size_t len = strlen(path->key);
			while (len > 0) {
				char c = path->key[--len];
				if (c == '~') {
					*--end = '0';
					*--end = '~';
				}
				else if (c == '/') {
					*--end = '1';
					*--end = '~';
				}
				else
					*--end = c;
			}
		}
		else {
			size_t idx = path->index;
			do {
				*--end = (char)('0' + (idx % 10));
				idx /= 10;
			}
			while(idx > 0);
		}
		*--end = '/';
		return pointer_get(path->parent, end);
	}
}

/* validation of a path */
int rp_jsonc_path_valid(const rp_jsonc_path_t *path, int ckobj)
{
	const rp_jsonc_path_t *slow = path;
	int len = 0;

	if (path == NULL)
		return 0;

	for(;;) {
		/* at root ? */
		if (path->parent == NULL)
			/* check if valid root */
			return path->key == NULL;

		/* not at root, should it check the container? */
		if (ckobj) {
			/* should the container be an object? */
			if (path->key != NULL) {
				/* yes, check it */
				if (!json_object_is_type(path->parent->object, json_type_object))
					return 0;
			}
			else {
				/* no, then check it is an array */
				if (!json_object_is_type(path->parent->object, json_type_array))
					return 0;
			}
		}

		/* go to parent */
		path = path->parent;

		/* check using a slow iterator that there is no cycle */
		if (path == slow)
			return 0;
		if (len++ & 1)
			slow = slow->parent;
	}
}

size_t rp_jsonc_path_pointer_length(const rp_jsonc_path_t *path)
{
	assert(rp_jsonc_path_valid(path, 0));
	return pointer_length(path, 0);
}

char *rp_jsonc_path_pointer(const rp_jsonc_path_t *path)
{
	assert(rp_jsonc_path_valid(path, 0));
	size_t length = pointer_length(path, 0);
	char *pointer = malloc(1 + length);
	if (pointer != NULL) {
		char *end = &pointer[length];
		*end = 0;
		end = pointer_get(path, end);
		assert(pointer == end);
	}
	return pointer;
}

int rp_jsonc_path_search(struct json_object *root, const char *pointer, struct json_object **result)
{
	int number, idx;
	size_t value;
	char key[200];

	*result = NULL;
	while (*pointer) {

		/* valid pointer? */
		if (*pointer++ != '/')
			return 0; /* malformed pointer */

		/* copy with unescaping the field */
		number = 1;
		idx = 0;
		value = 0;
		while (*pointer != '\0' && *pointer != '/') {
			if (*pointer != '~') {
				key[idx] = *pointer;
				if (number) {
					if ('0' <= *pointer && *pointer <= '9')
						value = 10 * value + (size_t)(*pointer - '0');
					else
						number = 0;
				}
			}
			else {
				switch (*++pointer) {
				case '0': key[idx] = '~'; break;
				case '1': key[idx] = '/'; break;
				default: return -1; /* malformed pointer */
				}
				number = 0;
			}
			pointer++;
			if (++idx == (int)sizeof key)
				return 0; /* key too long */
		}

		/* extract the key */
		switch (json_object_get_type(root)) {
		case json_type_object:
			key[idx] = 0;
			if (!json_object_object_get_ex(root, key, &root))
				return 0;
			break;
		case json_type_array:
			if (!number || idx == 0)
				return 0; /* bad array index */
			if (value >= json_object_array_length(root))
				return 0; /* not found */
			root = json_object_array_get_idx(root, value);
			break;
		default:
			return 0; /* container missing in path */
		}
	}
	/* end of the pointer */
	*result = root;
	return 1;
}
