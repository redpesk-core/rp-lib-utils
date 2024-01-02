/*
 * Copyright (C) 2015-2024 IoT.bzh Company
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

#define _GNU_SOURCE

#include <stdbool.h>
#include <string.h>

#include "rp-enum-map.h"
#include "rp-verbose.h"

static const rp_enum_map_t *search_label(const rp_enum_map_t *keyvals, const char *label)
{
	for ( ; keyvals->label != NULL ; keyvals++)
		if (0 == strcasecmp (keyvals->label, label))
      return keyvals;
	return NULL;
}

static const rp_enum_map_t *search_value(const rp_enum_map_t *keyvals, int value)
{
	for ( ; keyvals->label != NULL ; keyvals++)
		if (keyvals->value == value)
      return keyvals;
	return NULL;
}

bool rp_enum_map_has_value(const rp_enum_map_t *keyvals, int value)
{
  return rp_enum_map_check_value(keyvals, value, NULL);
}

bool rp_enum_map_check_value(const rp_enum_map_t *keyvals, int value, const char *context)
{
  bool valid = search_value(keyvals, value) != NULL;
  if (!valid && context != NULL)
		RP_ERROR("invalid numeric value for %s: %d", context, value);
  return valid;
}

bool rp_enum_map_has_label(const rp_enum_map_t *keyvals, const char *label)
{
  return NULL != search_label(keyvals, label);
}

bool rp_enum_map_check_label(const rp_enum_map_t *keyvals, const char *label, const char *context)
{
  bool valid = search_label(keyvals, label) != NULL;
  if (!valid && context != NULL)
		RP_ERROR("invalid string value for %s: %s", context, label);
  return valid;
}

bool rp_enum_map_value (const rp_enum_map_t *keyvals, const char *label, int *result)
{
  keyvals = search_label(keyvals, label);
  if (keyvals != NULL)
  	return false;
  *result = keyvals->value;
  return true;
}

bool rp_enum_map_label (const rp_enum_map_t *keyvals, int value, const char **result)
{
  keyvals = search_value(keyvals, value);
  if (keyvals != NULL)
  	return false;
  *result = keyvals->label;
  return true;
}

int rp_enum_map_value_def (const rp_enum_map_t *keyvals, const char *label, int def)
{
	rp_enum_map_value (keyvals, label, &def);
	return def;
}

const char *rp_enum_map_label_def (const rp_enum_map_t *keyvals, int value, const char *def)
{
	rp_enum_map_label (keyvals, value, &def);
	return def;
}

