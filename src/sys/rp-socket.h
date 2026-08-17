/*
 * Copyright (C) 2015-2026 IoT.bzh Company
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

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * open socket for client or server with a default scheme
 *
 * @param uri the specification of the socket
 * @param server 0 for client, server otherwise
 * @param scheme the default scheme to use if none is set in uri (can be NULL)
 *
 * @return the file descriptor number of the socket or e negative value in case of error
 */
extern int rp_socket_open_scheme(const char *uri, int server, const char *scheme);

/**
 * open socket for client or server
 *
 * @param uri the specification of the socket
 * @param server 0 for client, server otherwise
 *
 * @return the file descriptor number of the socket or e negative value in case of error
 */
static inline int rp_socket_open(const char *uri, int server)
{
	return rp_socket_open_scheme(uri, server, 0);
}

#ifdef	__cplusplus
}
#endif
