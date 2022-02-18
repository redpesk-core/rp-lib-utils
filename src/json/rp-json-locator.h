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

#pragma once

#include <json-c/json.h>

/**
 * Extract the json object of the file given by 'filename' and put it in *jso.
 * In the same time, tag the created object with their line in the file
 * in a such way that the function @see rp_json_locator_locate is able to
 * return a valid indication
 *
 * @param jso      where to put the result if return is 0
 * @param filename the file to be loaded
 *
 * @return 0 in case of success or the negative value of detected errno
 */
extern int rp_json_locator_from_file(struct json_object **jso, const char *filename);

/**
 * Returns the location if available of the json object jso.
 * The location is possible only if the object jso was generated by
 * th function @see rp_json_locator_from_file
 *
 * @param jso the json object to locate
 * @param linenum the pointer for storing, if not NULL, the line number
 *
 * @return the file of the object if any or NULL if not found
 */
extern const char *rp_json_locator_locate(struct json_object *jso, unsigned *linenum);

/**
 * Copies, if it exists, the locator of the object 'from' to the object 'to'
 *
 * @param from the from object
 * @param to   the to object
 */
extern void rp_json_locator_copy(struct json_object *from, struct json_object *to);

/**
 * Computes the path location of jso within root. The returned path is a string
 * that must be freed by the caller. NULL is returned if jso is not part of root.
 *
 * @param root the root object possibly containing jso
 * @param jso the object whose path within root is queried
 *
 * @return NULL if jso is not part of root or a string that must be freed using 'free'
 */
extern char *rp_json_locator_search_path(struct json_object *root, struct json_object *jso);
