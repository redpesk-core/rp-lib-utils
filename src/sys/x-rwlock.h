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

#include <pthread.h>

#define x_rwlock_t                     pthread_rwlock_t
#define x_rwlock_init(prwlock)         pthread_rwlock_init(prwlock,NULL)
#define x_rwlock_destroy(prwlock)      pthread_rwlock_destroy(prwlock)
#define x_rwlock_rdlock(prwlock)       pthread_rwlock_rdlock(prwlock)
#define x_rwlock_wrlock(prwlock)       pthread_rwlock_wrlock(prwlock)
#define x_rwlock_trywrlock(prwlock)    pthread_rwlock_trywrlock(prwlock)
#define x_rwlock_unlock(prwlock)       pthread_rwlock_unlock(prwlock)
#define X_RWLOCK_INITIALIZER           PTHREAD_RWLOCK_INITIALIZER
