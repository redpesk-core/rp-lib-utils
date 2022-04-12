/*
 Copyright (C) 2015-2022 IoT.bzh Company

 Author: José Bollo <jose.bollo@iot.bzh>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is furnished
 to do so, subject to the following conditions:

 The above copyright notice and this permission notice (including the next
 paragraph) shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#include <string.h>
#include <limits.h>

#include "rp-jsonc.h"
#include "../misc/rp-base64.h"

#define STACKCOUNT  32

static const char ignore_all[] = " \t\n\r,:";
static const char pack_accept_arr[] = "][{snbiIfoOyY";
static const char pack_accept_key[] = "s}";
#define pack_accept_any (&pack_accept_arr[1])

static const char unpack_accept_arr[] = "*!][{snbiIfFoOyY";
static const char unpack_accept_key[] = "*!s}";
#define unpack_accept_any (&unpack_accept_arr[3])

static const char *pack_errors[_rp_jsonc_error_count_] =
{
	[rp_jsonc_error_none] = "unknown error",
	[rp_jsonc_error_null_object] = "null object",
	[rp_jsonc_error_truncated] = "truncated",
	[rp_jsonc_error_internal_error] = "internal error",
	[rp_jsonc_error_out_of_memory] = "out of memory",
	[rp_jsonc_error_invalid_character] = "invalid character",
	[rp_jsonc_error_too_long] = "too long",
	[rp_jsonc_error_too_deep] = "too deep",
	[rp_jsonc_error_null_spec] = "spec is NULL",
	[rp_jsonc_error_null_key] = "key is NULL",
	[rp_jsonc_error_null_string] = "string is NULL",
	[rp_jsonc_error_out_of_range] = "array too small",
	[rp_jsonc_error_incomplete] = "incomplete container",
	[rp_jsonc_error_missfit_type] = "missfit of type",
	[rp_jsonc_error_key_not_found] = "key not found",
	[rp_jsonc_error_bad_base64] = "bad base64 encoding"
};

/* position of the error code */
int rp_jsonc_get_error_position(int rc)
{
	if (rc < 0)
		rc = -rc;
	return (rc >> 4) + 1;
}


int rp_jsonc_get_error_code(int rc)
{
	if (rc < 0)
		rc = -rc;
	return rc & 15;
}

const char *rp_jsonc_get_error_string(int rc)
{
	rc = rp_jsonc_get_error_code(rc);
	if (rc >= (int)(sizeof pack_errors / sizeof *pack_errors))
		rc = 0;
	return pack_errors[rc];
}

static inline const char *skip(const char *d)
{
	while (*d && strchr(ignore_all, *d))
		d++;
	return d;
}

int rp_jsonc_vpack(struct json_object **result, const char *desc, va_list args)
{
	/* TODO: the case of structs with key being single char should be optimized */
	int notnull, nullable, rc, init;
	size_t sz, ssz, nsz;
	char *s, *sa;
	char c;
	const char *d, *sv;
	char buffer[256];
	struct { const uint8_t *in; size_t insz; char *out; size_t outsz; } bytes;
	struct { const char *str; size_t sz; } str;
	struct { struct json_object *cont, *key; const char *acc; char type; } stack[STACKCOUNT], *top;
	struct json_object *obj;

	ssz = sizeof buffer;
	s = buffer;
	top = stack;
	sa = NULL;
	top->key = NULL;
	top->cont = NULL;
	top->acc = pack_accept_any;
	top->type = 0;
	d = desc;
	if (!d)
		goto null_spec;
	d = skip(d);
	for(;;) {
		c = *d;
		if (!c)
			goto truncated;
		if (!strchr(top->acc, c))
			goto invalid_character;
		d = skip(d + 1);
		switch(c) {
		case 's':
			nullable = 0;
			notnull = 0;
			sz = 0;
			sv = 0;
			for (;;) {
				str.str = va_arg(args, const char*);
				if (*d == '?') {
					d = skip(d + 1);
					nullable = 1;
				}
				switch(*d) {
				case '%': str.sz = va_arg(args, size_t); d = skip(d + 1); break;
				case '#': str.sz = (size_t)va_arg(args, int); d = skip(d + 1); break;
				default: str.sz = str.str ? strlen(str.str) : 0; break;
				}
				if (str.str) {
					notnull = 1;
					nsz = sz + str.sz;
					if (!sv)
						sv = str.str;
					else {
						init = sv != s;
						if (nsz > ssz) {
							ssz += ssz;
							if (ssz < nsz)
								ssz = nsz;
							s = realloc(sa, ssz);
							if (!s)
								goto out_of_memory;
							if (!sa)
								memcpy(s, buffer, sz);
							sa = s;
						}
						if (init)
							memcpy(s, sv, sz);
						memcpy(&s[sz], str.str, str.sz);
						sv = s;
					}
					sz = nsz;
				}
				if (*d == '?') {
					d = skip(d + 1);
					nullable = 1;
				}
				if (*d != '+')
					break;
				d = skip(d + 1);
			}
			if (*d == '*')
				nullable = 1;
			if (notnull) {
				obj = json_object_new_string_len(sv, (int)sz);
				if (!obj)
					goto out_of_memory;
			} else if (nullable)
				obj = NULL;
			else
				goto null_string;
			break;
		case 'n':
			obj = NULL;
			break;
		case 'b':
			obj = json_object_new_boolean(va_arg(args, int));
			if (!obj)
				goto out_of_memory;
			break;
		case 'i':
			obj = json_object_new_int(va_arg(args, int));
			if (!obj)
				goto out_of_memory;
			break;
		case 'I':
			obj = json_object_new_int64(va_arg(args, int64_t));
			if (!obj)
				goto out_of_memory;
			break;
		case 'f':
			obj = json_object_new_double(va_arg(args, double));
			if (!obj)
				goto out_of_memory;
			break;
		case 'o':
		case 'O':
			obj = va_arg(args, struct json_object*);
			if (*d == '?')
				d = skip(d + 1);
			else if (*d != '*' && !obj)
				goto null_object;
			if (c == 'O')
				json_object_get(obj);
			break;
		case 'y':
		case 'Y':
			bytes.in = va_arg(args, const uint8_t*);
			bytes.insz = va_arg(args, size_t);
			if (bytes.in == NULL || bytes.insz == 0)
				obj = NULL;
			else {
				rc = rp_base64_encode(bytes.in, bytes.insz,
					&bytes.out, &bytes.outsz, 0, 0, c == 'y');
				if (rc < 0)
					goto out_of_memory;
				obj = json_object_new_string_len(bytes.out, (int)bytes.outsz);
				free(bytes.out);
				if (!obj)
					goto out_of_memory;
			}
			if (*d == '?')
				d = skip(d + 1);
			else if (*d != '*' && !obj) {
				obj = json_object_new_string_len(d, 0);
				if (!obj)
					goto out_of_memory;
			}
			break;
		case '[':
		case '{':
			if (++top >= &stack[STACKCOUNT])
				goto too_deep;
			top->key = NULL;
			if (c == '[') {
				top->type = ']';
				top->acc = pack_accept_arr;
				top->cont = json_object_new_array();
			} else {
				top->type = '}';
				top->acc = pack_accept_key;
				top->cont = json_object_new_object();
			}
			if (!top->cont)
				goto out_of_memory;
			continue;
		case '}':
		case ']':
			if (c != top->type || top <= stack)
				goto internal_error;
			obj = (top--)->cont;
			if (*d == '*' && !(c == '}' ? !!json_object_object_length(obj) : !!json_object_array_length(obj))) {
				json_object_put(obj);
				obj = NULL;
			}
			break;
		default:
			goto internal_error;
		}
		switch (top->type) {
		case 0:
			if (top != stack)
				goto internal_error;
			if (*d)
				goto invalid_character;
			*result = obj;
			free(sa);
			return 0;
		case ']':
			if (obj || *d != '*')
				json_object_array_add(top->cont, obj);
			if (*d == '*')
				d = skip(d + 1);
			break;
		case '}':
			if (!obj)
				goto null_key;
			top->key = obj;
			top->acc = pack_accept_any;
			top->type = ':';
			break;
		case ':':
			if (obj || *d != '*')
				json_object_object_add(top->cont, json_object_get_string(top->key), obj);
			if (*d == '*')
				d = skip(d + 1);
			json_object_put(top->key);
			top->key = NULL;
			top->acc = pack_accept_key;
			top->type = '}';
			break;
		default:
			goto internal_error;
		}
	}

null_object:
	rc = rp_jsonc_error_null_object;
	goto error;
truncated:
	rc = rp_jsonc_error_truncated;
	goto error;
internal_error:
	rc = rp_jsonc_error_internal_error;
	goto error;
out_of_memory:
	rc = rp_jsonc_error_out_of_memory;
	goto error;
invalid_character:
	rc = rp_jsonc_error_invalid_character;
	goto error;
too_deep:
	rc = rp_jsonc_error_too_deep;
	goto error;
null_spec:
	rc = rp_jsonc_error_null_spec;
	goto error;
null_key:
	rc = rp_jsonc_error_null_key;
	goto error;
null_string:
	rc = rp_jsonc_error_null_string;
	goto error;
error:
	do {
		json_object_put(top->key);
		json_object_put(top->cont);
	} while (--top >= stack);
	*result = NULL;
	rc = rc | (int)((d - desc) << 4);
	free(sa);
	return -rc;
}

