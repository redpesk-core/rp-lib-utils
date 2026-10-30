
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
#include "rp-jsonc-path.h"

/**
 * This enumeration encodes the reason why the callback
 * is called.
 */
typedef
enum rp_jsonc_iter_visit {
	/**
	 * Called on leaf object.
	 * A negative return stops iteration.
	 */
	RP_JSONC_ITER_LEAF          = 0,

	/**
	 * Called on object before entering it.
	 * A negative return stops iteration.
	 * A zero return avoid entering the object.
	 * A positive return validate entering the object.
	 */
	RP_JSONC_ITER_BEFORE_OBJECT = 1,

	/**
	 * Called on object when leaving it.
	 * A negative return stops iteration.
	 */
	RP_JSONC_ITER_AFTER_OBJECT  = 2,

	/**
	 * Called on array before entering it.
	 * A negative return stops iteration.
	 * A zero return avoid entering the array.
	 * A positive return validate entering the array.
	 */
	RP_JSONC_ITER_BEFORE_ARRAY  = 3,

	/**
	 * Called on array when leaving it.
	 * A negative return stops iteration.
	 */
	RP_JSONC_ITER_AFTER_ARRAY   = 4,

	/**
	 * Called on an item of a container (object or array) before
	 * iterating in the item.
	 * A negative return stops iteration.
	 * A zero return avoid iterating in the item.
	 * A positive return validate iterating in the item.
	 */
	RP_JSONC_ITER_ELEMENT       = 5
}
	rp_jsonc_iter_visit_t;

/**
 * This enumeration defines mask of what callback expects.
 */
typedef
enum rp_jsonc_iter_want {
	/** The callback expects RP_JSONC_ITER_LEAF */
	RP_JSONC_ITER_WANT_LEAF          = 1,

	/** The callback expects RP_JSONC_ITER_BEFORE_OBJECT */
	RP_JSONC_ITER_WANT_BEFORE_OBJECT = 2,

	/** The callback expects RP_JSONC_ITER_AFTER_OBJECT */
	RP_JSONC_ITER_WANT_AFTER_OBJECT  = 4,

	/** The callback expects RP_JSONC_ITER_BEFORE_ARRAY */
	RP_JSONC_ITER_WANT_BEFORE_ARRAY  = 8,

	/** The callback expects RP_JSONC_ITER_AFTER_ARRAY */
	RP_JSONC_ITER_WANT_AFTER_ARRAY   = 16,

	/** The callback expects RP_JSONC_ITER_ELEMENT */
	RP_JSONC_ITER_WANT_ELEMENT       = 32,

	/** The callback expects every visit */
	RP_JSONC_ITER_WANT_ALL           = 63
}
	rp_jsonc_iter_want_t;

/**
 * Callback type of iterators.
 * The received arguments are:
 *  - closure: the given closure argument
 *  - object: the target object
 *  - visit: the reason of the call
 *  - path: the path to the visited object
 * The value returned is interpreted by the iteration
 * process. In all cases, a negative value stops the
 * iteration. Otherwise, interpretation of the returned
 * value depends on the value of 'visit',
 * @see rp_jsonc_iter_visit_t for details.
 */
typedef
int (*rp_jsonc_iter_cb)(
	void *closure,
	struct json_object *object,
	rp_jsonc_iter_visit_t visit,
	rp_jsonc_path_t *path);

/**
 * Walk through the tree of the JSON object 'object'.
 * Calls the 'callback' with the given 'closure'
 * accordingly to the expected mask 'want'.
 *
 * @param object   the root to walk through
 * @param want     or of masks of expected reason for calling back
 * @param callback the function to call
 * @param closure  closure for the callback
 *
 * @return 0 on normal completion or the negative value
 *         returned by the callback that stopped the walk
 */
extern
int
rp_jsonc_iter_walk(
	struct json_object *object,
	rp_jsonc_iter_want_t want,
	rp_jsonc_iter_cb callback,
	void *closure);

/**
 * Walk through the tree of the JSON object 'object'.
 * Calls the 'callback' with the given 'closure'
 * accordingly to the expected mask 'want'.
 *
 * During the walk, it is possible to alter the tree
 * by replacing the field 'object' given in 'path'
 * received by the callback.
 *
 * @param object   pointer to the root to walk through
 * @param want     or of masks of expected reason for calling back
 * @param callback the function to call
 * @param closure  closure for the callback
 *
 * @return the negative value returned by the callback that stopped the walk
 *         or zero if the root object didn't change or 1 if the root object
 *         changed (in that case, the original root object is released
 *         using json_object_put).
 */
extern
int
rp_jsonc_iter_alter(
	struct json_object **object,
	rp_jsonc_iter_want_t want,
	rp_jsonc_iter_cb callback,
	void *closure);

#ifdef	__cplusplus
}
#endif
