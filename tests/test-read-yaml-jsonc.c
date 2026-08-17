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