int rp_jsonc_pack(struct json_object **result, const char *desc, ...)
{
	int rc;
	va_list args;

	va_start(args, desc);
	rc = rp_jsonc_vpack(result, desc, args);
	va_end(args);
	return rc;
}

static int vunpack(struct json_object *object, const char *desc, va_list args, int store)
{
	int rc = 0, optionnal, ignore;
	char c, xacc[2] = { 0, 0 };
	const char *acc;
	const char *d, *fit = NULL;
	const char *key = NULL;
	const char **ps = NULL;
	double *pf = NULL;
	int *pi = NULL;
	int64_t *pI = NULL;
	size_t *pz = NULL;
	uint8_t **py = NULL;
	struct { struct json_object *parent; const char *acc; int index; int count; char type; } stack[STACKCOUNT], *top;
	struct json_object *obj;
	struct json_object **po;

	xacc[0] = 0;
	ignore = 0;
	top = NULL;
	acc = unpack_accept_any;
	d = desc;
	if (!d)
		goto null_spec;
	d = skip(d);
	obj = object;
	for(;;) {
		fit = d;
		c = *d;
		if (!c)
			goto truncated;
		if (!strchr(acc, c))
			goto invalid_character;
		d = skip(d + 1);
		switch(c) {
		case 's':
			if (xacc[0] == '}') {
				/* expects a key */
				key = va_arg(args, const char *);
				if (!key)
					goto null_key;
				if (*d != '?')
					optionnal = 0;
				else {
					optionnal = 1;
					d = skip(d + 1);
				}
				if (ignore)
					ignore++;
				else {
					if (json_object_object_get_ex(top->parent, key, &obj)) {
						/* found */
						top->index++;
					} else {
						/* not found */
						if (!optionnal)
							goto key_not_found;
						ignore = 1;
						obj = NULL;
					}
				}
				xacc[0] = ':';
				acc = unpack_accept_any;
				continue;
			}
			/* get a string */
			if (store)
				ps = va_arg(args, const char **);
			if (!ignore) {
				if (!json_object_is_type(obj, json_type_string))
					goto missfit;
				if (store && ps)
					*ps = json_object_get_string(obj);
			}
			if (*d == '%') {
				d = skip(d + 1);
				if (store) {
					pz = va_arg(args, size_t *);
					if (!ignore && pz)
						*pz = (size_t)json_object_get_string_len(obj);
				}
			}
			break;
		case 'n':
			if (!ignore && !json_object_is_type(obj, json_type_null))
				goto missfit;
			break;
		case 'b':
			if (store)
				pi = va_arg(args, int *);

			if (!ignore) {
				if (!json_object_is_type(obj, json_type_boolean))
					goto missfit;
				if (store && pi)
					*pi = json_object_get_boolean(obj);
			}
			break;
		case 'i':
			if (store)
				pi = va_arg(args, int *);

			if (!ignore) {
				if (!json_object_is_type(obj, json_type_int))
					goto missfit;
				if (store && pi)
					*pi = json_object_get_int(obj);
			}
			break;
		case 'I':
			if (store)
				pI = va_arg(args, int64_t *);

			if (!ignore) {
				if (!json_object_is_type(obj, json_type_int))
					goto missfit;
				if (store && pI)
					*pI = json_object_get_int64(obj);
			}
			break;
		case 'f':
		case 'F':
			if (store)
				pf = va_arg(args, double *);

			if (!ignore) {
				if (!(json_object_is_type(obj, json_type_double) || (c == 'F' && json_object_is_type(obj, json_type_int))))
					goto missfit;
				if (store && pf)
					*pf = json_object_get_double(obj);
			}
			break;
		case 'o':
		case 'O':
			if (store) {
				po = va_arg(args, struct json_object **);
				if (!ignore && po) {
					if (c == 'O')
						obj = json_object_get(obj);
					*po = obj;
				}
			}
			break;
		case 'y':
		case 'Y':
			if (store) {
				py = va_arg(args, uint8_t **);
				pz = va_arg(args, size_t *);
			}
			if (!ignore) {
				if (obj == NULL) {
					if (store && py && pz) {
						*py = NULL;
						*pz = 0;
					}
				} else {
					if (!json_object_is_type(obj, json_type_string))
						goto missfit;
					if (store && py && pz) {
						rc = rp_base64_decode(
							json_object_get_string(obj),
							(size_t)json_object_get_string_len(obj),
							py, pz, 0);
						if (rc) {
							if (rc == -1)
								rc = rp_jsonc_error_out_of_memory;
							else
								rc = rp_jsonc_error_bad_base64;
							goto error;
						}
					}
				}
			}
			break;

		case '[':
		case '{':
			if (!top)
				top = stack;
			else if (++top  >= &stack[STACKCOUNT])
				goto too_deep;

			top->acc = acc;
			top->type = xacc[0];
			top->index = 0;
			top->parent = obj;
			if (ignore)
				ignore++;
			if (c == '[') {
				if (!ignore) {
					if (!json_object_is_type(obj, json_type_array))
						goto missfit;
					top->count = (int)json_object_array_length(obj);
				}
				xacc[0] = ']';
				acc = unpack_accept_arr;
			} else {
				if (!ignore) {
					if (!json_object_is_type(obj, json_type_object))
						goto missfit;
					top->count = json_object_object_length(obj);
				}
				xacc[0] = '}';
				acc = unpack_accept_key;
				continue;
			}
			break;
		case '}':
		case ']':
			if (c != xacc[0])
				goto internal_error;
			acc = top->acc;
			xacc[0] = top->type;
			top = top == stack ? NULL : top - 1;
			if (ignore)
				ignore--;
			break;
		case '!':
			if (*d != xacc[0] || top == NULL)
				goto invalid_character;
			if (!ignore && top->index != top->count)
				goto incomplete;
			/*@fallthrough@*/
		case '*':
			acc = xacc;
			continue;
		default:
			goto internal_error;
		}
		switch (xacc[0]) {
		case 0:
			if (top)
				goto internal_error;
			if (*d)
				goto invalid_character;
			return 0;
		case ']':
			if (!ignore) {
				key = strchr(unpack_accept_arr, *d);
				if (key && key >= unpack_accept_any) {
					if (top->index >= top->count)
						goto out_of_range;
					obj = json_object_array_get_idx(top->parent, (rp_jsonc_index_t)top->index++);
				}
			}
			break;
		case ':':
			acc = unpack_accept_key;
			xacc[0] = '}';
			if (ignore)
				ignore--;
			break;
		default:
			goto internal_error;
		}
	}
truncated:
	rc = rp_jsonc_error_truncated;
	goto error;
internal_error:
	rc = rp_jsonc_error_internal_error;
	goto error;
invalid_character:
	rc = rp_jsonc_error_invalid_character;
	goto error;
too_deep:
	rc = rp_jsonc_error_too_deep;
	goto error;
null_spec:
	rc = rp_jsonc_error_null_spec;
	goto error;
null_key:
	rc = rp_jsonc_error_null_key;
	goto error;
out_of_range:
	rc = rp_jsonc_error_out_of_range;
	goto error;
incomplete:
	rc = rp_jsonc_error_incomplete;
	goto error;
missfit:
	rc = rp_jsonc_error_missfit_type;
	goto errorfit;
key_not_found:
	rc = rp_jsonc_error_key_not_found;
	goto error;
errorfit:
	d = fit;
error:
	rc = rc | (int)((d - desc) << 4);
	return -rc;
}

