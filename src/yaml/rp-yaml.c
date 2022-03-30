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

#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include <yaml.h>
#include <json-c/json.h>

#include "rp-yaml.h"
#include "../json/rp-jsonc-locator.h"

// it is safe to call yaml_event_delete more than one time

typedef struct y2j_s y2j_t;

struct y2j_s
{
	yaml_parser_t parser;
	yaml_event_t  event;
	json_object  *aliases;
	FILE         *file;
	const char   *buffer;
	size_t        readpos;
	size_t        size;
	rp_jsonc_locator_t *locator;
};

/**
 * reading call back from the root
 */
static
int
y2jt_read(void *data, unsigned char *buffer, size_t size, size_t *size_read)
{
	y2j_t *y2jt = data;
	int result;
	size_t read;

	if (y2jt->file != NULL) {
		read = fread(buffer, 1, size, y2jt->file);
		result = !ferror(y2jt->file);
	}
	else {
		read = y2jt->size - y2jt->readpos;
		if (read > size)
			read = size;
		memcpy(buffer, &y2jt->buffer[y2jt->readpos], read);
		y2jt->readpos += read;
		result = 1;
	}
	*size_read = read;
	return result;
}

/**
 * initialize the y2jt structure
 */
static
int
y2jt_init(y2j_t *y2jt, const char *name)
{
	int rc;
	if (name == NULL)
		y2jt->locator = NULL;
	else {
		rc = rp_jsonc_locator_begin(&y2jt->locator, name);
		if (rc < 0)
			return rc;
	}
	y2jt->aliases = json_object_new_object();
	if (y2jt->aliases == NULL
	|| !yaml_parser_initialize(&y2jt->parser)) {
		json_object_put(y2jt->aliases);
		if (y2jt->locator != NULL)
			rp_jsonc_locator_end(y2jt->locator);
		return -ENOMEM;
	}
	yaml_parser_set_input(&y2jt->parser, y2jt_read, y2jt);
	memset(&y2jt->event, 0, sizeof y2jt->event);
	return 0;
}

/**
 * initialize the y2jt structure for a file
 */
static
int
y2jt_init_file(y2j_t *y2jt, const char *name, FILE *file)
{
	y2jt->file = file;
	y2jt->buffer = NULL;
	y2jt->readpos = 0;
	y2jt->size = 0;
	return y2jt_init(y2jt, name);
}

/**
 * initialize the y2jt structure for a buffer
 */
static
int
y2jt_init_buffer(y2j_t *y2jt, const char *name, const char *buffer, size_t size)
{
	y2jt->file = NULL;
	y2jt->buffer = buffer;
	y2jt->readpos = 0;
	y2jt->size = size;
	return y2jt_init(y2jt, name);
}

/**
 * deinitialize the y2jt structure
 */
static
void
y2jt_deinit(y2j_t *y2jt)
{
	if (y2jt->locator != NULL)
		rp_jsonc_locator_end(y2jt->locator);
	yaml_event_delete(&y2jt->event);
	yaml_parser_delete(&y2jt->parser);
	json_object_put(y2jt->aliases);
}

/**
 * deinitialize the y2jt structure
 */
static
int
y2jt_parse(y2j_t *y2jt)
{
	yaml_event_delete(&y2jt->event);
	return yaml_parser_parse(&y2jt->parser, &y2jt->event) - 1;
}

/**
 * record the item in aliases
 */
static int
y2j_rec_anchor(y2j_t *y2jt, json_object *node, const char *anchor)
{
	int rc;
	if (anchor == NULL)
		rc = 0;
	else if (json_object_object_get_ex(y2jt->aliases, anchor, NULL))
		rc = -1;
	else {
		json_object_object_add_ex(y2jt->aliases, anchor, json_object_get(node), JSON_C_OBJECT_ADD_KEY_IS_NEW);
		rc = 0;
	}
	return rc;
}

/**
 * record the line
 */
static void
y2j_rec_line(y2j_t *y2jt, json_object *node, size_t line)
{
	if (y2jt->locator != NULL)
		rp_jsonc_locator_set_location(y2jt->locator, node, (unsigned)(line + 1));
}

