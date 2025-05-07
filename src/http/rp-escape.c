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

#include "rp-escape.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Test if 'c' is to be escaped or not.
 * Any character that is not in [-.0-9A-Z_a-z~]
 * must be escaped.
 * Note that space versus + is not managed here.
 */
static inline int should_escape(char c)
{
/* ASCII CODES OF UNESCAPED CHARS
car hx/oct  idx
'-' 2d/055  1
'.' 2e/056  2
'0' 30/060  3
...         ..
'9' 39/071  12
'A' 41/101  13
...         ..
'Z' 5a/132  38
'_' 5f/137  39
'a' 61/141  40
...         ..
'z' 7a/172  65
'~' 7e/176  66
*/
	/* [-.0-9A-Z_a-z~] */
	if (c <= 'Z') {
		/* [-.0-9A-Z] */
		if (c < '0') {
			/* [-.] */
			return c != '-' && c != '.';
		} else {
			/* [0-9A-Z] */
			return c < 'A' && c > '9';
		}
	} else {
		/* [_a-z~] */
		if (c <= 'z') {
			/* [_a-z] */
			return c < 'a' && c != '_';
		} else {
			/* [~] */
			return c != '~';
		}
	}
}

/*
 * returns the ASCII char for the hexadecimal
 * digit of the binary value 'f'.
 * returns 0 if f isn't in [0 ... 15].
 */
static inline char bin2hex(int f)
{
	if ((f & 15) != f)
		f = 0;
	else if (f < 10)
		f += '0';
	else
		f += 'A' - 10;
	return (char)f;
}

/*
 * returns the binary value for the hexadecimal
 * digit whose char is 'c'.
 * returns -1 if c isn't i n[0-9A-Fa-f]
 */
static inline int hex2bin(char c)
{
	/* [0-9A-Fa-f] */
	if (c <= 'F') {
		/* [0-9A-F] */
		if (c <= '9') {
			/* [0-9] */
			if (c >= '0') {
				return (int)(c - '0');
			}
		} else if (c >= 'A') {
			/* [A-F] */
			return (int)(c - ('A' - 10));
		}
	} else {
		/* [a-f] */
		if (c >= 'a' && c <= 'f') {
			return (int)(c - ('a' - 10));
		}
	}
	return -1;
}

/*
 * returns the length that will have the text 'itext' of length
 * 'ilen' when escaped. When 'ilen' == 0, strlen is used to
 * compute the size of 'itext'.
 */
static size_t escaped_length(const char *itext, size_t ilen)
{
	char c;
	size_t i, r;

	if (!ilen)
		ilen = strlen(itext);
	c = itext[i = r = 0];
	while (i < ilen) {
		r += c != ' ' && should_escape(c) ? 3 : 1;
		c = itext[++i];
	}
	return r;
}

/*
 * Escapes the text 'itext' of length 'ilen'.
 * When 'ilen' == 0, strlen is used to compute the size of 'itext'.
 * The escaped text is put in 'otext' of length 'olen'.
 * Returns the length of the escaped text (it can be greater than 'olen').
 * When 'olen' is greater than the needed length, an extra null terminator
 * is appened to the escaped string.
 */
static size_t escape_to(const char *itext, size_t ilen, char *otext, size_t olen)
{
	char c;
	size_t i, r;

	if (!ilen)
		ilen = strlen(itext);
	c = itext[i = r = 0];
	while (i < ilen) {
		if (c == ' ')
			c = '+';
		else if (should_escape(c)) {
			if (r < olen)
				otext[r] = '%';
			r++;
			if (r < olen)
				otext[r] = bin2hex((c >> 4) & 15);
			r++;
			c = bin2hex(c & 15);
		}
		if (r < olen)
			otext[r] = c;
		r++;
		c = itext[++i];
	}
	if (r < olen)
		otext[r] = 0;
	return r;
}

/*
 * returns the length of 'itext' of length 'ilen' that can be unescaped.
 * compute the size of 'itext' when 'ilen' == 0.
 */
