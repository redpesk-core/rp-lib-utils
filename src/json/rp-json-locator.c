/*
 * Copyright (C) 2015-2022 IoT.bzh Company
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

#include "rp-json-locator.h"

#if JSON_C_MINOR_VERSION < 13 /************* DONT IMPLEMENT LOCATOR *********/

int rp_json_locator_begin(rp_json_locator_t **locator, const char *name)
{
	*locator = NULL;
	return 0;
}

void rp_json_locator_end(rp_json_locator_t *locator)
{
}

int rp_json_locator_set_location(rp_json_locator_t *locator, unsigned *linenum, struct json_object *jso)
{
	return 0;
}

const char *rp_json_locator_locate(struct json_object *jso, unsigned *linenum)
{
	return NULL;
}

void rp_json_locator_copy(struct json_object *from, struct json_object *to)
{
}

#else /************* IMPLEMENT LOCATOR *************************/

/**
 * Records the line for a file
 * This record exists in two modes:
 *  - file: with line == 0 and name defined to the filename
 *  - line: with line > 0 and name0 invalid
 */
struct tagline
{
	/** an other tagline or itself if line == 0 */
	struct tagline *other;

	/** number of the line or 0 if filename */
	unsigned line;

	/** reference count of the structure */
	unsigned refcount;

	/** the filename if line == 0, invalid otherwise */
	char name[];
};

/** structure for creation of locators */
struct rp_json_locator_s
{
	struct tagline *root;
	struct tagline *last;
};

/** remove a refence count for the tag */
static void tag_unref(struct tagline *tag)
{
	if (!--tag->refcount) {
		if (tag->other != NULL && tag->other != tag)
			tag_unref(tag->other);
		free(tag);
	}
}

/** check that the pointer looks like a tagline */
static struct tagline *tagline_check(struct tagline *tagline)
{
	struct tagline *tagfile;
	if (tagline != NULL && tagline->line > 0 && tagline->refcount > 0) {
		tagfile = tagline->other;
		if (tagfile != NULL && tagfile->line == 0 && tagfile->refcount > 0 && tagfile->other == tagfile)
			return tagfile;
	}
	return NULL;
}

/**
 * Callback deleter function for json's userdata
 *
 * @param jso the json object whose user data is released
 * @param userdata the userdata to release
 */
static void untag_object(struct json_object *jso, void *userdata)
{
	tag_unref((struct tagline *)userdata);
}

/** create a tagging context */
int rp_json_locator_begin(rp_json_locator_t **result, const char *name)
{
	struct tagline *root;
	rp_json_locator_t *locator;

	*result = locator = malloc(sizeof *locator);
	if (locator != NULL) {
		root = malloc(sizeof *root + 1 + strlen(name));
		if (root != NULL) {
			root->other = root;
			locator->root = root;
			locator->last = root;
			root->refcount = 1;
			root->line = 0;
			strcpy(root->name, name);
			return 0;
		}
		free(locator);
		*result = NULL;
	}
	return -ENOMEM;
}

/** destroy a tagging context */
void rp_json_locator_end(rp_json_locator_t *locator)
{
	tag_unref(locator->root);
	free(locator);
}

/** set the location of jso */
int rp_json_locator_set_location(rp_json_locator_t *locator, struct json_object *jso, unsigned line)
{
	struct tagline *item;

	/* check validity */
	if (line == 0 || jso == NULL)
		return -EINVAL;

	/* optimized if same line that previous */
	if (locator->last->line == line) {
		item = locator->last;
		item->refcount++;
	}
	else {
		/* not found, create */
		item = malloc(sizeof *item);
		if (item == NULL)
			return -ENOMEM;

		/* init */
		locator->root->refcount++;
		item->other = locator->root;
		item->refcount = 1;
		item->line = line;
		locator->last = item;
	}
	/* set */
	json_object_set_userdata(jso, item, untag_object);
	return 0;
}

/* return the file and the line of the object jso */
const char *rp_json_locator_locate(struct json_object *jso, unsigned *linenum)
{
	struct tagline *tagfile, *tagline;
	const char *result;
	unsigned line;

	result = NULL;
	line = 0;

	/* read the userata and check it looks like a locator */
	if (jso != NULL) {
		tagline = json_object_get_userdata(jso);
		tagfile = tagline_check(tagline);
		if (tagfile != NULL) {
			/* yes probably, use it */
			line = tagline->line;
			result = tagfile->name;
		}
	}
	if (linenum != NULL)
		*linenum = line;
	return result;
}

