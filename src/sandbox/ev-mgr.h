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

#include "x-epoll.h"
#include <time.h>
/******************************************************************************/

struct ev_mgr;

/**
 * Creates a new event manager
 *
 * @param mgr   address where is stored the result
 *
 * @return 0 on success or an negative error code
 */
extern int ev_mgr_create(struct ev_mgr **mgr);

/**
 * Increase the reference count of the event manager
 *
 * @param mgr  the event manager
 *
 * @return the event manager
 */
extern struct ev_mgr *ev_mgr_addref(struct ev_mgr *mgr);

/**
 * Deccrease the reference count of the event manager and release it
 * if it falls to zero
 *
 * @param mgr  the event manager
 */
extern void ev_mgr_unref(struct ev_mgr *mgr);

/**
 * Return a pollable/selectable file descriptor
 * for the event manager
 *
 * @param mgr  the event manager
 *
 * @return the file descriptor
 */
extern int ev_mgr_get_fd(struct ev_mgr *mgr);

/**
 * prepare the ev_mgr to run
 *
 * @param mgr  the event manager
 *
 * @return 0 on success or an negative error code
 */
extern int ev_mgr_prepare(struct ev_mgr *mgr);

/**
 * prepare the ev_mgr to run with a wakeup
 *
 * @param mgr  the event manager
 * @param wakeup_ms the wakeup time in millisecond
 *
 * @return 0 on success or an negative error code
 */
extern int ev_mgr_prepare_with_wakeup(struct ev_mgr *mgr, int wakeup_ms);

/**
 * wait an event
 *
 * @param mgr  the event manager
 *
 * @return 0 if no event were raise, 1 if one event
 * occured or an negative error code
 */
extern int ev_mgr_wait(struct ev_mgr *mgr, int timeout_ms);

/**
 * dispatch lastest events
 *
 * @param mgr  the event manager
 *
 * @return 0 on success or an negative error code
 */
extern void ev_mgr_dispatch(struct ev_mgr *mgr);

/**
 */
extern int ev_mgr_run(struct ev_mgr *mgr, int timeout_ms);
extern int ev_mgr_can_run(struct ev_mgr *mgr);
extern void ev_mgr_recover_run(struct ev_mgr *mgr);

/**
 * wake up the event loop if needed
 *
 * @param mgr  the event manager
 */
extern void ev_mgr_wakeup(struct ev_mgr *mgr);

/**
 * Try to change the holder
 *
 * It is successful if the given holder is the current holder
 *
 * @param mgr     the event manager
 * @param holder  the requesting possible holder
 * @param next    next holder to be set if holder matches current holder
 *
 * @return the afterward holder that is either
 */
extern void *ev_mgr_try_change_holder(struct ev_mgr *mgr, void *holder, void *next);


/**
 * Gets the current holder
 *
 * @return the current holder
 */
extern void *ev_mgr_holder(struct ev_mgr *mgr);


/******************************************************************************/

struct ev_prepare;

typedef void (*ev_prepare_cb_t)(struct ev_prepare *prep, void *closure);

extern int ev_mgr_add_prepare(
		struct ev_mgr *mgr,
		struct ev_prepare **prep,
		ev_prepare_cb_t handler,
		void *closure);

extern struct ev_prepare *ev_prepare_addref(struct ev_prepare *prep);
extern void ev_prepare_unref(struct ev_prepare *prep);

/******************************************************************************/

struct ev_fd;

typedef void (*ev_fd_cb_t)(struct ev_fd *efd, int fd, uint32_t revents, void *closure);

/**
 * Callbacks of file event handlers
 *
 * These callbacks are called when an event occurs on the handled
 * file descriptor.
 *
 * @param efd the file event handler object
 * @param fd the file descriptor index
 * @param revents the received events
 * @param closure the closure given at creation
 */
extern int ev_mgr_add_fd(
		struct ev_mgr *mgr,
		struct ev_fd **efd,
		int fd,
		uint32_t events,
		ev_fd_cb_t handler,
		void *closure,
		int autounref,
		int autoclose);

extern struct ev_fd *ev_fd_addref(struct ev_fd *efd);
extern void ev_fd_unref(struct ev_fd *efd);

extern int ev_fd_fd(struct ev_fd *efd);

extern uint32_t ev_fd_events(struct ev_fd *efd);
extern void ev_fd_set_events(struct ev_fd *efd, uint32_t events);

extern void ev_fd_set_handler(struct ev_fd *efd, ev_fd_cb_t handler, void *closure);

/******************************************************************************/

struct ev_timer;

/**
 * Callbacks of timers
 *
 * These callbacks are called when a programmed time even occurs.
 *
 * @param timer the timer object
 * @param closure the closure given at creation
 * @param decount reverse index of the event: zero for infinite timer
 *                or a decreasing value finishing with 1
 */
typedef void (*ev_timer_cb_t)(struct ev_timer *timer, void *closure, int decount);

extern int ev_mgr_add_timer(
		struct ev_mgr *mgr,
		struct ev_timer **timer,
		int absolute,
		time_t start_sec,
		unsigned start_ms,
		unsigned count,
		unsigned period_ms,
		unsigned accuracy_ms,
		ev_timer_cb_t handler,
		void *closure,
		int autounref);

extern struct ev_timer *ev_timer_addref(struct ev_timer *timer);
extern void ev_timer_unref(struct ev_timer *timer);
