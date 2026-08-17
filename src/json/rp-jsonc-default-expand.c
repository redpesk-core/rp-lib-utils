/*
 * Copyright (C) 2015-2026 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
 *
 * $RP_BEGIN_LICENSE$
 * Commercial License Usage
 *  Licensees holding valid commercial IoT.bzh licenses may use this file in
 *  accordance with the commercial license agreement provided with the
 *  Software or, alternatively, in accordance with the terms contained in
 *  a written agreement between you and The IoT.bzh Company. For licensing terms
 *  and conditions see https://www.iot.bzh/terms-conditions. For further
 *  information use the contact form at https://www.iot.bzh/contact.
 *
 * GNU General Public License Usage
 *  Alternatively, this file may be used under the terms of the GNU General
 *  Public license version 3. This license is as published by the Free Software
 *  Foundation and appearing in the file LICENSE.GPLv3 included in the packaging
 *  of this file. Please review the following information to ensure the GNU
 *  General Public License requirements will be met
 *  https://www.gnu.org/licenses/gpl-3.0.html.
 * $RP_END_LICENSE$
 */

#include "rp-jsonc-default-expand.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>

#include <json-c/json.h>

#include "../misc/rp-expand-vars.h"
#include "../sys/rp-verbose.h"
#include "rp-jsonc.h"
#include "rp-jsonc-expand.h"
#include "rp-jsonc-path.h"

#define MERGEOPT rp_jsonc_merge_option_replace

/**
 * callback data for expanding references
 */
struct expref
{
	/** processing flags */
	int flags;

	/** root object */
	struct json_object *root;

	/** found error code */
	int error_code;

	/** target is the resulting object */
	struct json_object *target;

	/** callback for reading files */
	int (*readfunc)(void *closure, struct json_object **obj, const char *filename);

	/** closure of readfunc */
	void *closure;
};

/**
 * Emits an error for a given object of within path of ref
 *
 * @param object the object leading to an error
 * @param path   path to the expanded reference or NULL
 * @param format the message as in printf
 * @param ...    argument of the printf like message
 */
static
void
set_error(
	struct expref *expref,
	int code,
	struct json_object *object,
	const char *format,
	...
) {
	char *jpath;
	char *msg = NULL;
	va_list ap;
	int rc;

	/* string for the message */
	va_start(ap, format);
	rc = vasprintf(&msg, format, ap);
	va_end(ap);

	/* locating object */
	jpath = rp_jsonc_path(expref->root, object);

	/* emit the error */
	RP_ERROR("%s (json-path %s)", rc > 0 ? msg : "json expansion error", jpath ? jpath : "?");
	free(jpath);
	free(msg);

	expref->error_code = code;
	expref->flags = 0; /* stop further processings */
}

/*************************************************************************************
 * expansion of objects and references
 ************************************************************************************/

/**
 * Default read function
 */
static
int
default_readfunc(
	void *closure,
	struct json_object **obj,
	const char *filename
) {
	/*struct expref *expref = closure;*/
	return -!(*obj = json_object_from_file (filename));
}

/**
 * Called for each object referenced by "$ref", must be a string.
 * The string is then loaded.
 *
 * @param closure callback closure pointing a expref
 * @param the object referencing what to expand
 */
static void expand_ref(void *closure, struct json_object *object)
{
	struct expref *expref = closure;
	const char *string;
	struct json_object *obj;
	int rc;

	/* check type of object */
	if (!json_object_is_type(object, json_type_string))
		set_error(expref, -EINVAL, object, "$ref expects string");
	else {
		/* read the file */
		string = json_object_get_string(object);
		rc = expref->readfunc(expref->closure, &obj, string);
		if (rc < 0)
			set_error(expref, rc, object, "Reading of %s failed", string);
		else {
			if (!json_object_is_type(expref->target, json_type_object))
				expref->target = obj;
			else {
				rp_jsonc_object_merge(expref->target, obj, MERGEOPT);
				json_object_put(obj);
			}
		}
	}
}

/**
 * Check if the object is to be expanded
 * If yes the return its expansion, otherwise, returns the object
 * @see expand_json
 * $ref it accepted to be a string or an array of strings
 *
 * @param closure pointer to an integer for storing erreors
 * @param object  the object to expand
 * @param path    the path from root
 *
 * @return either the given object or its expansion
 */
static struct json_object *expand_object(void *closure, struct json_object* object, rp_jsonc_expand_path_t epath)
{
	struct expref *expref = closure;
	struct json_object *ref;

	/* if there is a "$ref" the object needs expansion */
	if ((expref->flags & RP_JSONEXP_$REFS) != 0
	 && json_object_object_get_ex(object, "$ref", &ref)) {
		expref->target = NULL;
		rp_jsonc_optarray_for_all(ref, expand_ref, expref);
		if (expref->error_code == 0)
			object = expref->target;
		else
			json_object_put(expref->target);
	}