/* copy the locator */
void rp_json_locator_copy(struct json_object *from, struct json_object *to)
{
	struct tagline *tagfile, *tagline;

	/* read the userata and check it looks like a locator */
	if (from != NULL && to != NULL) {
		tagline = json_object_get_userdata(from);
		tagfile = tagline_check(tagline);
		if (tagfile != NULL) {
			/* yes probably, use it */
			tagline->refcount++;
			json_object_set_userdata(to, tagline, untag_object);
		}
	}
}

#endif


#if JSON_C_MINOR_VERSION < 13 || __GLIBC_PREREQ(2,34)
 /************* DONT IMPLEMENT LOCATOR *********/

int rp_json_locator_from_file(struct json_object **jso, const char *filename)
{
	*jso = json_object_from_file(filename);
	return *jso ? 0 : -ENOMEM;
}

#else

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define COUNT 2000

struct block
{
	uintptr_t begin;
	uintptr_t end;
	void *tag;
};

struct group
{
	int top;
	struct group *next;
	struct block blocks[COUNT];
};

static struct group *groups_head = NULL;
static void *hooktag;

static struct block *getblocktag(void *ptr, struct group **group)
{
	int idx;
	struct group *grp;
	struct block *blk;

	for (grp = groups_head ; grp ; grp = grp->next) {
		blk = grp->blocks;
		for (idx = grp->top ; idx ; idx--, blk++) {
			if ((uintptr_t)ptr >= blk->begin && (uintptr_t)ptr < blk->end) {
				*group = grp;
				return blk;
			}
		}
	}
	return NULL;
}

static void *searchtag(void *ptr)
{
	struct group *group;
	struct block *block = getblocktag(ptr, &group);

	return block ? block->tag : NULL;
}

static void cleartags()
{
	struct group *group;
	while ((group = groups_head) != NULL) {
		groups_head = group->next;
		free(group);
	}
}

static void deltag(void *ptr)
{
	struct group *group;
	struct block *block = getblocktag(ptr, &group);

	if (block)
		*block = group->blocks[--group->top];
}

static void addtag(void *ptr, size_t size, void *(*alloc)(size_t))
{
	struct block *block;
	struct group *group;

	group = groups_head;
	if (group && group->top >= (int)(sizeof group->blocks / sizeof *group->blocks))
		group = NULL;
	if (group == NULL) {
		group = alloc(sizeof *group);
		if (group != NULL) {
			group->top = 0;
			group->next = groups_head;
			groups_head = group;
		}
	}
	if (group != NULL) {
		block = &group->blocks[group->top++];
		block->begin = (uintptr_t)ptr;
		block->end = (uintptr_t)ptr + (uintptr_t)size;
		block->tag = hooktag;
	}
}

static void *(*memo_malloc)(size_t size, const void *caller);
static void *(*memo_realloc)(void *ptr, size_t size, const void *caller);
static void *(*memo_memalign)(size_t alignment, size_t size, const void *caller);
static void  (*memo_free)(void *ptr, const void *caller);

static void *my_malloc(size_t size, const void *caller);
static void *my_realloc(void *ptr, size_t size, const void *caller);
static void *my_memalign(size_t alignment, size_t size, const void *caller);
static void  my_free(void *ptr, const void *caller);

static void memorize_hooks()
{
	memo_malloc = __malloc_hook;
	memo_realloc = __realloc_hook;
	memo_memalign = __memalign_hook;
	memo_free = __free_hook;
}

static void restore_hooks()
{
	__malloc_hook = memo_malloc;
	__realloc_hook = memo_realloc;
	__memalign_hook = memo_memalign;
	__free_hook = memo_free;
}

static void set_my_hooks()
{
	__malloc_hook = my_malloc;
	__realloc_hook = my_realloc;
	__memalign_hook = my_memalign;
	__free_hook = my_free;
}

static void *my_malloc(size_t size, const void *caller)
{
	void *result;

	restore_hooks();
	result = malloc(size);
	if (result)
		addtag(result, size, malloc);
	set_my_hooks();
	return result;
}

static void *my_realloc(void *ptr, size_t size, const void *caller)
{
	void *result;

	restore_hooks();
	if (ptr)
		deltag(ptr);
	result = realloc(ptr, size);
	if (result)
		addtag(result, size, malloc);
	else if (ptr)
		addtag(ptr, size, malloc);
	set_my_hooks();
	return result;
}

