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


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <check.h>
#if !defined(ck_assert_ptr_null)
# define ck_assert_ptr_null(X)      ck_assert_ptr_eq(X, NULL)
# define ck_assert_ptr_nonnull(X)   ck_assert_ptr_ne(X, NULL)
#endif

/*********************************************************************/

#include <rp-utils/rp-expand-vars.h>
#include <rp-utils/rp-jsonc-expand.h>
#include <rp-utils/rp-jsonc.h>

/*********************************************************************/

char input[] = "{ \"key\": [ { \"$ref\": \"$valref\" }, 5, true, 0 ], \"item\": \"x$(valitem)x\" }";
char *vars[] = { "valref=ZZ\\$(toto)ZZ", "valitem=HELLO", "toto=item", NULL };
char output[] = "{ \"key\": [ \"ZZitemZZ\", 5, true, 0 ], \"item\": \"xHELLOx\" }";

void printpath(rp_jsonc_expand_path_t path, struct json_object* object)
{
	int i, n = rp_jsonc_expand_path_length(path);

	printf("ROOT");
	for (i = 0 ; i < n ; i++) {
		if (rp_jsonc_expand_path_is_array(path, i))
			printf("[%d]", (int)rp_jsonc_expand_path_index(path, i));
		else
			printf(".%s", rp_jsonc_expand_path_key(path, i));
	}
	printf(" = %s\n", json_object_get_string(object));
}

struct json_object *expobj(void *closure, struct json_object* object, rp_jsonc_expand_path_t path)
{
	struct json_object *ref;

	ck_assert_ptr_eq(closure, input);
	printpath(path, object);
	if (json_object_object_get_ex(object, "$ref", &ref))
		object = json_object_get(ref);
	return object;
}

struct json_object *expstr(void *closure, struct json_object* object, rp_jsonc_expand_path_t path)
{
	char *trf;

	ck_assert_ptr_eq(closure, input);
	printpath(path, object);
	trf = rp_expand_vars_only(json_object_get_string(object), 0, vars);
	if (trf) {
		object = json_object_new_string(trf);
		free(trf);
	}
	return object;
}

START_TEST (check_expand)
{
	struct json_object *in = json_tokener_parse(input);
	struct json_object *out = json_tokener_parse(output);
	struct json_object *res;

	res = rp_jsonc_expand(in, input, expobj, expstr);
	printf("got %s\n", json_object_get_string(res));
	ck_assert_int_eq(0, rp_jsonc_cmp(res, out));

	if (res != in)
		json_object_put(res);
	json_object_put(out);
	json_object_put(in);
}
END_TEST

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
	mksuite("expand-json");
		addtcase("expand-json");
			addtest(check_expand);
	return !!srun();
}
