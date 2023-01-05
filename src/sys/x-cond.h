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

#include "x-mutex.h"

#include <pthread.h>

#define x_cond_t                  pthread_cond_t
#define x_cond_init(pcond)        pthread_cond_init(pcond,NULL)
#define x_cond_destroy(pcond)     pthread_cond_destroy(pcond)
#define x_cond_wait(pcond,pmutex) pthread_cond_wait(pcond,pmutex)
#define x_cond_timedwait(pcond,pmutex,abst) pthread_cond_timedwait(pcond,pmutex,abst)
#define x_cond_signal(pcond)      pthread_cond_signal(pcond)
#define x_cond_broadcast(pcond)   pthread_cond_broadcast(pcond)
#define X_COND_INITIALIZER        PTHREAD_COND_INITIALIZER

