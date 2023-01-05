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

#pragma once

#include <stdint.h>
#include <sys/types.h>
#include "../sys/x-buf.h"

#define WS_CODE_OK                1000
#define WS_CODE_GOING_AWAY        1001
#define WS_CODE_PROTOCOL_ERROR    1002
#define WS_CODE_CANT_ACCEPT       1003
#define WS_CODE_RESERVED          1004
#define WS_CODE_NOT_SET           1005
#define WS_CODE_ABNORMAL          1006
#define WS_CODE_INVALID_UTF8      1007
#define WS_CODE_POLICY_VIOLATION  1008
#define WS_CODE_MESSAGE_TOO_LARGE 1009
#define WS_CODE_INTERNAL_ERROR    1011

#define WS_RSV_1                  4
#define WS_RSV_2                  2
#define WS_RSV_3                  1

#define WS_BUFS_COUNT	4

typedef struct ws_itf_s ws_itf_t;
typedef struct ws_s ws_t;

struct ws_itf_s
{
	ssize_t (*writev) (void *, const x_buf_t *, int);

	void (*on_ping) (ws_t *, const x_buf_t *data); /* optional, if not NULL, responsible of pong */
	void (*on_pong) (ws_t *, const x_buf_t *data); /* optional */
	void (*on_close) (ws_t *, uint16_t code, const x_buf_t *data);

	void (*on_text) (ws_t *, int last, const x_buf_t *data);
	void (*on_binary) (ws_t *, int last, const x_buf_t *data);
	void (*on_continue) (ws_t *, int last, const x_buf_t *data);
	int (*on_extension) (ws_t *, int last, const x_buf_t *data, int rsv123, int opcode); /* optional */

	void (*on_error) (ws_t *, uint16_t code, const void *data, size_t size); /* optional */
};

struct ws_s
{
	int state;
	uint64_t maxlength;
	int lenhead, szhead;
	uint64_t length;
	uint32_t mask;
	x_buf_t bufs[WS_BUFS_COUNT];
	unsigned uses[WS_BUFS_COUNT];
	unsigned char header[14];	/* 2 + 8 + 4 */
	const ws_itf_t *itf;
	void *data;
};


extern int ws_close_empty(ws_t *ws);
extern int ws_close(ws_t *ws, uint16_t code, const void *data, size_t length);
extern int ws_error(ws_t *ws, uint16_t code, const void *data, size_t length);

extern int ws_ping(ws_t *ws, const void *data, size_t length);
extern int ws_pong(ws_t *ws, const void *data, size_t length);
extern int ws_text(ws_t *ws, int last, const void *text, size_t length);
extern int ws_text_v(ws_t *ws, int last, const x_buf_t bufs[], int count);
extern int ws_binary(ws_t *ws, int last, const void *data, size_t length);
extern int ws_binary_v(ws_t *ws, int last, const x_buf_t bufs[], int count);
extern int ws_continue(ws_t *ws, int last, const void *data, size_t length);
extern int ws_continue_v(ws_t *ws, int last, const x_buf_t bufs[], int count);

extern int ws_dispatch(ws_t *ws, const x_buf_t buffers[], int count);

extern int ws_init(ws_t *ws, const ws_itf_t *itf);

extern void ws_set_default_max_length(size_t maxlen);
extern void ws_set_max_length(ws_t *ws, size_t maxlen);

extern const char *ws_strerror(uint16_t code);