int rp_jsonc_vcheck(struct json_object *object, const char *desc, va_list args)
{
	return vunpack(object, desc, args, 0);
}

int rp_jsonc_check(struct json_object *object, const char *desc, ...)
{
	int rc;
	va_list args;

	va_start(args, desc);
	rc = rp_jsonc_vcheck(object, desc, args);
	va_end(args);
	return rc;
}

int rp_jsonc_vmatch(struct json_object *object, const char *desc, va_list args)
{
	return !vunpack(object, desc, args, 0);
}

int rp_jsonc_match(struct json_object *object, const char *desc, ...)
{
	int rc;
	va_list args;

	va_start(args, desc);
	rc = rp_jsonc_vmatch(object, desc, args);
	va_end(args);
	return rc;
}

int rp_jsonc_vunpack(struct json_object *object, const char *desc, va_list args)
{
	return vunpack(object, desc, args, 1);
}

int rp_jsonc_unpack(struct json_object *object, const char *desc, ...)
{
	int rc;
	va_list args;

	va_start(args, desc);
	rc = vunpack(object, desc, args, 1);
	va_end(args);
	return rc;
}

/**
 * Call the callback for each item of the given object
 * The given callback receives 3 arguments:
 *  1. the closure
 *  2. the item
 *  3. the name of the item
 *
 * @param object   the object to iterate
 * @param callback the callback to call
 * @param closure  the closure for the callback
 */
