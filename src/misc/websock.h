/*
 * Copyright (C) 2015-2023 IoT.bzh Company
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

/*
 * This work is a far adaptation of apache-websocket:
 *   origin:  https://github.com/disconnect/apache-websocket
 *   commit:  cfaef071223f11ba016bff7e1e4b7c9e5df45b50
 *   Copyright 2010-2012 self.disconnect (APACHE-2)
 */

#pragma once

#include <stdint.h>
#include <sys/types.h>

struct iovec;

#define WEBSOCKET_CODE_OK                1000
#define WEBSOCKET_CODE_GOING_AWAY        1001
#define WEBSOCKET_CODE_PROTOCOL_ERROR    1002
#define WEBSOCKET_CODE_CANT_ACCEPT       1003
#define WEBSOCKET_CODE_RESERVED          1004
#define WEBSOCKET_CODE_NOT_SET           1005
#define WEBSOCKET_CODE_ABNORMAL          1006
#define WEBSOCKET_CODE_INVALID_UTF8      1007
#define WEBSOCKET_CODE_POLICY_VIOLATION  1008
#define WEBSOCKET_CODE_MESSAGE_TOO_LARGE 1009
#define WEBSOCKET_CODE_INTERNAL_ERROR    1011

struct websock_itf {
	ssize_t (*writev) (void *, const struct iovec *, int);
	ssize_t (*readv) (void *, const struct iovec *, int);

	void (*on_ping) (void *, size_t size); /* optional, if not NULL, responsible of pong */
	void (*on_pong) (void *, size_t size); /* optional */
	void (*on_close) (void *, uint16_t code, size_t size);
	void (*on_text) (void *, int last, size_t size);
	void (*on_binary) (void *, int last, size_t size);
	void (*on_continue) (void *, int last, size_t size);
	int (*on_extension) (void *, int last, int rsv1, int rsv2, int rsv3, int opcode, size_t size);

	void (*on_error) (void *, uint16_t code, const void *data, size_t size); /* optional */
};

struct websock;

extern int websock_close_empty(struct websock *ws);
extern int websock_close(struct websock *ws, uint16_t code, const void *data, size_t length);
extern int websock_error(struct websock *ws, uint16_t code, const void *data, size_t length);

extern int websock_ping(struct websock *ws, const void *data, size_t length);
extern int websock_pong(struct websock *ws, const void *data, size_t length);
extern int websock_text(struct websock *ws, int last, const void *text, size_t length);
extern int websock_text_v(struct websock *ws, int last, const struct iovec *iovec, int count);
extern int websock_binary(struct websock *ws, int last, const void *data, size_t length);
extern int websock_binary_v(struct websock *ws, int last, const struct iovec *iovec, int count);
extern int websock_continue(struct websock *ws, int last, const void *data, size_t length);
extern int websock_continue_v(struct websock *ws, int last, const struct iovec *iovec, int count);

extern ssize_t websock_read(struct websock *ws, void *buffer, size_t size);
extern int websock_drop(struct websock *ws);

extern int websock_dispatch(struct websock *ws, int loop);

extern struct websock *websock_create_v13(const struct websock_itf *itf, void *closure);
extern void websock_destroy(struct websock *ws);

extern void websock_set_default_max_length(size_t maxlen);
extern void websock_set_max_length(struct websock *ws, size_t maxlen);

extern const char *websocket_explain_error(uint16_t code);
