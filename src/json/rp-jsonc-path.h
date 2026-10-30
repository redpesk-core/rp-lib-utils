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

#pragma once

#ifdef	__cplusplus
extern "C" {
#endif

#include <json-c/json.h>

/**
 * Structure recording a path.
 *
 * This structure is filled by functions 'rp_jsonc_iter_walk'
 * and 'rp_jsonc_iter_alter' of rp-jsonc-iter as indication
 * of their position in the exploration of the JSON tree.
 *
 * This structure can also be used by any application.
 *
 * At root, 'object' must be the root object and 'parent'
 * and 'key' must be NULL.
 */
typedef struct rp_jsonc_path
{
	/**
	 * Path of the parent object.
	 * Should be NULL at root of the parent path.
	 */
	const struct rp_jsonc_path *parent;

	/**
	 * The object of the current path
	 */
	struct json_object *object;

	/**
	 * Key of 'object' in parent's object.
	 * It means that parent object (this->parent->object)
	 * is a dictionary.
	 * This field 'key' is NULL in two cases:
	 *  - at root, when 'parent' == NULL
	 *  - the parent object is an ARRAY.
	 */
	const char *key;

	/**
	 * Index of 'object' in parent's array.
	 * It means that parent object (this->parent->object)
	 * is an array. That is the case when 'parent' != NULL
	 * and 'key' == NULL.
	 */
	size_t index;
}
	rp_jsonc_path_t;

/**
 * Computes the path location of jso within root. The returned path is a string
 * that must be freed by the caller. NULL is returned if jso is not part of root.
 *
 * @param root the root object possibly containing jso
 * @param jso the object whose path within root is queried
 *
 * @return NULL if jso is not part of root or a string that must be freed using 'free'
 */
extern char *rp_jsonc_path(struct json_object *root, struct json_object *jso) __attribute__ ((deprecated));

/**
 * Checks that the given path is valid. A path is valid
 * if it is not circular and ends with a valid root.
 * This function can also ckeck on option (when 'ckobj'
 * isn't nul) that parent object matches its expected type.
 * When 'ckobj' is nul, this routine dosn't check any field 'object'.
 *
 * @param path the path to validate
 * @param ckobj flag if the parent object type should be checked
 *
 * @return 1 if valid or zero is invalid
 */
extern int rp_jsonc_path_valid(const rp_jsonc_path_t *path, int ckobj);

/**
 * From a given path computes the length (without trailing zero) of the
 * JSON pointer representing it. For this routine, the field 'object'
 * of paths is not used.
 *
 * CAUTION: this routine doesn't check that 'path' is valid. If the path
 * must be checked, use the function @see 'rp_jsonc_path_valid' before
 * calling this function.
 *
 * @param path the path whose pointer length is to compute
 *
 * @return the length of the pointer
 *
 * @see https://www.rfc-editor.org/info/rfc6901
 */
extern size_t rp_jsonc_path_pointer_length(const rp_jsonc_path_t *path);

/**
 * From a given path computes the JSON pointer representing it.
 * Returns the computed JSON pointer as a freshly allocated zero
 * termminated string that caller should free.
 *
 * CAUTION: this routine doesn't check that 'path' is valid. If the path
 * must be checked, use the function @see 'rp_jsonc_path_valid' before
 * calling this function.
 *
 * @param path the path whose JSON pointer is to compute
 *
 * @return the comuted JSON pointer or NULL if allocation failed.
 *
 * @see https://www.rfc-editor.org/info/rfc6901
 */
extern char *rp_jsonc_path_pointer(const rp_jsonc_path_t *path);

#ifdef	__cplusplus
}
#endif
