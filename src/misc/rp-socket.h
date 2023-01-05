/*
 * Copyright (C) 2015-2023 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
 *
 * $RP_BEGIN_LICENSE$
 * Commercial License Usage
 *  Licensees holding valid commercial IoT.bzh licenses may use this file in
 *  accordance with the commercial license agreement provided with the
 *  Software or, alternatively, in accordance with the terms contained in
 *  a written agreement between you and The IoT.bzh Company. For licensing terms
 *  and conditions see https://www.iot.bzh/terms-conditions. For further
 *  information use the contact form at https://www.iot.bzh/contact.
 *
 * GNU General Public License Usage
 *  Alternatively, this file may be used under the terms of the GNU General
 *  Public license version 3. This license is as published by the Free Software
 *  Foundation and appearing in the file LICENSE.GPLv3 included in the packaging
 *  of this file. Please review the following information to ensure the GNU
 *  General Public License requirements will be met
 *  https://www.gnu.org/licenses/gpl-3.0.html.
 * $RP_END_LICENSE$
 */

#pragma once

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
