/*
 Copyright (C) 2015-2026 IoT.bzh Company

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

#include "rp-str2int.h"

/* get an integer from the string value */
int rp_str2u64(const char *str, uint64_t *value)
{
	char c;
	unsigned digit, base;
	uint64_t max, val;
	int rc;

	/* accept a sign mark */
	c = *str;
	if (c == '-') {
		rc = -1;
		c = *++str;
	}
	else {
		rc = 1;
		if (c == '+')
			c = *++str;
	}

	/* compute the base and advance to start content */
	if (c != '0')
		base = 10;
	else {
		c = *++str;
		switch(c | 32) {
		case 'b': base = 2; break;
		case 'o': base = 8; break;
		case 'd': base = 10; break;
		case 'x': base = 16; break;
		default: base = 8; str--; break;
		}
		c = *++str;
	}

	/* avoid empty string */
	if (c == '\0')
		return 0;

	/* compute the value */
	max = UINT64_MAX / base;
	val = 0;
	for(; c != '\0' ; c = *++str) {
		/* translate current character to digit value */
		if (c == '_')
			continue;
		if ('A' <= c && c <= 'Z')
			digit = (unsigned)(c - 'A' + 10);
		else if ('a' <= c && c <= 'z')
			digit = (unsigned)(c - 'a' + 10);
		else if ('0' <= c && c <= '9')
			digit = (unsigned)(c - '0');
		else
			break;
		if (digit >= base)
			break;
		if (val > max)
			break;
		val = (val * base) + (uint64_t)digit;
	}
	if (c != '\0')
		return 0;
	*value = val;
	return rc;
}

int rp_str2uint64(const char *str, uint64_t *value)
{
	int rc = rp_str2u64(str, value);
	return rc > 0;
}

int rp_str2int64(const char *str, int64_t *value)
{
	uint64_t u64;
	int rc = rp_str2u64(str, &u64);

	if (rc < 0) {
		if (u64 > -(uint64_t)INT64_MIN)
			rc = 0;
		else {
			*value = -(int64_t)u64;
			rc = 1;
		}
	}
	else if (rc > 0) {
		if (u64 > INT64_MAX)
			rc = 0;
		else
			*value = (int64_t)u64;
	}
	return rc;
	return rc;
}

int rp_str2int32(const char *str, int32_t *value)
{
	uint64_t u64;
	int rc = rp_str2u64(str, &u64);

	if (rc < 0) {
		if (u64 > -(uint32_t)INT32_MIN)
			rc = 0;
		else {
			*value = -(int32_t)u64;
			rc = 1;
		}
	}
	else if (rc > 0) {
		if (u64 > INT32_MAX)
			rc = 0;
		else
			*value = (int32_t)u64;
	}
	return rc;
}

int rp_str2uint32(const char *str, uint32_t *value)
{
	uint64_t u64;
	int rc = rp_str2u64(str, &u64);

	if (rc > 0 && u64 <= UINT32_MAX)
		*value = (uint32_t)u64;
	else
		rc = 0;
	return rc;
}