static void object_for_all(struct json_object *object, void (*callback)(void*,struct json_object*,const char*), void *closure)
{
	struct json_object_iterator it = json_object_iter_begin(object);
	struct json_object_iterator end = json_object_iter_end(object);
	while (!json_object_iter_equal(&it, &end)) {
		callback(closure, json_object_iter_peek_value(&it), json_object_iter_peek_name(&it));
		json_object_iter_next(&it);
	}
}

/**
 * Call the callback for each item of the given array
 * The given callback receives 2 arguments:
 *  1. the closure
 *  2. the item
 *
 * @param object   the array to iterate
 * @param callback the callback to call
 * @param closure  the closure for the callback
 */
static void array_for_all(struct json_object *object, void (*callback)(void*,struct json_object*), void *closure)
{
	int n = (int)json_object_array_length(object);
	int i = 0;
	while(i < n)
		callback(closure, json_object_array_get_idx(object, (rp_jsonc_index_t)i++));
}

/* apply callback to items of an array or to it if not an object */
void rp_jsonc_optarray_for_all(struct json_object *object, void (*callback)(void*,struct json_object*), void *closure)
{
	if (json_object_is_type(object, json_type_array))
		array_for_all(object, callback, closure);
	else
		callback(closure, object);
}

/* apply callback to items of an array */
void rp_jsonc_array_for_all(struct json_object *object, void (*callback)(void*,struct json_object*), void *closure)
{
	if (json_object_is_type(object, json_type_array))
		array_for_all(object, callback, closure);
}

/* apply callback to items of an object */
void rp_jsonc_object_for_all(struct json_object *object, void (*callback)(void*,struct json_object*,const char*), void *closure)
{
	if (json_object_is_type(object, json_type_object))
		object_for_all(object, callback, closure);
}

/* apply callback to items of an object or to it if not an object */
void rp_jsonc_optobject_for_all(struct json_object *object, void (*callback)(void*,struct json_object*,const char*), void *closure)
{
	if (json_object_is_type(object, json_type_object))
		object_for_all(object, callback, closure);
	else
		callback(closure, object, NULL);
}

/* apply callback to items or object */
void rp_jsonc_for_all(struct json_object *object, void (*callback)(void*,struct json_object*,const char*), void *closure)
{
	if (!object)
		/* do nothing */;
	else if (json_object_is_type(object, json_type_object))
		object_for_all(object, callback, closure);
	else if (!json_object_is_type(object, json_type_array))
		callback(closure, object, NULL);
	else {
		int n = (int)json_object_array_length(object);
		int i = 0;
		while(i < n)
			callback(closure, json_object_array_get_idx(object, (rp_jsonc_index_t)i++), NULL);
	}
}

/**
 * Clones the 'object' for the depth 'subdepth'. The object 'object' is
 * duplicated and all its fields are cloned with the depth 'subdepth'.
 *
 * @param object the object to clone. MUST be an **object**.
 * @param subdepth the depth to use when cloning the fields of the object.
 *
 * @return the cloned object.
 */
static struct json_object *clone_object(struct json_object *object, int subdepth)
{
	struct json_object *r = json_object_new_object();
	struct json_object_iterator it = json_object_iter_begin(object);
	struct json_object_iterator end = json_object_iter_end(object);
	while (!json_object_iter_equal(&it, &end)) {
		json_object_object_add(r,
			json_object_iter_peek_name(&it),
			rp_jsonc_clone_depth(json_object_iter_peek_value(&it), subdepth));
		json_object_iter_next(&it);
	}
	return r;
}

/**
 * Clones the 'array' for the depth 'subdepth'. The array 'array' is
 * duplicated and all its fields are cloned with the depth 'subdepth'.
 *
 * @param array the array to clone. MUST be an **array**.
 * @param subdepth the depth to use when cloning the items of the array.
 *
 * @return the cloned array.
 */
static struct json_object *clone_array(struct json_object *array, int subdepth)
{
	int n = (int)json_object_array_length(array);
	struct json_object *r = json_object_new_array();
	while (n) {
		n--;
		json_object_array_put_idx(r, (rp_jsonc_index_t)n,
			rp_jsonc_clone_depth(json_object_array_get_idx(array, (rp_jsonc_index_t)n), subdepth));
	}
	return r;
}

/* clone with controled depth */
struct json_object *rp_jsonc_clone_depth(struct json_object *item, int depth)
{
	if (depth) {
		switch (json_object_get_type(item)) {
		case json_type_object:
			return clone_object(item, depth - 1);
		case json_type_array:
			return clone_array(item, depth - 1);
		default:
			break;
		}
	}
	return json_object_get(item);
}

/* clone first level */
struct json_object *rp_jsonc_clone(struct json_object *object)
{
	return rp_jsonc_clone_depth(object, 1);
}

/* clone entirely */
struct json_object *rp_jsonc_clone_deep(struct json_object *object)
{
	return rp_jsonc_clone_depth(object, INT_MAX);
}

/* add items of object added in dest */
struct json_object *rp_jsonc_object_add(struct json_object *dest, struct json_object *added)
{
	return rp_jsonc_object_merge(dest, added, rp_jsonc_merge_option_replace);
}

