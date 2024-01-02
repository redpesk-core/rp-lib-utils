/*
 * Copyright (C) 2015-2024 IoT.bzh Company
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
 * opaque type for location objects
 */
typedef struct rp_jsonc_locator_s rp_jsonc_locator_t;

/**
 * Create an instance of locator for the given name
 *
 * @param locator the locator to be created
 * @param name    the name to give to located object (filename for example)
 *
 * @return 0 in case of success or the negative value of detected errno
 */
extern int rp_jsonc_locator_begin(rp_jsonc_locator_t **locator, const char *name);

/**
 * Terminate an instance of locator
 *
 * @param locator the locator to be terminated
 */
extern void rp_jsonc_locator_end(rp_jsonc_locator_t *locator);

/**
 * Set the line location of an object within a locator
 * @see rp_jsonc_locator_locate
 *
 * @param locator the main locator of the object
 * @param jso the json object to wich give location
 * @param linenum the line number to be set
 *
 * @return 0 in case of success or the negative value of detected errno
 */
extern int rp_jsonc_locator_set_location(rp_jsonc_locator_t *locator, struct json_object *jso, unsigned linenum);

/**
 * Returns the location if available of the json object jso.
 * The location is possible only if the object jso was generated by
 * th function @see rp_jsonc_locator_from_file and @see rp_jsonc_locator_set_location
 *
 * @param jso the json object to locate
 * @param linenum the pointer for storing, if not NULL, the line number
 *
 * @return the file of the object if any or NULL if not found
 */
extern const char *rp_jsonc_locator_locate(struct json_object *jso, unsigned *linenum);

/**
 * Copies, if it exists, the locator of the object 'from' to the object 'to'
 *
 * @param from the from object
 * @param to   the to object
 */
extern void rp_jsonc_locator_copy(struct json_object *from, struct json_object *to);

/**
 * Extract the json object of the file given by 'filename' and put it in *jso.
 * In the same time, tag the created object with their line in the file
 * in a such way that the function @see rp_jsonc_locator_locate is able to
 * return a valid indication
 *
 * @param jso      where to put the result if return is 0
 * @param filename the file to be loaded
 *
 * @return 0 in case of success or the negative value of detected errno
 */
extern int rp_jsonc_locator_from_file(struct json_object **jso, const char *filename);
