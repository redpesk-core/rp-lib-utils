/*
 Copyright (C) 2015-2022 IoT.bzh Company

 Author: José Bollo <jose.bollo@iot.bzh>

 $RP_BEGIN_LICENSE$
 Commercial License Usage
  Licensees holding valid commercial IoT.bzh licenses may use this file in
  accordance with the commercial license agreement provided with the
  Software or, alternatively, in accordance with the terms contained in
  a written agreement between you and The IoT.bzh Company. For licensing terms
  and conditions see https://www.iot.bzh/terms-conditions. For further
  information use the contact form at https://www.iot.bzh/contact.

 GNU General Public License Usage
  Alternatively, this file may be used under the terms of the GNU General
  Public license version 3. This license is as published by the Free Software
  Foundation and appearing in the file LICENSE.GPLv3 included in the packaging
  of this file. Please review the following information to ensure the GNU
  General Public License requirements will be met
  https://www.gnu.org/licenses/gpl-3.0.html.
 $RP_END_LICENSE$
*/

#pragma once

/**
 * Reads the 'file' relative to 'dfd' (see openat) in a freshly
 * allocated memory and returns it in 'content' and 'size' (if not NULL).
 * To help in reading text files, a pending null is always added at the
 * end of the content but not counted reported in the returned size.
 *
 * @param dfd the directory file descriptor number
 * @param file filename to be read (absolute or relative to dfd)
 * @param content if not NULL, where to store the content of the file
 * @param size if not NULL, where to store the length read
 *
 * @return 0 in case of success or else -errno
 */
extern int rp_file_get_at(int dfd, const char *file, char **content, size_t *size);

/**
 * Reads the 'file' in a freshly allocated memory and returns it
 * in 'content' and 'size' (if not NULL).
 * To help in reading text files, a pending null is always added at the
 * end of the content but not counted reported in the returned size.
 *
 * alias for rp_file_get_at(AT_FDCWD, file, content, size)
 *
 * @param file filename to be read
 * @param content if not NULL, where to store the content of the file
 * @param size if not NULL, where to store the length read
 *
 * @return 0 in case of success or else -errno
 */
extern int rp_file_get(const char *file, char **content, size_t *size);

/**
 * Writes the 'file' relative to 'dfd' (see openat) with the 'content' of 'size'.
 *
 * @param dfd the directory file descriptor number
 * @param file filename to be written
 * @param content the content to write
 * @param size the length of the content
 *
 * @return 0 in case of success or else -errno
 */
extern int rp_file_put_at(int dfd, const char *file, const void *content, size_t size);

/**
 * Writes the 'file'  with the 'content' of 'size'.
 *
 * alias for putfile_at(AT_FDCWD, file, content, size)
 *
 * @param file filename to be written
 * @param content the content to write
 * @param size the length of the content
 *
 * @return 0 in case of success or else -errno
 */
extern int rp_file_put(const char *file, const void *content, size_t size);
