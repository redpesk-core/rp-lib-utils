/*
 * Copyright (C) 2015-2025 IoT.bzh Company
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
#define _GNU_SOURCE

#include "rp-curl.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "rp-escape.h"


/* internal representation of buffers */
struct buffer {
	size_t size;
	char *data;
};

static const char* curl_concatenate_args(const char * const *args, const char *sep, size_t *length)
{
	int i;
	size_t lq;
	const char *null;
	char *result, *front;

	/* ensure args */
	if (!args) {
		null = NULL;
		args = &null;
	}
	if (!sep)
		sep = "";

	/* compute lengths */
	lq = 0;
	i = 0;
	while (args[i]) {
		lq += strlen(args[i]);
		i++;
	}

	/* allocation */
	result = malloc(1 + lq + (i ? i - 1 : 0) * strlen(sep));
	if (result) {
		/* make the resulting args string contenated */
		i = 0;
		front = result;
		while (args[i]) {
			if (i) {
				front = stpcpy(front, sep);
			}
			front = stpcpy(front, args[i]);
			i++;
		}
		*front = 0;
		if (length)
			*length = (size_t)(front - result);
	}
	return result;
}

/* write callback for filling buffers with the response */
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	struct buffer *buffer = userdata;
	size_t sz = size * nmemb;
	size_t old_size = buffer->size;
	size_t new_size = old_size + sz;
	char *data = realloc(buffer->data, new_size + 1);
	if (!data)
		return 0;
	memcpy(&data[old_size], ptr, sz);
	data[new_size] = 0;
	buffer->size = new_size;
	buffer->data = data;
	return sz;
}

/*
 * Perform the CURL operation for 'curl' and put the result in
 * memory. If 'result' isn't NULL it receives the returned content
 * that then must be freed. If 'size' isn't NULL, it receives the
 * size of the returned content. Note that if not NULL, the real
 * content is one byte greater than the read size and the last byte
 * zero. This facility allows to handle the returned content as a
 * null terminated C-string.
 */
int rp_curl_perform(CURL *curl, char **result, size_t *size)
{
	int rc;
	struct buffer buffer;
	CURLcode code;

	/* init tthe buffer */
	buffer.size = 0;
	buffer.data = NULL;

	/* Perform the request, res will get the return code */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

	/* Perform the request, res will get the return code */
	code = curl_easy_perform(curl);
	rc = code == CURLE_OK;

	/* Check for no errors */
	if (rc) {
		/* no error */
		if (size)
			*size = buffer.size;
		if (result)
			*result = buffer.data;
		else
			free(buffer.data);
	} else {
		/* had error */
		if (size)
			*size = 0;
		if (result)
			*result = NULL;
		free(buffer.data);
	}

	return rc;
}

void rp_curl_do(CURL *curl, void (*callback)(void *closure, int status, CURL *curl, const char *result, size_t size), void *closure)
{
	int rc;
	char *result;
	size_t size;
	char errbuf[CURL_ERROR_SIZE];

	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
	rc = rp_curl_perform(curl, &result, &size);
	if (rc)
		callback(closure, rc, curl, result, size);
	else
		callback(closure, rc, curl, errbuf, 0);
	free(result);
	curl_easy_cleanup(curl);
}

int rp_curl_content_type_is(CURL *curl, const char *value)
{
	char *actual;
	CURLcode code;

	code = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &actual);
	if (code != CURLE_OK || !actual)
		return 0;

	return !strncasecmp(actual, value, strcspn(actual, "; "));
}

long rp_curl_response_code_get(CURL *curl)
{
	long rc;
	CURLcode code;

	code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rc);
	return (code == CURLE_OK) ? rc : 0;
}

CURL *rp_curl_prepare_get_url(const char *url)
{
	CURL *curl;
	CURLcode code;

	curl = curl_easy_init();
	if(curl) {
		code = curl_easy_setopt(curl, CURLOPT_URL, url);
		if (code == CURLE_OK)
			return curl;
		curl_easy_cleanup(curl);
	}
	return NULL;
}

CURL *rp_curl_prepare_get(const char *base, const char *path, const char * const *args)
{
	CURL *res;
	char *url;

	url = rp_escape_url(base, path, args, NULL);
	res = url ? rp_curl_prepare_get_url(url) : NULL;
	free(url);
	return res;
}

int rp_curl_add_header(CURL *curl, const char *header)
{
	int rc;
	struct curl_slist *list;

	list = curl_slist_append(NULL, header);
	rc = list ? curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list) == CURLE_OK : 0;
/*
	curl_slist_free_all(list);
*/
	return rc;
}

int rp_curl_add_header_value(CURL *curl, const char *name, const char *value)
{
	char *h;
	int rc;

	rc = asprintf(&h, "%s: %s", name, value);
	rc = rc < 0 ? 0 : rp_curl_add_header(curl, h);
	free(h);
	return rc;
}


CURL *rp_curl_prepare_post_url_data(const char *url, const char *datatype, const char *data, size_t szdata)
{
	CURL *curl;

	curl = curl_easy_init();
	if (curl
	&& CURLE_OK == curl_easy_setopt(curl, CURLOPT_URL, url)
	&& (!szdata || CURLE_OK == curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, szdata))
	&& CURLE_OK == curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data)
	&& (!datatype || rp_curl_add_header_value(curl, "content-type", datatype)))
		return curl;
	curl_easy_cleanup(curl);
	return NULL;
}

static CURL *rp_curl_prepare_post(const char *base, const char *path, int unescape_flag, const char *separator, const char * const *args, const char *simple_args)
{
	CURL *res;
	char *url;
	const char *data = NULL;
	size_t szdata = 0;

	url = rp_escape_url(base, path, NULL, NULL);
	if(args) {
		data = unescape_flag ?
			curl_concatenate_args(args, separator, &szdata) :
			rp_escape_args(args, &szdata);
	}
	else {
		data = unescape_flag ?
			rp_escape_str(simple_args, &szdata) :
			simple_args;
	}
	szdata = szdata ? szdata : strlen(data);

	res = url ? rp_curl_prepare_post_url_data(url, NULL, data, szdata) : NULL;
	free(url);
	return res;
}

CURL *rp_curl_prepare_post_simple_unescaped(const char *base, const char *path, const char *args)
{
	return rp_curl_prepare_post(base, path, 1, NULL, NULL, args);
}

CURL *rp_curl_prepare_post_simple_escaped(const char *base, const char *path, char *args)
{
	return rp_curl_prepare_post(base, path, 0, NULL, NULL, args);
}

CURL *rp_curl_prepare_post_unescaped(const char *base, const char *path, const char *separator, const char * const *args)
{
	return rp_curl_prepare_post(base, path, 1, separator, args, NULL);
}

CURL *rp_curl_prepare_post_escaped(const char *base, const char *path, const char * const *args)
{
	return rp_curl_prepare_post(base, path, 0, NULL, args, NULL);
}

/* vim: set colorcolumn=80: */