/* merge items of object 'merged' to the object 'dest' */
static void object_merge(struct json_object *dest, struct json_object *merged, int option)
{
	int exists, add;
	enum json_type tyto, tyfrom;
	struct json_object *to, *from;
	struct json_object_iterator it, end;

	/* iterate over elements of the merged object */
	it = json_object_iter_begin(merged);
	end = json_object_iter_end(merged);
	while (!json_object_iter_equal(&it, &end)) {
		from = json_object_iter_peek_value(&it);
		if (option == rp_jsonc_merge_option_replace) {
			/* always replace */
			add = 1;
		}
		else {
			/* check if dest has already an item of the name */
			exists = json_object_object_get_ex(dest, json_object_iter_peek_name(&it), &to);
			if (!exists) {
				/* add if not existing */
				add = 1;
			}
			else if (option == rp_jsonc_merge_option_keep) {
				/* no replacement */
				add = 0;
			}
			else {
				tyto = json_object_get_type(to);
				tyfrom = json_object_get_type(from);
				if (tyto == json_type_object && tyfrom == json_type_object) {
					/* recursive merge of objects */
					object_merge(to, from, option);
					add = 0;
				}
				else if (tyto == json_type_array && tyfrom == json_type_array) {
					/* append the array */
					rp_jsonc_array_insert_array(to, from, -1);
					add = 0;
				}
				else {
					/* fallback baheaviour */
					add = option & rp_jsonc_merge_option_replace;
				}
			}
		}

		/* add or replace the item if required */
		if (add) {
			json_object_object_add(dest,
				json_object_iter_peek_name(&it),
				json_object_get(from));
		}
		json_object_iter_next(&it);
	}
}

/* merge items of object 'merged' to the object 'dest' */
struct json_object *rp_jsonc_object_merge(struct json_object *dest, struct json_object *merged, int option)
{
	if (json_object_is_type(dest, json_type_object) && json_object_is_type(merged, json_type_object)) {
		object_merge(dest, merged, option);
	}
	return dest;
}

/* insert items of added in dest before position idx */
struct json_object *rp_jsonc_array_insert_array(struct json_object *dest, struct json_object *added, int idx)
{
	int i, nd, na;

	/* check th type */
	if (!json_object_is_type(dest, json_type_array) || !json_object_is_type(added, json_type_array))
		return dest;

	/* get lengths */
	nd = (int) json_object_array_length(dest);
	na = (int) json_object_array_length(added);

	/* handle case of negative indexes */
	if (idx < 0)
		idx = 1 + nd + idx;

	/* limit index to destination size */
	if (idx < 0)
		idx = 0;
	else if (idx > nd)
		idx = nd;

	/* move part of the array after insertion point */
	i = nd + na;
	while (i > idx + na) {
		i--;
		json_object_array_put_idx(dest,
			(rp_jsonc_index_t)i,
			json_object_get(json_object_array_get_idx(dest, (rp_jsonc_index_t)(i - na))));
	}
	/* copy the added items */
	while (i > idx) {
		i--;
		json_object_array_put_idx(dest,
			(rp_jsonc_index_t)i,
			json_object_get(json_object_array_get_idx(added, (rp_jsonc_index_t)(i - idx))));
	}
	return dest;
}

/* sort the array and return it */
struct json_object *rp_jsonc_sort(struct json_object *array)
{
	if (json_object_is_type(array, json_type_array))
		json_object_array_sort(array, (int(*)(const void*, const void*))rp_jsonc_cmp);

	return array;
}

/* return array of the sorted keys of the object */
struct json_object *rp_jsonc_keys(struct json_object *object)
{
	struct json_object *r;
	struct json_object_iterator it, end;
	if (!json_object_is_type(object, json_type_object))
		r = NULL;
	else {
		r = json_object_new_array();
		it = json_object_iter_begin(object);
		end = json_object_iter_end(object);
		while (!json_object_iter_equal(&it, &end)) {
			json_object_array_add(r, json_object_new_string(json_object_iter_peek_name(&it)));
			json_object_iter_next(&it);
		}
		r = rp_jsonc_sort(r);
	}
	return r;
}

/**
 * Internal comparison of 'x' with 'y'
 *
 * @param x first object to compare
 * @param y second object to compare
 * @param inc boolean true if should test for inclusion of y in x
 * @param sort boolean true if comparison used for sorting
 *
 * @return an integer indicating the computed result. Refer to
 * the table below for meaning of the returned value.
 *
 * inc | sort |  x < y  |  x == y  |  x > y  |  y in x
 * ----+------+---------+----------+---------+---------
 *  0  |  0   |  != 0   |     0    |  != 0   |   > 0
 *  0  |  1   |   < 0   |     0    |   > 0   |   > 0
 *  1  |  0   |  != 0   |     0    |  != 0   |    0
 *  1  |  1   |   < 0   |     0    |   > 0   |    0
 *
 *
 * if 'x' is found, respectively, to be less  than,  to match,
 * or be greater than 'y'. This is valid when 'sort'
 */
static int jcmp(struct json_object *x, struct json_object *y, int inc, int sort)
{
	double dx, dy;
	int64_t ix, iy;
	const char *sx, *sy;
	enum json_type tx, ty;
	int r, nx, ny, i;
	struct json_object_iterator it, end;
	struct json_object *jx, *jy;

	/* check equality of pointers */
	if (x == y)
		return 0;

	/* get the types */
	tx = json_object_get_type(x);
	ty = json_object_get_type(y);
	r = (int)tx - (int)ty;
	if (r)
		return r;

	/* compare following the type */
	switch (tx) {
	default:
	case json_type_null:
		break;

	case json_type_boolean:
		r = (int)json_object_get_boolean(x)
			- (int)json_object_get_boolean(y);
		break;

	case json_type_double:
		dx = json_object_get_double(x);
		dy = json_object_get_double(y);
		r =  dx < dy ? -1 : dx > dy;
		break;

	case json_type_int:
		ix = json_object_get_int64(x);
		iy = json_object_get_int64(y);
		r = ix < iy ? -1 : ix > iy;
		break;

	case json_type_object:
		it = json_object_iter_begin(y);
		end = json_object_iter_end(y);
		nx = json_object_object_length(x);
		ny = json_object_object_length(y);
		r = nx - ny;
		if (r > 0 && inc)
			r = 0;
		while (!r && !json_object_iter_equal(&it, &end)) {
			if (json_object_object_get_ex(x, json_object_iter_peek_name(&it), &jx)) {
				jy = json_object_iter_peek_value(&it);
				json_object_iter_next(&it);
				r = jcmp(jx, jy, inc, sort);
			} else if (sort) {
				jx = rp_jsonc_keys(x);
				jy = rp_jsonc_keys(y);
				r = rp_jsonc_cmp(jx, jy);
				json_object_put(jx);
				json_object_put(jy);
			} else
				r = 1;
		}
		break;

	case json_type_array:
		nx = (int)json_object_array_length(x);
		ny = (int)json_object_array_length(y);
		r = nx - ny;
		if (r > 0 && inc)
			r = 0;
		for (i = 0 ; !r && i < ny ; i++) {
			jx = json_object_array_get_idx(x, (rp_jsonc_index_t)i);
			jy = json_object_array_get_idx(y, (rp_jsonc_index_t)i);
			r = jcmp(jx, jy, inc, sort);
		}
		break;

	case json_type_string:
		sx = json_object_get_string(x);
		sy = json_object_get_string(y);
		r = strcmp(sx, sy);
		break;
	}
	return r;
}

