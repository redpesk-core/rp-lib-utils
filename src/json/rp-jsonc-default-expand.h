/*
 * Copyright (C) 2015-2026 IoT.bzh Company
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

#include <json-c/json.h>

#define RP_JSONEXP_SCAN_TRUE          1 /**< for replacing "true" by true */
#define RP_JSONEXP_SCAN_FALSE         2 /**< for replacing "false" by false */
#define RP_JSONEXP_SCAN_NULL          4 /**< for replacing "null" by null */
#define RP_JSONEXP_SCAN_HEXA          8 /**< for replacing "0xHHH" by N */
#define RP_JSONEXP_SCAN_OCTAL        16 /**< for replacing "0oOOO" or "0OOO" by N */
#define RP_JSONEXP_SCAN_BINARY       32 /**< for replacing "0bBBB" by N */
#define RP_JSONEXP_SCAN_DECIMAL      64 /**< for replacing "DDDDD" by N */
#define RP_JSONEXP_SCAN_DOUBLE      128 /**< for replacing "D.DeD" by N */

#define RP_JSONEXP_ENVVAR           256 /**< for replacing "${X}" by getenv(X) */

#define RP_JSONEXP_$REFS            512 /**< for expanding {"$ref":"PATH"} to content(PATH) */
#define RP_JSONEXP_DELETE_NULLS    1024 /**< for removing fields whose value is null */


/** for replacing strings encoding booleans */
#define RP_JSONEXP_SCAN_BOOLEAN    (RP_JSONEXP_SCAN_TRUE \
                                   |RP_JSONEXP_SCAN_FALSE)

/** for replacing strings encoding integers */
#define RP_JSONEXP_SCAN_INT        (RP_JSONEXP_SCAN_HEXA \
                                   |RP_JSONEXP_SCAN_OCTAL \
                                   |RP_JSONEXP_SCAN_BINARY \
                                   |RP_JSONEXP_SCAN_DECIMAL)

/** for replacing strings encoding numbers */
#define RP_JSONEXP_SCAN_NUMBER     (RP_JSONEXP_SCAN_INT \
                                   |RP_JSONEXP_SCAN_DOUBLE)

/** for replacing strings encoding any value */
#define RP_JSONEXP_SCAN_ANY        (RP_JSONEXP_SCAN_BOOLEAN \
                                   |RP_JSONEXP_SCAN_NULL \
                                   |RP_JSONEXP_SCAN_NUMBER)

/** for replacing strings encoding any value, expanding $ref adn removing null fields*/
#define RP_JSONEXP_ALL             (RP_JSONEXP_SCAN_ANY \
                                   |RP_JSONEXP_ENVVAR \
				   |RP_JSONEXP_$REFS \
				   |RP_JSONEXP_DELETE_NULLS)

/**
 * Expands strings and references of object and replace it with it new value.
 * This function does not load files. It instead calls the given function
 * 'readfunc' with 3 arguments: the given closure, a pointer were the function
 * should store the read JSON and the filename found in $ref. The function
 * 'readfunc' should return 0 on success or a negative value on error.
 *
 * If 'readfunc' is NULL but flags holds RP_JSONEXP_$REFS, a default loader
 * is used.
 *
 * @param object   pointer to the object to process
 * @param readfunc function for reading the JSON content of a filename (or NULL)
 * @param closure  closure data for readfunc
 * @param flags    bit or of allowed operations (see constants RP_JSONEXP_...)
 *
 * @return 0 on success or a negative value on error.
 */
extern
int
rp_jsonc_default_expanding(
	struct json_object **object,
	int (*readfunc)(void *closure, struct json_object **obj, const char *filename),
	void *closure,
	int flags
);

/**
 * Shortcut for calling rp_jsonc_default_expanding with flags RP_JSONEXP_ALL
 */
static inline
int
rp_jsonc_default_expand(struct json_object **object)
{
	return rp_jsonc_default_expanding(object, NULL, NULL, RP_JSONEXP_ALL);
}

