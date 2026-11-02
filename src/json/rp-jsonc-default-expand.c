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

#define _GNU_SOURCE /* for vasprintf */

#include "rp-jsonc-default-expand.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <json-c/json.h>

#include "../misc/rp-expand-vars.h"
#include "../sys/rp-verbose.h"
#include "rp-jsonc.h"
#include "rp-jsonc-expand.h"
#include "rp-jsonc-path.h"

#ifndef WITH_DIRENT
#define WITH_DIRENT 1
#endif

#define MERGEOPT rp_jsonc_merge_option_join_or_replace
#define DEPTHMAX 10

/**
 * callback data for expanding references
 */
struct expref
{
	/** found error code */
	int error_code;

	/** processing flags */
	int flags;

	/** nesting count */
	int nestcnt;

	/** root object */
	struct json_object *root;

	/** target filename */
	const char *filename;

	/** target is the resulting object */
	struct json_object *target;

	/** callback for reading files */
	int (*readfunc)(void *closure, struct json_object **obj, const char *filename);

	/** closure of readfunc */
	void *closure;
};

/**
 * for iterating over arrays
 */
struct forall
{
	struct expref *expref;
	rp_jsonc_path_t *path;
};

static int default_expand(struct expref *expref);

/**
 * Emits an error for a given object of within path of ref
 *
 * @param expref the object leading to an error
 * @param code   error code to set and return
 * @param path   path to the expanded reference or NULL
 * @param format the message as in printf
 * @param ...    argument of the printf like message
 */
static
int
set_error(
	struct expref *expref,
	int code,
	rp_jsonc_path_t *path,
	const char *format,
	...
) {
	char *jptr;
	char *msg = NULL;
	va_list ap;
	int rc;

	/* string for the message */
	va_start(ap, format);
	rc = vasprintf(&msg, format, ap);
	va_end(ap);

	/* JSON pointer of the error */
	jptr = path == NULL ? NULL : rp_jsonc_path_pointer(path);

	/* emit the error */
	RP_ERROR("%s (file %s, json-path %s)",
			rc > 0 ? msg : "json expansion error",
			expref->filename ? expref->filename : "?",
			jptr ? jptr : "?");
	free(jptr);
	free(msg);

	expref->flags = 0; /* stop further processings */
	expref->error_code = code;
	return code;
}

/*************************************************************************************
 * expansion of objects and references
 ************************************************************************************/

/**
 * File read function
 */
static
int
read_file(
	struct expref *expref,
	rp_jsonc_path_t *path,
	const char *filename,
	struct json_object **result
) {
	int rc;

	/* read the file */
	if (expref->readfunc != NULL)
		rc = expref->readfunc(expref->closure, result, filename);
	else
		rc = -!(*result = json_object_from_file(filename));
	if (rc >= 0)
		return 0;
	return set_error(expref, rc, path, "Reading of %s failed", filename);
}

/**
 * search the filename of the file and check if it exists
 */
static
const char *
search_path(
	struct expref *expref,
	char buffer[PATH_MAX],
	const char *filename
) {
	const char *lsl;
	int len;

	/* test relative filename */
	if (filename[0] != '/' && expref->filename != NULL) {
		lsl = strrchr(expref->filename, '/');
		if (lsl != NULL) {
			len = (int)(1 + (lsl - expref->filename));
			len = snprintf(buffer, PATH_MAX, "%.*s%s", len, expref->filename, filename);
			if (len >= 0 && len < PATH_MAX && access(buffer, R_OK) == 0)
				return buffer;
		}
	}

	/* test current access */
	if (access(filename, R_OK) == 0)
		return filename;

	/* not found */
	return NULL;
}

/**
 * Default read function
 */
static
int
read_file_ref(
	struct expref *expref,
	rp_jsonc_path_t *path,
	const char *filename
) {
	int rc;
	struct json_object *obj, *saved_root;
	char buffer[PATH_MAX];
	const char *newfilename, *saved_path;

	/* search the filename */
	newfilename = search_path(expref, buffer, filename);
	if (newfilename == NULL)
		return set_error(expref, -ENOENT, path, "Unable to locate file %s", filename);

	/* read the file */
	rc = read_file(expref, path, newfilename, &obj);
	if (rc < 0)
		return rc;

	/* expand the file read */
	saved_root = expref->root;
	saved_path = expref->filename;
	expref->root = obj;
	expref->filename = newfilename;
	rc = default_expand(expref);
	obj = expref->root;
	expref->root = saved_root;
	expref->filename = saved_path;
	if (rc < 0) {
		json_object_put(obj);
		return rc;
	}

	/* merge the readen content */
	if (!json_object_is_type(expref->target, json_type_object))
		expref->target = obj;
	else {
		rp_jsonc_object_merge(expref->target, obj, MERGEOPT);
		json_object_put(obj);
	}
	return 0;
}

/**
 * Read file or directory
 */