/* compares 2 items */
int rp_jsonc_cmp(struct json_object *x, struct json_object *y)
{
	return jcmp(x, y, 0, 1);
}

/* test equallity of two items */
int rp_jsonc_equal(struct json_object *x, struct json_object *y)
{
	return !jcmp(x, y, 0, 0);
}

/* if x contains y */
int rp_jsonc_contains(struct json_object *x, struct json_object *y)
{
	return !jcmp(x, y, 1, 0);
}

/* get or creates the subobject of 'object' with 'key' */
int rp_jsonc_subobject(struct json_object *object, const char *key, struct json_object **subobject)
{
	json_object *obj = NULL;
	if (!json_object_object_get_ex(object, key, &obj)) {
		json_object_object_add(object, key, json_object_new_object());
		json_object_object_get_ex(object, key, &obj);
	}
	if (subobject != NULL)
		*subobject = obj;
	return json_object_is_type(obj, json_type_object);
}

/* add the 'value' to the 'object' with the 'key' */
int rp_jsonc_add(struct json_object *object, const char *key, struct json_object *value)
{
	if (value == NULL)
		return 0;
	json_object_object_add(object, key, value);
	return json_object_object_get_ex(object, key, NULL);
}

/* adds the 'string' to the 'object' with the 'key', but only if 'string' is not NULL */
int rp_jsonc_add_string(struct json_object *object, const char *key, const char *string)
{
	return string == NULL ? 0 : rp_jsonc_add(object, key, json_object_new_string(string));
}

/**********************************************************************/
/*                TESTING                                             */
/**********************************************************************/
#if defined(WRAP_JSON_TEST)
#include <stdio.h>
#if !defined(JSON_C_TO_STRING_NOSLASHESCAPE)
#define JSON_C_TO_STRING_NOSLASHESCAPE 0
#endif
#define j2t(o) json_object_to_json_string_ext((o), JSON_C_TO_STRING_NOSLASHESCAPE | JSON_C_TO_STRING_SPACED)

void tclone(struct json_object *object)
{
	struct json_object *o;

	o = rp_jsonc_clone(object);
	if (!rp_jsonc_equal(object, o))
		printf("ERROR in clone or equal: %s VERSUS %s\n", j2t(object), j2t(o));
	json_object_put(o);

	o = rp_jsonc_clone_deep(object);
	if (!rp_jsonc_equal(object, o))
		printf("ERROR in clone_deep or equal: %s VERSUS %s\n", j2t(object), j2t(o));
	json_object_put(o);
}

void p(const char *desc, ...)
{
	int rc;
	va_list args;
	struct json_object *result;

	va_start(args, desc);
	rc = rp_jsonc_vpack(&result, desc, args);
	va_end(args);
	if (!rc)
		printf("  SUCCESS %s\n\n", j2t(result));
	else
		printf("  ERROR[char %d err %d] %s\n\n", rp_jsonc_get_error_position(rc), rp_jsonc_get_error_code(rc), rp_jsonc_get_error_string(rc));
	tclone(result);
	json_object_put(result);
}

const char *xs[10];
int *xi[10];
int64_t *xI[10];
double *xf[10];
struct json_object *xo[10];
size_t xz[10];
uint8_t *xy[10];

void u(const char *value, const char *desc, ...)
{
	unsigned m, k;
	int rc;
	va_list args;
	struct json_object *object, *o;

	memset(xs, 0, sizeof xs);
	memset(xi, 0, sizeof xi);
	memset(xI, 0, sizeof xI);
	memset(xf, 0, sizeof xf);
	memset(xo, 0, sizeof xo);
	memset(xy, 0, sizeof xy);
	memset(xz, 0, sizeof xz);
	object = json_tokener_parse(value);
	va_start(args, desc);
	rc = rp_jsonc_vunpack(object, desc, args);
	va_end(args);
	if (rc)
		printf("  ERROR[char %d err %d] %s\n\n", rp_jsonc_get_error_position(rc), rp_jsonc_get_error_code(rc), rp_jsonc_get_error_string(rc));
	else {
		value = NULL;
		printf("  SUCCESS");
		va_start(args, desc);
		k = m = 0;
		while(*desc) {
			switch(*desc) {
			case '{': m = (m << 1) | 1; k = 1; break;
			case '}': m = m >> 1; k = m&1; break;
			case '[': m = m << 1; k = 0; break;
			case ']': m = m >> 1; k = m&1; break;
			case 's': printf(" s:%s", k ? va_arg(args, const char*) : *(va_arg(args, const char**)?:&value)); k ^= m&1; break;
			case '%': printf(" %%:%zu", *va_arg(args, size_t*)); k = m&1; break;
			case 'n': printf(" n"); k = m&1; break;
			case 'b': printf(" b:%d", *va_arg(args, int*)); k = m&1; break;
			case 'i': printf(" i:%d", *va_arg(args, int*)); k = m&1; break;
			case 'I': printf(" I:%lld", *va_arg(args, int64_t*)); k = m&1; break;
			case 'f': printf(" f:%f", *va_arg(args, double*)); k = m&1; break;
			case 'F': printf(" F:%f", *va_arg(args, double*)); k = m&1; break;
			case 'o': printf(" o:%s", j2t(*va_arg(args, struct json_object**))); k = m&1; break;
			case 'O': o = *va_arg(args, struct json_object**); printf(" O:%s", j2t(o)); json_object_put(o); k = m&1; break;
			case 'y':
			case 'Y': {
				uint8_t *p = *va_arg(args, uint8_t**);
				size_t s = *va_arg(args, size_t*);
				printf(" y/%d:%.*s", (int)s, (int)s, (char*)p);
				k = m&1;
				break;
				}
			default: break;
			}
			desc++;
		}
		va_end(args);
		printf("\n\n");
	}
	tclone(object);
	json_object_put(object);
}

