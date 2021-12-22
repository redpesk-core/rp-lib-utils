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

#include "expand-json.h"

/**
 * Structure recording the path path of the expansion
 */
struct expand_json_path
{
	/** depth */
	int depth;

	/** previous, aka parent, path */
	expand_json_path_t previous;

	/** object in expansion at current depth */
	struct json_object *object;

	/** index of expanded child if object is an array */
	size_t index;

	/** key of expanded child if object is an object */
	const char *key;

};

/**
 * Get the path of index or NULL if index is invalid
 *
 * @param path the initial path (the deepest)
 * @param index   the required index
 *
 * @return the path of the index or NULL if the index is invalid
 */
static inline
expand_json_path_t
at(expand_json_path_t path, int index)
{
	if (index < 0 || path->depth < index)
		return NULL;
	while (index != path->depth)
		path = path->previous;
	return path;
}

/**
 * Internal function for expanding json objects
 *
 * @param object the object to be expanded
 * @param previous link to the parent object
 * @param expand_object a function for replacing json objects or NULL
 * @param expand_string a function for replacing json strings or NULL
 *
 * @return either the given object or its replacement
 */
static
struct json_object *
expand(
	struct json_object *object,
	void *closure,
	expand_json_cb expand_object,
	expand_json_cb expand_string,
	expand_json_path_t previous
) {
#if JSON_C_VERSION_NUM >= 0x000d00
	size_t idx, len;
#else
	int idx, len;
#endif
	enum json_type type;
	struct json_object_iterator it, end;
	struct json_object *curval, *nxtval;
	struct expand_json_path path;

	/* inspect type of the object */
	type = json_object_get_type(object);
	switch (type) {
	case json_type_object:
		path.index = 0;
		path.previous = previous;
		path.depth = previous->depth + 1;
		path.object = object;
		/* first, expand content of the object */
		it = json_object_iter_begin(object);
		end = json_object_iter_end(object);
		while (!json_object_iter_equal(&it, &end)) {
			curval = json_object_iter_peek_value(&it);
			path.key = json_object_iter_peek_name(&it);
			nxtval = expand(curval, closure, expand_object, expand_string, &path);
			if (nxtval != curval)
				json_object_object_add(object, path.key, nxtval);
			json_object_iter_next(&it);
		}
		/* expand the result using the function */
		if (expand_object != NULL) {
			nxtval = expand_object(closure, object, previous);
			if (nxtval != object) {
				/* the function returned a new object, try recursive expansion of it */
				object = expand(nxtval, closure, expand_object, expand_string, previous);
				if (nxtval != object)
					json_object_put(nxtval);
			}
		}
		break;
	case json_type_array:
		/* arrays are not expanded but their values yes */
		path.key = 0;
		path.previous = previous;
		path.depth = previous->depth + 1;
		path.object = object;
		len = json_object_array_length(object);
		for (idx = 0 ; idx < len ; idx++) {
			curval = json_object_array_get_idx(object, idx);
			path.index = (size_t)idx;
			nxtval = expand(curval, closure, expand_object, expand_string, &path);
			if (nxtval != curval)
				json_object_array_put_idx(object, idx, nxtval);
		}
		break;
	case json_type_string:
		/* expansion of strings using given function */
		if (expand_string != NULL)
			object = expand_string(closure, object, previous);
		break;
	default:
		/* no expansion on number, bool, null */
		break;
	}
	return object;
}

/* expand an object using user functions */
struct json_object *
expand_json(
	struct json_object *object,
	void *closure,
	expand_json_cb expand_object,
	expand_json_cb expand_string
) {
	struct expand_json_path path;

	path.depth = -1;
	path.previous = NULL;
	path.index = 0;
	path.key = NULL;
	path.object = NULL;

	return expand(object, closure, expand_object, expand_string, &path);
}

/* length of the path */
int expand_json_path_length(expand_json_path_t path)
{
	return path->depth + 1;
}

/* object at index */
struct json_object *expand_json_path_get(expand_json_path_t path, int index)
{
	path = at(path, index);
	return path ? path->object : NULL;
}

/* is object at index an object? */
int expand_json_path_is_object(expand_json_path_t path, int index)
{
	path = at(path, index);
	return path && path->key != NULL;
}

/* is object at index an array */
int expand_json_path_is_array(expand_json_path_t path, int index)
{
	path = at(path, index);
	return path && path->key == NULL;
}

/* key of the subobject of the object at index */
const char *expand_json_path_key(expand_json_path_t path, int index)
{
	path = at(path, index);
	return path ? path->key : NULL;
}

/* index of the subobject of the array at index */
size_t expand_json_path_index(expand_json_path_t path, int index)
{
	path = at(path, index);
	return path ? path->index : 0;
}
