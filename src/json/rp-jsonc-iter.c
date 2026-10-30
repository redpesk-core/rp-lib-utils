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

#include "rp-jsonc-iter.h"

/**
 * Structure recording callbacks
 */
struct cbs
{
	/** the callback */
	rp_jsonc_iter_cb callback;

	/** closure of the callback */
	void *closure;

	/** expected visits */
	rp_jsonc_iter_want_t want;

	/** is replacing? */
	int replacing;
};

static inline
int
visit(
	const struct cbs *cbs,
	rp_jsonc_path_t *path,
	rp_jsonc_iter_visit_t visit
) {
	return cbs->callback(cbs->closure, path->object, visit, path);
}

/**
 * Internal function for expanding json objects
 *
 * @param object   the object to be expanded
 * @param cbs      structure holding callbacks
 * @param path     link to the path object
 *
 * @return either the given object or its replacement
 */
static
int
iter(
	const struct cbs *cbs,
	rp_jsonc_path_t *path
) {
#if JSON_C_VERSION_NUM >= 0x000d00
	size_t idx, len;
#else
	int idx, len;
#endif
	int rc;
	enum json_type type;
	struct json_object_iterator it, end;
	struct json_object *object;
	rp_jsonc_path_t chldpath;

	/* inspect type of the path object */
	object = path->object;
	type = json_object_get_type(object);
	switch (type) {

	/* OBJECT */
	case json_type_object:
		/* check if entering object is ok */
		rc = 1;
		if (cbs->want & RP_JSONC_ITER_WANT_BEFORE_OBJECT) {
			rc = visit(cbs, path, RP_JSONC_ITER_BEFORE_OBJECT);
			if (rc < 0)
				return rc;
		}
		if (rc > 0) {
			/* iterate over content */
			chldpath.parent = path;
			chldpath.index = 0;
			it = json_object_iter_begin(object);
			end = json_object_iter_end(object);
			while (!json_object_iter_equal(&it, &end)) {
				chldpath.key = json_object_iter_peek_name(&it);
				chldpath.object = json_object_iter_peek_value(&it);
				rc = 1;
				if (cbs->want & RP_JSONC_ITER_WANT_ELEMENT)
					rc = visit(cbs, &chldpath, RP_JSONC_ITER_ELEMENT);
				if (rc > 0)
					rc = iter(cbs, &chldpath);
				if (rc < 0)
					return rc;
				if (cbs->replacing && chldpath.object != json_object_iter_peek_value(&it))
					json_object_object_add(object, chldpath.key, chldpath.object);
				json_object_iter_next(&it);
			}
		}
		rc = 0;
		if (cbs->want & RP_JSONC_ITER_WANT_AFTER_OBJECT) {
			rc = visit(cbs, path, RP_JSONC_ITER_AFTER_OBJECT);
			if (rc > 0)
				rc = 0;
		}
		break;

	/* ARRAY */
	case json_type_array:
		/* arrays are not expanded but their values yes */
		/* check if entering object is ok */
		rc = 1;
		if (cbs->want & RP_JSONC_ITER_WANT_BEFORE_ARRAY) {
			rc = visit(cbs, path, RP_JSONC_ITER_BEFORE_ARRAY);
			if (rc < 0)
				return rc;
		}
		if (rc > 0) {
			/* iterate over content */
			chldpath.parent = path;
			chldpath.key = NULL;
			len = json_object_array_length(object);
			for (idx = 0 ; idx < len ; idx++) {
				chldpath.index = (size_t)idx;
				chldpath.object = json_object_array_get_idx(object, idx);
				rc = 1;
				if (cbs->want & RP_JSONC_ITER_WANT_ELEMENT)
					rc = visit(cbs, &chldpath, RP_JSONC_ITER_ELEMENT);
				if (rc > 0)
					rc = iter(cbs, &chldpath);
				if (rc < 0)
					return rc;
				if (cbs->replacing && chldpath.object != json_object_array_get_idx(object, idx))
					json_object_array_put_idx(object, idx, chldpath.object);
			}
		}
		rc = 0;
		if (cbs->want & RP_JSONC_ITER_WANT_AFTER_ARRAY) {
			rc = visit(cbs, path, RP_JSONC_ITER_AFTER_ARRAY);
			if (rc > 0)
				rc = 0;
		}
		break;

	/* LEAF */
	default:
		rc = 0;
		if (cbs->want & RP_JSONC_ITER_WANT_LEAF) {
			rc = visit(cbs, path, RP_JSONC_ITER_LEAF);
			if (rc > 0)
				rc = 0;
		}
		break;
	}
	return rc;
}

/* expand an object using user functions */
int
rp_jsonc_iter_walk(
	struct json_object *object,
	rp_jsonc_iter_want_t want,
	rp_jsonc_iter_cb callback,
	void *closure
) {
	struct cbs cbs;
	rp_jsonc_path_t path;

	cbs.callback = callback;
	cbs.closure = closure;
	cbs.want = want;
	cbs.replacing = 0;

	path.parent = NULL;
	path.index = 0;
	path.key = NULL;
	path.object = object;

	return iter(&cbs, &path);
}

/* expand an object using user functions */
int
rp_jsonc_iter_alter(
	struct json_object **object,
	rp_jsonc_iter_want_t want,
	rp_jsonc_iter_cb callback,
	void *closure
) {
	struct cbs cbs;
	rp_jsonc_path_t path;
	int rc;

	cbs.callback = callback;
	cbs.closure = closure;
	cbs.want = want;
	cbs.replacing = 1;

	path.parent = NULL;
	path.index = 0;
	path.key = NULL;
	path.object = *object;

	rc = iter(&cbs, &path);
	if (rc >= 0) {
		if (*object == path.object)
			rc = 0;
		else {
			json_object_put(*object);
			*object = path.object;
			rc = 1;
		}
	}
	return rc;
}

