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

#pragma once

#include <pthread.h>
#include <signal.h>
#include "x-errno.h"

#define x_thread_t	pthread_t

typedef void (*x_thread_cb)(void* arg);

static inline int x_thread_create(
			x_thread_t *tid,
                        x_thread_cb entry,
			void *arg,
			int detached)
{
	int rc;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	if (detached)
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	rc = pthread_create(tid, &attr, (void*)entry, arg) ? -errno : 0;
	pthread_attr_destroy(&attr);
	return rc;
}

static inline int x_thread_detach(x_thread_t tid)
{
	return pthread_detach(tid) ? -errno : 0;
}

static inline x_thread_t x_thread_self(void)
{
	return pthread_self();
}

static inline int x_thread_equal(x_thread_t t1, x_thread_t t2)
{
	return pthread_equal(t1, t2);
}

static inline int x_thread_kill(x_thread_t tid, int sig)
{
	return pthread_kill(tid, sig);
}

static inline void x_thread_exit(void *retval)
{
	pthread_exit(retval);
}

static inline int x_thread_join(x_thread_t tid, void **retval)
{
	return pthread_join(tid, retval);
}

#if WITH_THREAD_LOCAL
#define X_TLS(type,name) \
	static _Thread_local type *__tls_##name;\
	static inline type *x_tls_get_##name()\
		{ return __tls_##name; } \
	static inline void x_tls_set_##name(type *value)\
		{ __tls_##name = value; }
#else
#define X_TLS(type,name) \
	static inline pthread_key_t __tls_key_##name()\
		{\
			static pthread_key_t key = 0;\
			if (!key)\
				pthread_key_create(&key, NULL);\
			return key;\
		}\
	static inline type *x_tls_get_##name()\
		{ return (type*)pthread_getspecific(__tls_key_##name()); }\
	static inline void x_tls_set_##name(type *value)\
		{ pthread_setspecific(__tls_key_##name(),value); }
#endif