static size_t unescapable_length(const char *itext, size_t ilen)
{
	char c;
	size_t i;

	c = itext[i = 0];
	while (i < ilen) {
		if (c != '%')
			i++;
		else {
			if (i + 3 > ilen
			 || hex2bin(itext[i + 1]) < 0
			 || hex2bin(itext[i + 2]) < 0)
				break;
			i += 3;
		}
		c = itext[i];
	}
	return i;
}

/*
 * returns the length that will have the text 'itext' of length
 * 'ilen' when escaped. When 'ilen' == 0, strlen is used to
 * compute the size of 'itext'.
 */
static size_t unescaped_length(const char *itext, size_t ilen)
{
	char c;
	size_t i, r;

	c = itext[i = r = 0];
	while (i < ilen) {
		i += (size_t)(1 + ((c == '%') << 1));
		r++;
		c = itext[i];
	}
	return r;
}

static size_t unescape_to(const char *itext, size_t ilen, char *otext, size_t olen)
{
	char c;
	size_t i, r;
	int h, l;

	ilen = unescapable_length(itext, ilen);
	c = itext[i = r = 0];
	while (i < ilen) {
		if (c != '%') {
			if (c == '+')
				c = ' ';
			i++;
		} else {
			if (i + 2 >= ilen)
				break;
			h = hex2bin(itext[i + 1]);
			l = hex2bin(itext[i + 2]);
			c = (char)((h << 4) | l);
			i += 3;
		}
		if (r < olen)
			otext[r] = c;
		r++;
		c = itext[i];
	}
	if (r < olen)
		otext[r] = 0;
	return r;
}

/* create an url */
static size_t escape_url_to(const char *base, const char *path, const char * const *args, char *buffer, size_t length, size_t lenbase, size_t lenpath)
{
	int i;
	size_t l;

	l = lenbase;
	if (lenbase) {
		if (length)
			memcpy(buffer, base, length >= lenbase ? lenbase : length);
		if (base[lenbase - 1] != '/' && lenpath && path[0] != '/') {
			if (l < length)
				buffer[l] = '/';
			l++;
		}
	}
	if (lenpath) {
		if (l < length)
			memcpy(buffer + l, path, length - l >= lenpath ? lenpath : length - l);
		l += lenpath;
	}
	i = 0;
	while (args[i]) {
		if (i) {
			if (l < length)
				buffer[l] = '&';
			l++;
		} else if (lenbase | lenpath) {
			if (l < length)
				buffer[l] = memchr(buffer, '?', l) ? '&' : '?';
			l++;
		}
		l += escape_to(args[i], strlen(args[i]), buffer + l, l < length ? length - l : 0);
		i++;
		if (args[i]) {
			if (l < length)
				buffer[l] = '=';
			l++;
			l += escape_to(args[i], strlen(args[i]), buffer + l, l < length ? length - l : 0);
		}
		i++;
	}
	if (l < length)
		buffer[l] = 0;
	return l;
}


/* create an url */
size_t rp_escape_url_to(const char *base, const char *path, const char * const *args, char *buffer, size_t length)
{
	size_t lenbase, lenpath;
	const char *null;

	/* normalize args */
	lenbase = base ? strlen(base) : 0;
	lenpath = path ? strlen(path) : 0;
	if (!args) {
		null = NULL;
		args = &null;
	}

	/* make the URL */
	return escape_url_to(base, path, args, buffer, length, lenbase, lenpath);
}

/* create an url */
char *rp_escape_url(const char *base, const char *path, const char * const *args, size_t *length)
{
	size_t rlen, lenbase, lenpath;
	char *result;
	const char *null;

	/* normalize args */
	lenbase = base ? strlen(base) : 0;
	lenpath = path ? strlen(path) : 0;
	if (!args) {
		null = NULL;
		args = &null;
	}

	/* compute size */
	rlen = escape_url_to(base, path, args, NULL, 0, lenbase, lenpath);
	if (length)
		*length = rlen;

	/* allocates */
	result = malloc(++rlen);
	if (result)
		/* make the URL */
		escape_url_to(base, path, args, result, rlen, lenbase, lenpath);
	return result;
}

