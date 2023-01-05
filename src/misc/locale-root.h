/*
 * Copyright (C) 2015-2023 IoT.bzh Company
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

struct locale_root;
struct locale_search;

#if WITH_OPENAT
extern struct locale_root *locale_root_create(int dirfd);
extern struct locale_root *locale_root_create_at(int dirfd, const char *path);
#endif

extern struct locale_root *locale_root_create_path(const char *path);
extern struct locale_root *locale_root_addref(struct locale_root *root);
extern void locale_root_unref(struct locale_root *root);

extern struct locale_search *locale_root_search(struct locale_root *root, const char *definition, int immediate);
extern struct locale_search *locale_search_addref(struct locale_search *search);
extern void locale_search_unref(struct locale_search *search);

extern void locale_root_set_default_search(struct locale_root *root, struct locale_search *search);

#if WITH_OPENAT
extern int locale_root_get_dirfd(struct locale_root *root);
#endif

extern const char *locale_root_get_path(struct locale_root *root);

extern int locale_root_open(struct locale_root *root, const char *filename, int flags, const char *locale);
extern char *locale_root_resolve(struct locale_root *root, const char *filename, const char *locale);

extern int locale_search_open(struct locale_search *search, const char *filename, int flags);
extern char *locale_search_resolve(struct locale_search *search, const char *filename);


