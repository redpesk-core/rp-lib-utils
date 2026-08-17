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

#define _GNU_SOURCE

#include "rp-enum-map.h"

#include <stdbool.h>
#include <string.h>

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