	/* remove fields with NULL value */
	if ((expref->flags & RP_JSONEXP_DELETE_NULLS) != 0
	 && json_object_is_type(object, json_type_object)) {
		struct json_object_iterator it = json_object_iter_begin(object);
		struct json_object_iterator end = json_object_iter_end(object);
		while (!json_object_iter_equal(&it, &end)) {
			if (json_object_iter_peek_value(&it) != NULL)
				json_object_iter_next(&it);
			else {
				json_object_object_del(object, json_object_iter_peek_name(&it));
				it = json_object_iter_begin(object);
				end = json_object_iter_end(object);
			}
		}
	}

	return object;
}

/*************************************************************************************
 * expansion of strings
 ************************************************************************************/

/**
 * Auxiliary function setting *dest with obj and returning 1
 */
static
int
aux_set(
	struct json_object **dest,
	struct json_object *obj
) {
	*dest = obj;
	return 1;
}

/**
 * Auxiliary function for scanning integer
 */
static
int
aux_scan_int(
	const char *string,
	struct json_object **obj,
	const char *prefix,
	int base
) {
	long lval;
	char *end;

	/* test that string starts with prefix */
	while (*prefix)
		if (*prefix++ != (32 | *string++))
			return 0;

	/* extract a possible integer */
	errno = 0;
	lval = strtol(string, &end, base);
	if (*end == 0 && end != string && errno == 0
	 && lval <= INT32_MAX && lval >= INT32_MIN)
		return aux_set(obj, json_object_new_int((int32_t)lval));

	return 0;
}

/**
 * Auxiliary function for scanning double
 */
static int aux_scan_double(
		const char *string,
		struct json_object **obj
) {
	double dval;
	char *end;

	errno = 0;
	dval = strtod(string, &end);
	if (*end == 0 && end != string && errno == 0)
		return aux_set(obj, json_object_new_double_s(dval, string));
	return 0;
}

/**
 * Scan the string to see if it encode a value
 *
 * @return 0 if no value found, 1 otherwise with *obj set to the value
 * if no expansion has been done
 */
static
int
scan_string_value(
	const char *string,
	struct json_object **obj,
	int flags
) {
#define W(x) ((flags & RP_JSONEXP_SCAN_##x) != 0)
	/* try if null */
	if (W(NULL) && strcasecmp(string, "null") == 0)
		return aux_set(obj, NULL);

	/* try if true */
	if (W(TRUE) && strcasecmp(string, "true") == 0)
		return aux_set(obj, json_object_new_boolean(1));

	/* try if false */
	if (W(FALSE) && strcasecmp(string, "false") == 0)
		return aux_set(obj, json_object_new_boolean(0));

	return (W(HEXA)    && aux_scan_int(string, obj, "0x", 16))
	    || (W(BINARY)  && aux_scan_int(string, obj, "0b",  2))
	    || (W(OCTAL)   && aux_scan_int(string, obj, "0o",  8))
	    || (W(OCTAL)   && aux_scan_int(string, obj, "0",   8))
	    || (W(DECIMAL) && aux_scan_int(string, obj, "",   10))
	    || (W(DOUBLE)  && aux_scan_double(string, obj));
#undef W
}

/**
 * callback for expanding strings
 *
 * @param closure the closure
 * @param object the string object to be exanded
 * @param path path of the string to expand
 *
 * @return the object resulting of expanding the string or the given object
 * if no expansion has been done
 */
static
struct json_object *
expand_string(
	void *closure,
	struct json_object *object,
	rp_jsonc_expand_path_t epath
) {
	struct expref *expref = closure;
	char *subst = NULL;
	const char *value = json_object_get_string(object);

	/* expand the value with environment variables */
	if ((expref->flags & RP_JSONEXP_ENVVAR) != 0)
		subst = rp_expand_vars_env_only(value, 0);

	/*
	 * scan string for values
	 * this has to be done after expansion because it
	 * expansion produces strings possibly standing for values
	 */
	if (subst == NULL)
		scan_string_value(value, &object, expref->flags);
	else {
		if (!scan_string_value(subst, &object, expref->flags))
			object = json_object_new_string(subst);
		free(subst);
	}
	return object;
}

/*************************************************************************************
 * main entry for default expansion
 ************************************************************************************/
int
rp_jsonc_default_expanding(
	struct json_object **object,
	int (*readfunc)(void *closure, struct json_object **obj, const char *filename),
	void *closure,
	int flags
) {
	struct json_object *obj;
	struct expref expref;

	expref.flags = flags;
	expref.root = *object;
	if (readfunc != NULL) {
		expref.readfunc = readfunc;
		expref.closure = closure;
	}
	else {
		expref.readfunc = default_readfunc;
		expref.closure = &expref;
	}
	expref.error_code = 0;
	obj = rp_jsonc_expand(expref.root, &expref, expand_object, expand_string);
	if (obj != expref.root) {
		json_object_put(expref.root);
		*object = obj;
	}
	return expref.error_code;
}