#if !WITH_DIRENT
#define read_any_ref read_file_ref
#else
#include <sys/types.h>
#include <dirent.h>
static
int
read_dir_ref(
	struct expref *expref,
	rp_jsonc_path_t *path,
	const char *dirname,
	DIR *dir
) {
	int status = 0;
	char filename[PATH_MAX];
	size_t lenent, lendir = strlen(dirname);
	struct dirent *ent;

	if (lendir >= sizeof filename)
		status = -1;
	else {
		memcpy(filename, dirname, lendir);
		if (filename[lendir - 1] != '/')
			filename[lendir++] = '/';
		while ((ent = readdir(dir)) != NULL) {
			/* skip directories */
			if (ent->d_type == DT_DIR)
				continue;

			/* make the filename */
			lenent = strlen(ent->d_name);
			if (lenent + lendir + 1 > sizeof filename) {
				status = -1;
				break;
			}
			memcpy(&filename[lendir], ent->d_name, 1 + lenent);

			/* read the file */
			status = read_file_ref(expref, path, filename);
			if (status < 0)
				break;
		}
	}
	closedir(dir);
	return status;
}

static
int
read_any_ref(
	struct expref *expref,
	rp_jsonc_path_t *path,
	const char *filename
) {
	if ((expref->flags & RP_JSONEXP_$REFS_DIR) != 0) {
		DIR *dir = opendir(filename);
		if (dir != NULL)
			return read_dir_ref(expref, path, filename, dir);
	}
	return read_file_ref(expref, path, filename);
}
#endif

/**
 * Called for each object referenced by "$ref", must be a string.
 * The string is then loaded.
 *
 * @param closure callback closure pointing a expref
 * @param the object referencing what to expand
 */
static void expand_ref(struct expref *expref, struct json_object *object, rp_jsonc_path_t *path)
{
	const char *string;

	/* check type of object */
	if (!json_object_is_type(object, json_type_string))
		set_error(expref, -EINVAL, path, "$ref expects string");
	else {
		/* read the file */
		string = json_object_get_string(object);
		read_any_ref(expref, path, string);
	}
}

/**
 */
static void array_expand_ref(void *closure, struct json_object *object)
{
	struct forall *forall = closure;
	expand_ref(forall->expref, object, forall->path);
}

/**
 * Check if the object is to be expanded
 * If yes returns its expansion, otherwise, returns the object
 * @see expand_json
 * $ref it accepted to be a string or an array of strings
 *
 * @param closure pointer to an integer for storing erreors
 * @param object  the object to expand
 * @param path    the path from root
 *
 * @return either the given object or its expansion
 */
static struct json_object *expand_object(void *closure, struct json_object* object, rp_jsonc_path_t *path)
{
	struct expref *expref = closure;
	struct json_object *ref;

	/* if there is a "$ref" the object needs expansion */
	if ((expref->flags & RP_JSONEXP_$REFS) != 0
	 && json_object_object_get_ex(object, "$ref", &ref)) {
		if (expref->nestcnt >= DEPTHMAX)
			set_error(expref, -ELOOP, path, "Max include depth reached");
		else {
			struct json_object *saved_target = expref->target;
			expref->nestcnt++;
			expref->target = NULL;
			if ((expref->flags & RP_JSONEXP_$REFS_MULTIPLE) == 0)
				expand_ref(expref, ref, path);
			else {
				struct forall forall = { expref, path };
				rp_jsonc_optarray_for_all(ref, array_expand_ref, &forall);
			}
			if (expref->error_code == 0)
				object = expref->target;
			else
				json_object_put(expref->target);
			expref->target = saved_target;
			expref->nestcnt--;
		}
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
 * @param path path of the string object to expand
 *
 * @return the object resulting of expanding the string or the given object
 * if no expansion has been done
 */
static
struct json_object *
expand_string(
	void *closure,
	struct json_object *object,
	rp_jsonc_path_t *path
) {
	struct expref *expref = closure;
	char *subst = NULL;
	const char *value = json_object_get_string(object);

	/* expand the value with environment variables */
	if ((expref->flags & RP_JSONEXP_ENVVAR) != 0)
		subst = rp_expand_vars_env_only(value, 0);

	/*
	 * scan string for values
	 * this has to be done after expansion because
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

static
int
default_expand(struct expref *expref)
{
	struct json_object *obj;

	obj = rp_jsonc_expand(expref->root, expref, expand_object, expand_string);
	if (obj != expref->root) {
		json_object_put(expref->root);
		expref->root = obj;
	}
	return expref->error_code;
}


/*************************************************************************************
 * main entry for default expansion
 ************************************************************************************/
int
rp_jsonc_default_expanding(
	struct json_object **object,
	const char *filename,
	int (*readfunc)(void *closure, struct json_object **obj, const char *filename),
	void *closure,
	int flags
) {
	struct expref expref;

	/* initialize the expander */
	expref.error_code = 0;
	expref.flags = flags;
	expref.nestcnt = 0;
	expref.root = *object;
	expref.filename = filename;
	expref.target = NULL;
	expref.readfunc = readfunc;
	expref.closure = closure;

	/* read the object if required */
	if (expref.root == NULL) {
		if (filename == NULL)
			return set_error(&expref, -EINVAL, NULL, "No object and no filename!");
		if (read_file(&expref, NULL, filename, &expref.root) < 0)
			return expref.error_code;
	}

	/* process */
	default_expand(&expref);
	*object = expref.root;
	return expref.error_code;
}