static int
y2j_node(y2j_t *y2jt, json_object **node)
{
	int rc;
	char *text;
	json_object *value, *item;

	*node = NULL;
	switch (y2jt->event.type) {
	case YAML_ALIAS_EVENT:
		rc = -!json_object_object_get_ex(y2jt->aliases, (char*)y2jt->event.data.alias.anchor, &value);
		if (rc == 0) {
			y2j_rec_line(y2jt, value, y2jt->event.start_mark.line);
			*node = value;
			json_object_get(value);
		}
		break;

	case YAML_SCALAR_EVENT:
		text = y2jt->event.data.scalar.value;
		value = json_tokener_parse(text);
		if (value == NULL && strcmp(text, "null"))
			value = json_object_new_string(text);
		else
			json_object_set_serializer(value, NULL, NULL, NULL); /* unset userdata */
		*node = value;
		y2j_rec_line(y2jt, value, y2jt->event.start_mark.line);
		rc = y2j_rec_anchor(y2jt, value, y2jt->event.data.sequence_start.anchor);
		rc = 0;
		break;

	case YAML_SEQUENCE_START_EVENT:
		*node = value = json_object_new_array();
		y2j_rec_line(y2jt, value, y2jt->event.start_mark.line);
		rc = y2j_rec_anchor(y2jt, value, y2jt->event.data.sequence_start.anchor);
		while (rc == 0) {
			rc = y2jt_parse(y2jt);
			if (rc == 0) {
				if (y2jt->event.type == YAML_SEQUENCE_END_EVENT)
					break;
				rc = y2j_node(y2jt, &item);
				if (rc == 0)
					json_object_array_add(value, item);
			}
		}
		break;

	case YAML_MAPPING_START_EVENT:
		*node = value = json_object_new_object();
		y2j_rec_line(y2jt, value, y2jt->event.start_mark.line);
		rc = y2j_rec_anchor(y2jt, value, y2jt->event.data.mapping_start.anchor);
		while (rc == 0) {
			rc = y2jt_parse(y2jt);
			if (rc == 0) {
				if (y2jt->event.type == YAML_MAPPING_END_EVENT)
					break;
				if (y2jt->event.type != YAML_SCALAR_EVENT)
					rc = -1;
				else {
					text = strdup(y2jt->event.data.scalar.value);
					if (text == NULL)
						rc = -1;
					else {
						rc = y2jt_parse(y2jt);
						if (rc == 0) {
							rc = y2j_node(y2jt, &item);
							if (rc == 0)
								json_object_object_add(value, text, item);
						}
						free(text);
					}
				}
			}
		}
		break;

	default:
		rc = -1;
		break;
	}

	if (rc < 0 && *node != NULL) {
		json_object_put(*node);
		*node = NULL;
	}

	return rc;
}


static
int
y2j_root(y2j_t *y2jt, json_object **root)
{
	size_t written = 0;
	int count = 0;
	int rc;
	int k;

	rc = y2jt_parse(y2jt);
	if (rc == 0) {
		if (y2jt->event.type != YAML_STREAM_START_EVENT)
			rc = -1;
		else {
			rc = y2jt_parse(y2jt);
			if (rc == 0) {
				if (y2jt->event.type != YAML_DOCUMENT_START_EVENT)
					rc = -1;
				else {
					rc = y2jt_parse(y2jt);
					if (rc == 0)
						rc = y2j_node(y2jt, root);
				}
			}
		}
	}
	return rc;
}

int
rp_yaml_buffer_to_json_c(json_object **root, const char *buffer, size_t size, const char *name)
{
	y2j_t y2jt;
	int rc = y2jt_init_buffer(&y2jt, name, buffer, size);
	if (rc == 0) {
		rc = y2j_root(&y2jt, root);
		y2jt_deinit(&y2jt);
	}
	return rc;
}

int
rp_yaml_file_to_json_c(json_object **root, FILE *file, const char *name)
{
	y2j_t y2jt;
	int rc = y2jt_init_file(&y2jt, name, file);
	if (rc == 0) {
		rc = y2j_root(&y2jt, root);
		y2jt_deinit(&y2jt);
	}
	return rc;
}

int
rp_yaml_path_to_json_c(json_object **root, const char *path, const char *name)
{
	int rc;
	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		*root = NULL;
		rc = -errno;
	}
	else {
		rc = rp_yaml_file_to_json_c(root, file, name);
		fclose(file);
	}
	return rc;
}
