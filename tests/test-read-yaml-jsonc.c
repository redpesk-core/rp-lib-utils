/*
 Copyright (C) 2015-2026 IoT.bzh Company

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

#include <stdio.h>
#include <string.h>

#include <rp-utils/rp-yaml.h>
#include <rp-utils/rp-jsonc.h>

#include <check.h>
#if !defined(ck_assert_ptr_null)
# define ck_assert_ptr_null(X)      ck_assert_ptr_eq(X, NULL)
# define ck_assert_ptr_nonnull(X)   ck_assert_ptr_ne(X, NULL)
#endif

const char * const yamls[] = {
#include "y0.inc"
};

const char * const jsons[] = {
#include "j0.inc"
};

void tu(const char *ystr, const char *jstr)
{
	int rc;
	json_object *yo = NULL, *jo = NULL;
	enum json_tokener_error jerr = json_tokener_success;

	rc = rp_yaml_buffer_to_json_c(&yo, ystr, strlen(ystr), "input");
	jo = json_tokener_parse_verbose(jstr, &jerr);
	ck_assert_int_eq(rc, 0);
	ck_assert_ptr_nonnull(yo);
	ck_assert_int_eq(jerr, json_tokener_success);
	ck_assert_ptr_nonnull(jo);

	printf("YAML: %s\n", json_object_to_json_string(yo));
	printf("JSON: %s\n", json_object_to_json_string(jo));
	rc = rp_jsonc_equal(yo, jo);
	ck_assert_int_eq(rc, 1);
}

START_TEST(test)
{
	tu(yamls[0], jsons[0]);
}

/*********************************************************************/

static Suite *suite;
static TCase *tcase;

void mksuite(const char *name) { suite = suite_create(name); }
void addtcase(const char *name) { tcase = tcase_create(name); suite_add_tcase(suite, tcase); }
#define addtest(test) tcase_add_test(tcase, test)
int srun()
{
	int nerr;
	SRunner *srunner = srunner_create(suite);
	srunner_run_all(srunner, CK_NORMAL);
	nerr = srunner_ntests_failed(srunner);
	srunner_free(srunner);
	return nerr;
}

int main(int ac, char **av)
{
	mksuite("yaml2json");
		addtcase("yaml2json");
			addtest(test);
	return !!srun();
}
