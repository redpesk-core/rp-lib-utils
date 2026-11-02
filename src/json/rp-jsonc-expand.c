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

#include "rp-jsonc-expand.h"
#include "rp-jsonc-iter.h"

/**
 * Structure recording callbacks
 */
struct cbs
{
	/** closure of the callbacks */
	void *closure;

	/** callback processing objects */
	rp_jsonc_expandcb expand_object;

	/** callback processing strings */
	rp_jsonc_expandcb expand_string;
};

static int expand(struct json_object **object, const struct cbs *cbs);

static
int expandcb(
	void *closure,
	struct json_object *object,
	rp_jsonc_iter_visit_t visit,
	rp_jsonc_path_t *path
) {
	const struct cbs *cbs = closure;
	switch (visit) {
	case RP_JSONC_ITER_LEAF:
		if (cbs->expand_string != NULL && json_object_is_type(object, json_type_string))
			path->object = cbs->expand_string(cbs->closure, object, path);
		break;
	case RP_JSONC_ITER_AFTER_OBJECT:
		if (cbs->expand_object != NULL) {
			path->object = cbs->expand_object(cbs->closure, object, path);
			if (path->object != object)
				expand(&path->object, cbs);
		}
		break;
	default:
		break;
	}
	return 1;
}

static
int expand(
	struct json_object **object,
	const struct cbs *cbs
) {
	return rp_jsonc_iter_alter(object, RP_JSONC_ITER_WANT_AFTER_OBJECT | RP_JSONC_ITER_WANT_LEAF,
			expandcb, (void*)cbs);
}

/* expand an object using user functions */
struct json_object *
rp_jsonc_expand(
	struct json_object *object,
	void *closure,
	rp_jsonc_expandcb expand_object,
	rp_jsonc_expandcb expand_string
) {
	struct cbs cbs;

	cbs.closure = closure;
	cbs.expand_object = expand_object;
	cbs.expand_string = expand_string;

	expand(&object, &cbs);
	return object;
}