static void *my_memalign(size_t alignment, size_t size, const void *caller)
{
	void *result;

	restore_hooks();
	result = memalign(alignment, size);
	if (result)
		addtag(result, size, malloc);
	set_my_hooks();
	return result;
}
static void  my_free(void *ptr, const void *caller)
{
	deltag(ptr);
}

static void hook_off()
{
	restore_hooks();
	hooktag = NULL;
}

static void hook_on(void *tag)
{
	hooktag = tag;
	memorize_hooks();
	set_my_hooks();
}

static void hook_lino(unsigned lino)
{
	hook_on((void*)(intptr_t)lino);
}

/**
 * Tag the objects with their recorded line recursively
 *
 * @param jso the object to tag
 */


static void tag_objects(rp_json_locator_t *locator, struct json_object *jso)
{
#if JSON_C_VERSION_NUM >= 0x000d00
	size_t idx, len;
#else
	int idx, len;
#endif
	struct json_object_iterator it, end;
	void *tag;

	/* nothing to do for nulls */
	if (jso == NULL)
		return;

	/* search the object in tagged blocks */
	tag = searchtag(jso);
	if (tag != NULL)
		rp_json_locator_set_location(locator, jso, (unsigned)(intptr_t)tag);

	/* inspect type of the jso */
	switch (json_object_get_type(jso)) {
	case json_type_object:
		it = json_object_iter_begin(jso);
		end = json_object_iter_end(jso);
		while (!json_object_iter_equal(&it, &end)) {
			tag_objects(locator, json_object_iter_peek_value(&it));
			json_object_iter_next(&it);
		}
		break;
	case json_type_array:
		len = json_object_array_length(jso);
		for (idx = 0 ; idx < len ; idx++) {
			tag_objects(locator, json_object_array_get_idx(jso, idx));
		}
		break;
	default:
		break;
	}
}

/**
 * Reads the json file of filename and returns a json object whose userdata
 * records the line and file
 *
 * @param object where to store the read object if returning 0
 * @param filename filename of the file
 * @param file the file to read
 *
 * @return 0 in case of success or -errno
 */
static int get_from_file(struct json_object **object, const char *filename, FILE *file)
{
	int rc;
	int length;
	int stop;
	unsigned linum;
	char *line;
	size_t linesz;
	ssize_t linelen;
	json_tokener *tok;
	struct json_object *obj;
	rp_json_locator_t *locator;

	/* create the tokenizer */
	tok = json_tokener_new_ex(JSON_TOKENER_DEFAULT_DEPTH);
	if (tok == NULL) {
		rc = -ENOMEM;
		goto end;
	}

	/* create the file tag */
	rc = rp_json_locator_begin(&locator, filename);
	if (rc < 0)
		goto end2;

	/* read lines */
	line = NULL;
	linesz = 0;
	stop = 0;
	obj = NULL;
	for (linum = 1 ; !stop ; linum++) {
		/* read one line */
		linelen = getline(&line, &linesz, file);
		if (linelen < 0) {
			if (!feof(file))
				rc = -errno;
			else {
				hook_lino(linum);
				obj = json_tokener_parse_ex(tok, "", 1);
				hook_off();
				rc = obj != NULL ? 0 : -EBADMSG;
			}
			stop = 1;
		}
		else {
			/* scan the line */
			while (!stop && linelen > 0) {
				length = linelen > INT_MAX ? INT_MAX : (int)linelen;
				linelen -= (ssize_t)length;
				hook_lino(linum);
				obj = json_tokener_parse_ex(tok, line, length);
				hook_off();
				if (obj != NULL) {
					/* tokenizer returned an object */
					rc = 0;
					stop = 1;
				}
				else if (json_tokener_get_error(tok) != json_tokener_continue) {
					/* tokenizer has an error */
					rc = -EBADMSG;
					stop = 1;
				}
			}
		}
	}
	free(line);
	if (rc >= 0) {
		/* tag the created objects */
		tag_objects(locator, obj);

		/* record the result */
		*object = obj;
	}
	rp_json_locator_end(locator);
end2:
	json_tokener_free(tok);
end:
	cleartags();
	return rc;
}

/* parse the file of filename and make its json object representation */
int rp_json_locator_from_file(struct json_object **object, const char *filename)
{
	FILE *file;
	int rc;

	/* open */
	*object = NULL;
	file = fopen(filename, "r");
	if (file == NULL)
		rc = -errno;
	else {
		/* read */
		rc = get_from_file(object, filename, file);
		fclose(file);
	}
	return rc;
}

#endif /************* IMPLEMENT LOCATOR *************************/

