/*
 * Copyright (C) 2015-2024 IoT.bzh Company
 * Author jobol
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

#include "rp-jconf.h"

#include <stdbool.h>
#include <json-c/json.h>

#include "rp-enum-map.h"
#include "rp-verbose.h"

static bool jconf_get(
		json_object *conf,
		const char *key,
		bool required,
		json_object **store
) {
	if (json_object_object_get_ex(conf, key, store))
		return true;
	*store = NULL;
	if (required)
		RP_ERROR("key %s is missing in %s",
			key, json_object_to_json_string(conf));
	return false;
}

bool jconf_any(
		json_object *conf,
		const char *key,
		bool required,
		json_object **store
) {
	if (jconf_get(conf, key, required, store))
		return true;
	*store = NULL;
	return !required;
}

bool jconf_int(
		json_object *conf,
		const char *key,
		bool required,
		int *store,
		int defvalue
) {
	json_object *obj;
	*store = defvalue;
	if (jconf_get(conf, key, required, &obj)) {
		if (json_object_is_type(obj, json_type_int)) {
			*store = json_object_get_int(obj);
			return true;
		}
		RP_ERROR("key %s isn't an integer in %s",
			key, json_object_to_json_string(conf));
	}
	else if (!required)
		return true;
	return false;
}

bool jconf_bool(
		json_object *conf,
		const char *key,
		bool required,
		bool *store,
		bool defvalue
) {
	json_object *obj;
	*store = defvalue;
	if (jconf_get(conf, key, required, &obj)) {
		if (json_object_is_type(obj, json_type_boolean)) {
			*store = json_object_get_boolean(obj);
			return true;
		}
		RP_ERROR("key %s isn't a boolean in %s",
			key, json_object_to_json_string(conf));
	}
	else if (!required)
		return true;
	return false;
}

bool jconf_string(
		json_object *conf,
		const char *key,
		bool required,
		const char **store,
		const char *defvalue
) {
	json_object *obj;
	*store = defvalue;
	if (jconf_get(conf, key, required, &obj)) {
		if (json_object_is_type(obj, json_type_string)) {
			*store = json_object_get_string(obj);
			return true;
		}
		RP_ERROR("key %s isn't a string in %s",
			key, json_object_to_json_string(conf));
	}
	else if (!required)
		return true;
	return false;
}

bool jconf_enum(
		json_object *conf,
		const char *key,
		bool required,
		int *store,
		int defvalue,
		const rp_enum_map_t *keyvals
) {
	char c = 0;
	const char *found = NULL;
	*store = defvalue;
	bool ok = jconf_string(conf, key, required, &found, &c);
	if (ok && found != &c) {
		ok = rp_enum_map_value (keyvals, found, store);
		if (!ok) {
			RP_ERROR("invalid value %s for key %s in %s",
			         found, key, json_object_to_json_string(conf));
		}
	}
	return ok;
}

