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


#include <stdint.h>
#include <stdlib.h>

#include "../sys/x-errno.h"
#include "../sys/x-mutex.h"
#include "../sys/x-cond.h"

/*****************************************************************************/
/* LOCKING                                                                   */
/*****************************************************************************/

struct locker {
	struct locker *next;
	const void *item;
	uint32_t lockw: 1;
	uint32_t lockr: 1;
	uint32_t cntw: 15;
	uint32_t cntr: 15;
	x_cond_t cond;
};

static struct locker *lockers;
static x_mutex_t mutlock = X_MUTEX_INITIALIZER;

static struct locker *locker_get(const void *item)
{
	struct locker *lock, *unused;

	/* lock all */
	x_mutex_lock(&mutlock);

	/* search a lock for the item and records first frelock */
	unused = 0;
	lock = lockers;
	while (lock && lock->item != item && lock->item)
		lock = lock->next;
	if (lock && lock->item != item) {
		unused = lock;
		lock = lock->next;
		while (lock && lock->item != item)
			lock = lock->next;
	}

	/* treat the case where lock isn't found */
	if (!lock) {
		if (unused) {
			/* an unused locker exists, predate it */
			unused->item = item;
			lock = unused;
		}
		else {
			/* not locker exists, create it */
			lock = malloc(sizeof *lock);
			if (lock) {
				lock->next = lockers;
				lockers = lock;
				lock->item = item;
				lock->lockw = 0;
				lock->lockr = 0;
				lock->cntw = 0;
				lock->cntr = 0;
				x_cond_init(&lock->cond);
			}
		}
	}
	return lock;
}

static int locker_used(struct locker *lock)
{
	return lock->cntw != 0 || lock->cntr != 0;
}

static void locker_release(struct locker *lock)
{
	if (!locker_used(lock))
		lock->item = 0;
	x_mutex_unlock(&mutlock);
}

static void locker_wait(struct locker *lock)
{
	x_cond_wait(&lock->cond, &mutlock);
}

static void locker_signal(struct locker *lock)
{
	x_cond_broadcast(&lock->cond);
}

void lockany_lock_read(const void *item)
{
	struct locker *lock = locker_get(item);
	if (lock) {
		lock->cntr++;
		while (lock->lockw)
			locker_wait(lock);
		lock->lockr = 1;
		locker_release(lock);
	}
}

int lockany_try_lock_read(const void *item)
{
	int rc = 0;
	struct locker *lock = locker_get(item);
	if (lock) {
		if (lock->lockw)
			rc = X_EAGAIN;
		else {
			lock->cntr++;
			lock->lockr = 1;
		}
		locker_release(lock);
	}
	return rc;
}

void lockany_lock_write(const void *item)
{
	struct locker *lock = locker_get(item);
	if (lock) {
		lock->cntw++;
		while (lock->lockw || lock->lockr)
			locker_wait(lock);
		lock->lockw = 1;
		locker_release(lock);
	}
}

int lockany_try_lock_write(const void *item)
{
	int rc = 0;
	struct locker *lock = locker_get(item);
	if (lock) {
		if (lock->lockw && lock->lockr)
			rc = X_EAGAIN;
		else {
			lock->cntw++;
			lock->lockw = 1;
		}
		locker_release(lock);
	}
	return rc;
}

int lockany_unlock(const void *item)
{
	int rc = 0, sig = 0;
	struct locker *lock = locker_get(item);
	if (lock) {
		if (lock->lockw) {
			lock->lockw = 0;
			lock->cntw--;
			sig = 1;
		}
		else if (lock->lockr) {
			if (!--lock->cntr) {
				sig = 1;
				lock->lockr = 0;
			}
		}
		rc = locker_used(lock);
		if (rc && sig)
			locker_signal(lock);
		locker_release(lock);
	}
	return rc;
}

