/*
 * Copyright (C) 2015-2025 IoT.bzh Company
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

#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

/** structure for TEXT <> INTEGER association */
typedef struct
{
    const char *label; /**< text associated to the integer */
    const int  value;  /**< integer associated to the text */
} rp_enum_map_t;

/** search the 'value' in the label=NULL terminated array 'keyvals'
* if found, returns true else return false
*/
extern bool rp_enum_map_has_value(const rp_enum_map_t *keyvals, int value);

/** search the 'label' in the label=NULL terminated array 'keyvals'
* if found, returns true else return false
*/
extern bool rp_enum_map_has_label(const rp_enum_map_t *keyvals, const char *label);

/** search the 'value' in the label=NULL terminated array 'keyvals'
* if found, returns true else return false and emits a report with context
*/
extern bool rp_enum_map_check_value(const rp_enum_map_t *keyvals, int value, const char *context);

/** search the 'label' in the label=NULL terminated array 'keyvals'
* if found, returns true else return false and emits a report with context
*/
extern bool rp_enum_map_check_label(const rp_enum_map_t *keyvals, const char *label, const char *context);

/** search the 'value' in the label=NULL terminated array 'keyvals'
* if found, returns true and store the label in 'result' else return false
*/
extern bool rp_enum_map_valid_value(const rp_enum_map_t *keyvals, int value);

/** search the 'label' in the label=NULL terminated array 'keyvals'
* if found, returns true and store the value in 'result' else return false
*/
extern bool rp_enum_map_value(const rp_enum_map_t *keyvals, const char *label, int *result);

/** search the 'value' in the label=NULL terminated array 'keyvals'
* if found, returns true and store the label in 'result' else return false
*/
extern bool rp_enum_map_label(const rp_enum_map_t *keyvals, int value, const char **result);

/** search the 'label' in the label=NULL terminated array 'keyvals'
* if found, returns its value, otherwise if not found, returns the default value 'def'
*/
extern int rp_enum_map_value_def(const rp_enum_map_t *keyvals, const char *label, int def);

/** search the 'value' in the label=NULL terminated array 'keyvals'
* if found, returns its label, otherwise if not found, returns the default label 'def'
*/
extern const char *rp_enum_map_label_def(const rp_enum_map_t *keyvals, int value, const char *def);

#ifdef	__cplusplus
}
#endif