char *rp_escape_args(const char * const *args, size_t *length)
{
	return rp_escape_url(NULL, NULL, args, length);
}

char *rp_escape_str(const char* str, size_t *length)
{
	const char *args[3] = { str, NULL, NULL };
	return rp_escape_url(NULL, NULL, args, length);
}

const char **rp_unescape_args(const char *args)
{
	char **r, **q;
	char c, *p;
	size_t j, z, l, n, lt;

	/* count: number of args (n) text length (lt) */
	lt = n = 0;
	if (args[0]) {
		z = 0;
		do {
			l = strcspn(&args[z], "&=");
			j = 1 + unescaped_length(&args[z], l);
			lt += j;
			z += l;
			c = args[z++];
			if (c == '=') {
				l = strcspn(&args[z], "&");
				j = 1 + unescaped_length(&args[z], l);
				lt += j;
				z += l;
				c = args[z++];
			}
			n++;
		} while(c);
	}

	/* alloc the result */
	l = lt + (2 * n + 1) * sizeof(char *);
	r = malloc(l);
	if (!r)
		return (const char **)r;

	/* fill the result */
	q = r;
	p = (void*)&r[2 * n + 1];
	if (args[0]) {
		z = 0;
		do {
			q[0] = p;
			l = strcspn(&args[z], "&=");
			j = 1 + unescape_to(&args[z], l, p, lt);
			lt -= j;
			p += j;
			z += l;
			c = args[z++];
			if (c != '=')
				q[1] = p - 1; /*empty string from key name*/
			else {
				q[1] = p;
				l = strcspn(&args[z], "&");
				j = 1 + unescape_to(&args[z], l, p, lt);
				lt -= j;
				p += j;
				z += l;
				c = args[z++];
			}
			q = &q[2];
		} while(c);
	}
	q[0] = NULL;
	return (const char **)r;
}

const char *rp_unescaped_args_get(const char **args, const char *key)
{
    /* iterate to wanted key or end */
    while (*args != NULL && strcmp(*args, key) != 0) {
        args += 2;
    }

    /* if NULL (end) not reached, the key was found */
    return args[*args != NULL];
}

char *rp_escape(const char *text, size_t textlen, size_t *reslength)
{
	size_t len;
	char *result;

	if (!textlen)
		textlen = strlen(text);
	len = 1 + escaped_length(text, textlen);
	result = malloc(len);
	if (result)
		escape_to(text, textlen, result, len);
	if (reslength)
		*reslength = len - 1;
	return result;
}

char *rp_unescape(const char *text, size_t textlen, size_t *reslength)
{
	size_t len;
	char *result;

	if (!textlen)
		textlen = strlen(text);
	len = 1 + unescaped_length(text, textlen);
	result = malloc(len);
	if (result)
		unescape_to(text, textlen, result, len);
	if (reslength)
		*reslength = len - 1;
	return result;
}

#if 0
#include <stdio.h>
int main(int ac, char **av)
{
	int i;
	char *x, *y, *z;
	const char **v;

	for (i=1 ; i < ac ; i++)
		if (!av[i][0])
			av[i] = NULL;
	av[ac] = av[ac + 1] = NULL;

	x = rp_escape_args((void*)++av, NULL);
	y = rp_escape(x, strlen(x), NULL);
	z = rp_unescape(y, strlen(y), NULL);
	v = rp_unescape_args(x);

	printf("%s\n%s\n%s\n", x, y, z);
	free(x);
	free(y);
	free(z);
	i = 0;
	while(v[i]) {
		printf("%s=%s / %s=%s\n", av[i], av[i+1], v[i], v[i+1]);
		i += 2;
	}
	free(v);
	return 0;
}
#endif
/* vim: set colorcolumn=80: */