void c(const char *sx, const char *sy, int e, int c)
{
	int re, rc;
	struct json_object *jx, *jy;

	jx = json_tokener_parse(sx);
	jy = json_tokener_parse(sy);

	re = rp_jsonc_cmp(jx, jy);
	rc = rp_jsonc_contains(jx, jy);

	printf("compare(%s)(%s)\n", sx, sy);
	printf("   -> %d / %d\n", re, rc);

	if (!re != !!e)
		printf("  ERROR should be %s\n", e ? "equal" : "different");
	if (!rc != !c)
		printf("  ERROR should %scontain\n", c ? "" : "not ");

	printf("\n");
}

#define P(...) do{ printf("pack(%s)\n",#__VA_ARGS__); p(__VA_ARGS__); } while(0)
#define U(...) do{ printf("unpack(%s)\n",#__VA_ARGS__); u(__VA_ARGS__); } while(0)

int main()
{
	char buffer[4] = {'t', 'e', 's', 't'};

	P("n");
	P("b", 1);
	P("b", 0);
	P("i", 1);
	P("I", (uint64_t)0x123456789abcdef);
	P("f", 3.14);
	P("s", "test");
	P("s?", "test");
	P("s?", NULL);
	P("s#", "test asdf", 4);
	P("s%", "test asdf", (size_t)4);
	P("s#", buffer, 4);
	P("s%", buffer, (size_t)4);
	P("s++", "te", "st", "ing");
	P("s+++++++++++++++",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
	);
	P("s#+#+", "test", 1, "test", 2, "test");
	P("s%+%+", "test", (size_t)1, "test", (size_t)2, "test");
	P("{}", 1.0);
	P("[]", 1.0);
	P("o", json_object_new_int(1));
	P("o?", json_object_new_int(1));
	P("o?", NULL);
	P("O", json_object_new_int(1));
	P("O?", json_object_new_int(1));
	P("O?", NULL);
	P("{s:[]}", "foo");
	P("{s+#+: []}", "foo", "barbar", 3, "baz");
	P("{s:s,s:o,s:O}", "a", NULL, "b", NULL, "c", NULL);
	P("{s:**}", "a", NULL);
	P("{s:s*,s:o*,s:O*}", "a", NULL, "b", NULL, "c", NULL);
	P("[i,i,i]", 0, 1, 2);
	P("[s,o,O]", NULL, NULL, NULL);
	P("[**]", NULL);
	P("[s*,o*,O*]", NULL, NULL, NULL);
	P(" s ", "test");
	P("[ ]");
	P("[ i , i,  i ] ", 1, 2, 3);
	P("{\n\n1");
	P("[}");
	P("{]");
	P("[");
	P("{");
	P("[i]a", 42);
	P("ia", 42);
	P("s", NULL);
	P("+", NULL);
	P(NULL);
	P("{s:i}", NULL, 1);
	P("{ {}: s }", "foo");
	P("{ s: {},  s:[ii{} }", "foo", "bar", 12, 13);
	P("[[[[[   [[[[[  [[[[ }]]]] ]]]] ]]]]]");
	P("y", "???????hello>>>>>>>", (size_t)19);
	P("Y", "???????hello>>>>>>>", (size_t)19);
	P("{sy?}", "foo", "hi", (size_t)2);
	P("{sy?}", "foo", NULL, 0);
	P("{sy*}", "foo", "hi", (size_t)2);
	P("{sy*}", "foo", NULL, 0);

	U("true", "b", &xi[0]);
	U("false", "b", &xi[0]);
	U("null", "n");
	U("42", "i", &xi[0]);
	U("123456789", "I", &xI[0]);
	U("3.14", "f", &xf[0]);
	U("12345", "F", &xf[0]);
	U("3.14", "F", &xf[0]);
	U("\"foo\"", "s", &xs[0]);
	U("\"foo\"", "s%", &xs[0], &xz[0]);
	U("{}", "{}");
	U("[]", "[]");
	U("{}", "o", &xo[0]);
	U("{}", "O", &xo[0]);
	U("{\"foo\":42}", "{si}", "foo", &xi[0]);
	U("[1,2,3]", "[i,i,i]", &xi[0], &xi[1], &xi[2]);
	U("{\"a\":1,\"b\":2,\"c\":3}", "{s:i, s:i, s:i}", "a", &xi[0], "b", &xi[1], "c", &xi[2]);
	U("42", "z");
	U("null", "[i]");
	U("[]", "[}");
	U("{}", "{]");
	U("[]", "[");
	U("{}", "{");
	U("[42]", "[i]a", &xi[0]);
	U("42", "ia", &xi[0]);
	U("[]", NULL);
	U("\"foo\"", "s", NULL);
	U("42", "s", NULL);
	U("42", "n");
	U("42", "b", NULL);
	U("42", "f", NULL);
	U("42", "[i]", NULL);
	U("42", "{si}", "foo", NULL);
	U("\"foo\"", "n");
	U("\"foo\"", "b", NULL);
	U("\"foo\"", "i", NULL);
	U("\"foo\"", "I", NULL);
	U("\"foo\"", "f", NULL);
	U("\"foo\"", "F", NULL);
	U("true", "s", NULL);
	U("true", "n");
	U("true", "i", NULL);
	U("true", "I", NULL);
	U("true", "f", NULL);
	U("true", "F", NULL);
	U("[42]", "[ii]", &xi[0], &xi[1]);
	U("{\"foo\":42}", "{si}", NULL, &xi[0]);
	U("{\"foo\":42}", "{si}", "baz", &xi[0]);
	U("[1,2,3]", "[iii!]", &xi[0], &xi[1], &xi[2]);
	U("[1,2,3]", "[ii!]", &xi[0], &xi[1]);
	U("[1,2,3]", "[ii]", &xi[0], &xi[1]);
	U("[1,2,3]", "[ii*]", &xi[0], &xi[1]);
	U("{\"foo\":42,\"baz\":45}", "{sisi}", "baz", &xi[0], "foo", &xi[1]);
	U("{\"foo\":42,\"baz\":45}", "{sisi*}", "baz", &xi[0], "foo", &xi[1]);
	U("{\"foo\":42,\"baz\":45}", "{sisi!}", "baz", &xi[0], "foo", &xi[1]);
	U("{\"foo\":42,\"baz\":45}", "{si}", "baz", &xi[0], "foo", &xi[1]);
	U("{\"foo\":42,\"baz\":45}", "{si*}", "baz", &xi[0], "foo", &xi[1]);
	U("{\"foo\":42,\"baz\":45}", "{si!}", "baz", &xi[0], "foo", &xi[1]);
	U("[1,{\"foo\":2,\"bar\":null},[3,4]]", "[i{sisn}[ii]]", &xi[0], "foo", &xi[1], "bar", &xi[2], &xi[3]);
	U("[1,2,3]", "[ii!i]", &xi[0], &xi[1], &xi[2]);
	U("[1,2,3]", "[ii*i]", &xi[0], &xi[1], &xi[2]);
	U("{\"foo\":1,\"bar\":2}", "{si!si}", "foo", &xi[1], "bar", &xi[2]);
	U("{\"foo\":1,\"bar\":2}", "{si*si}", "foo", &xi[1], "bar", &xi[2]);
	U("{\"foo\":{\"baz\":null,\"bar\":null}}", "{s{sn!}}", "foo", "bar");
	U("[[1,2,3]]", "[[ii!]]", &xi[0], &xi[1]);
	U("{}", "{s?i}", "foo", &xi[0]);
	U("{\"foo\":1}", "{s?i}", "foo", &xi[0]);
	U("{}", "{s?[ii]s?{s{si!}}}", "foo", &xi[0], &xi[1], "bar", "baz", "quux", &xi[2]);
	U("{\"foo\":[1,2]}", "{s?[ii]s?{s{si!}}}", "foo", &xi[0], &xi[1], "bar", "baz", "quux", &xi[2]);
	U("{\"bar\":{\"baz\":{\"quux\":15}}}", "{s?[ii]s?{s{si!}}}", "foo", &xi[0], &xi[1], "bar", "baz", "quux", &xi[2]);
	U("{\"foo\":{\"bar\":4}}", "{s?{s?i}}", "foo", "bar", &xi[0]);
	U("{\"foo\":{}}", "{s?{s?i}}", "foo", "bar", &xi[0]);
	U("{}", "{s?{s?i}}", "foo", "bar", &xi[0]);
	U("{\"foo\":42,\"baz\":45}", "{s?isi!}", "baz", &xi[0], "foo", &xi[1]);
	U("{\"foo\":42}", "{s?isi!}", "baz", &xi[0], "foo", &xi[1]);

	U("\"Pz8_Pz8_P2hlbGxvPj4-Pj4-Pg\"", "y", &xy[0], &xz[0]);
	U("\"\"", "y", &xy[0], &xz[0]);
	U("null", "y", &xy[0], &xz[0]);
	U("{\"foo\":\"Pz8_Pz8_P2hlbGxvPj4-Pj4-Pg\"}", "{s?y}", "foo", &xy[0], &xz[0]);
	U("{\"foo\":\"\"}", "{s?y}", "foo", &xy[0], &xz[0]);
	U("{}", "{s?y}", "foo", &xy[0], &xz[0]);
	U("{}", "!");
	U("{}", "{!}");
	U("{}", "{!}!");
	U("[]", "[!]");
	U("{}", "}");
	U("[]", "]");
	U("{}", "{}}");
	U("[]", "[]]");

	c("null", "null", 1, 1);
	c("true", "true", 1, 1);
	c("false", "false", 1, 1);
	c("1", "1", 1, 1);
	c("1.0", "1.0", 1, 1);
	c("\"\"", "\"\"", 1, 1);
	c("\"hi\"", "\"hi\"", 1, 1);
	c("{}", "{}", 1, 1);
	c("{\"a\":true,\"b\":false}", "{\"b\":false,\"a\":true}", 1, 1);
	c("[]", "[]", 1, 1);
	c("[1,true,null]", "[1,true,null]", 1, 1);

	c("null", "true", 0, 0);
	c("null", "false", 0, 0);
	c("0", "1", 0, 0);
	c("1", "0", 0, 0);
	c("0", "true", 0, 0);
	c("0", "false", 0, 0);
	c("0", "null", 0, 0);

	c("\"hi\"", "\"hello\"", 0, 0);
	c("\"hello\"", "\"hi\"", 0, 0);

	c("{}", "null", 0, 0);
	c("{}", "true", 0, 0);
	c("{}", "1", 0, 0);
	c("{}", "1.0", 0, 0);
	c("{}", "[]", 0, 0);
	c("{}", "\"x\"", 0, 0);

	c("[1,true,null]", "[1,true]", 0, 1);
	c("{\"a\":true,\"b\":false}", "{\"a\":true}", 0, 1);
	c("{\"a\":true,\"b\":false}", "{\"a\":true,\"c\":false}", 0, 0);
	c("{\"a\":true,\"c\":false}", "{\"a\":true,\"b\":false}", 0, 0);
	return 0;
}

#endif

#if 0


    /* Unpack the same item twice */
    j = json_pack("{s:s, s:i, s:b}", "foo", "bar", "baz", 42, "quux", 1);
    if(!json_unpack_ex(j, &error, 0, "{s:s,s:s!}", "foo", &s, "foo", &s))
        fail("json_unpack object with strict validation failed");
    {
        const char *possible_errors[] = {
            "2 object item(s) left unpacked: baz, quux",
            "2 object item(s) left unpacked: quux, baz"
        };
        check_errors(possible_errors, 2, "<validation>", 1, 10, 10);
    }
    json_decref(j);

#endif
