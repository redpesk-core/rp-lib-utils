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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>

/**
 * @brief parse a YAML buffer to a json-c structure optionaly recording
 * locations for @see rp_jsonc_locator_locate
 * 
 * @param root pointer where to store the result
 * @param buffer the buffer (UTF8)
 * @param size the size of the buffer
 * @param name if not NULL a name returned by @see rp_jsonc_locator_locate
 * @return 0 on success or a negative error code
 */
extern
int
rp_yaml_buffer_to_json_c(json_object **root, const char *buffer, size_t size, const char *name);

/**
 * @brief parse a YAML stream to a json-c structure optionaly recording
 * locations for @see rp_jsonc_locator_locate
 * 
 * @param root pointer where to store the result
 * @param file the file to parse (will not be closed)
 * @param name if not NULL a name returned by @see rp_jsonc_locator_locate
 * @return 0 on success or a negative error code
 */
extern
int
rp_yaml_file_to_json_c(json_object **root, FILE *file, const char *name);

/**
 * @brief parse a YAML file to a json-c structure optionaly recording
 * locations for @see rp_jsonc_locator_locate
 * 
 * @param root pointer where to store the result
 * @param path path of the file to parse
 * @param name if not NULL a name returned by @see rp_jsonc_locator_locate
 * @return 0 on success or a negative error code
 */
extern
int
rp_yaml_path_to_json_c(json_object **root, const char *path, const char *name);

#ifdef __cplusplus
}
#endif

