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
 *
 */

#pragma once

#include <curl/curl.h>

#ifdef	__cplusplus
extern "C" {
#endif

extern int rp_curl_perform (CURL * curl, char **result, size_t * size);

extern void rp_curl_do(CURL *curl, void (*callback)(void *closure, int status, CURL *curl, const char *result, size_t size), void *closure);

extern int rp_curl_content_type_is (CURL * curl, const char *value);

extern long rp_curl_response_code_get(CURL *curl);

extern CURL *rp_curl_prepare_get_url(const char *url);

extern CURL *rp_curl_prepare_get(const char *base, const char *path, const char * const *args);

extern CURL *rp_curl_prepare_post_url_data(const char *url, const char *datatype, const char *data, size_t szdata);

extern int rp_curl_add_header(CURL *curl, const char *header);

extern int rp_curl_add_header_value(CURL *curl, const char *name, const char *value);

extern CURL *rp_curl_prepare_post_simple_unescaped(const char *base, const char *path, const char *args);

extern CURL *rp_curl_prepare_post_simple_escaped(const char *base, const char *path, char *args);

extern CURL *rp_curl_prepare_post_unescaped(const char *base, const char *path, const char *separator, const char * const *args);

extern CURL *rp_curl_prepare_post_escaped(const char *base, const char *path, const char * const *args);

#ifdef	__cplusplus
}
#endif
