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

#include <rp-utils/rp-jsonc-path.h>
#include <rp-utils/rp-jsonc-iter.h>

#include <check.h>
#if !defined(ck_assert_ptr_null)
# define ck_assert_ptr_null(X)      ck_assert_ptr_eq(X, NULL)
# define ck_assert_ptr_nonnull(X)   ck_assert_ptr_ne(X, NULL)
#endif

const char * const jsons[] = {
#include "jptr.inc"
};

const char *visit_names[] = {
	[RP_JSONC_ITER_LEAF] = "=",
	[RP_JSONC_ITER_BEFORE_OBJECT] = "{",
	[RP_JSONC_ITER_AFTER_OBJECT] = "}",
	[RP_JSONC_ITER_BEFORE_ARRAY] = "[",
	[RP_JSONC_ITER_AFTER_ARRAY] = "]",
	[RP_JSONC_ITER_ELEMENT] = "?"
};

struct desc {
	rp_jsonc_iter_visit_t visit;
	const char *pointer;
};

struct desc dscall[] = {
  { RP_JSONC_ITER_BEFORE_OBJECT, "" }, /* 0 */
  { RP_JSONC_ITER_ELEMENT,       "/foo" }, /* 1 */
  { RP_JSONC_ITER_BEFORE_ARRAY,  "/foo" }, /* 2 */
  { RP_JSONC_ITER_ELEMENT,       "/foo/0" }, /* 3 */
  { RP_JSONC_ITER_LEAF,          "/foo/0" }, /* 4 */
  { RP_JSONC_ITER_ELEMENT,       "/foo/1" }, /* 5 */
  { RP_JSONC_ITER_LEAF,          "/foo/1" }, /* 6 */
  { RP_JSONC_ITER_AFTER_ARRAY,   "/foo" }, /* 7 */
  { RP_JSONC_ITER_ELEMENT,       "/" }, /* 8 */
  { RP_JSONC_ITER_LEAF,          "/" }, /* 9 */
  { RP_JSONC_ITER_ELEMENT,       "/a~1b" }, /* 10 */
  { RP_JSONC_ITER_LEAF,          "/a~1b" }, /* 11 */
  { RP_JSONC_ITER_ELEMENT,       "/c%d" }, /* 12 */
  { RP_JSONC_ITER_LEAF,          "/c%d" }, /* 13 */
  { RP_JSONC_ITER_ELEMENT,       "/e^f" }, /* 14 */
  { RP_JSONC_ITER_LEAF,          "/e^f" }, /* 15 */
  { RP_JSONC_ITER_ELEMENT,       "/g|h" }, /* 16 */
  { RP_JSONC_ITER_LEAF,          "/g|h" }, /* 17 */
  { RP_JSONC_ITER_ELEMENT,       "/i\\j" }, /* 18 */
  { RP_JSONC_ITER_LEAF,          "/i\\j" }, /* 19 */
  { RP_JSONC_ITER_ELEMENT,       "/k\"l" }, /* 20 */
  { RP_JSONC_ITER_LEAF,          "/k\"l" }, /* 21 */
  { RP_JSONC_ITER_ELEMENT,       "/ " }, /* 22 */
  { RP_JSONC_ITER_LEAF,          "/ " }, /* 23 */
  { RP_JSONC_ITER_ELEMENT,       "/m~0n" }, /* 24 */
  { RP_JSONC_ITER_LEAF,          "/m~0n" }, /* 25 */
  { RP_JSONC_ITER_AFTER_OBJECT,  "" }, /* 26 */
};

struct desc dscleafcon[] = {
  { RP_JSONC_ITER_BEFORE_OBJECT, "" },
  { RP_JSONC_ITER_BEFORE_ARRAY,  "/foo" },
  { RP_JSONC_ITER_LEAF,          "/foo/0" },
  { RP_JSONC_ITER_LEAF,          "/foo/1" },
  { RP_JSONC_ITER_AFTER_ARRAY,   "/foo" },
  { RP_JSONC_ITER_LEAF,          "/" },
  { RP_JSONC_ITER_LEAF,          "/a~1b" },
  { RP_JSONC_ITER_LEAF,          "/c%d" },
  { RP_JSONC_ITER_LEAF,          "/e^f" },
  { RP_JSONC_ITER_LEAF,          "/g|h" },
  { RP_JSONC_ITER_LEAF,          "/i\\j" },
  { RP_JSONC_ITER_LEAF,          "/k\"l" },
  { RP_JSONC_ITER_LEAF,          "/ " },
  { RP_JSONC_ITER_LEAF,          "/m~0n" },
  { RP_JSONC_ITER_AFTER_OBJECT,  "" },
};

struct desc dscleaf[] = {
  { RP_JSONC_ITER_LEAF,          "/foo/0" },
  { RP_JSONC_ITER_LEAF,          "/foo/1" },
  { RP_JSONC_ITER_LEAF,          "/" },
  { RP_JSONC_ITER_LEAF,          "/a~1b" },
  { RP_JSONC_ITER_LEAF,          "/c%d" },
  { RP_JSONC_ITER_LEAF,          "/e^f" },
  { RP_JSONC_ITER_LEAF,          "/g|h" },
  { RP_JSONC_ITER_LEAF,          "/i\\j" },
  { RP_JSONC_ITER_LEAF,          "/k\"l" },
  { RP_JSONC_ITER_LEAF,          "/ " },
  { RP_JSONC_ITER_LEAF,          "/m~0n" },
};

struct cloprt {
	int cnt;
	struct desc *ref;
	struct json_object *root;
};

int prtpath(
	void *closure,
	struct json_object *object,
	rp_jsonc_iter_visit_t visit,
	rp_jsonc_path_t *path
) {
	int rc;
	char *pointer;
	struct cloprt *clp = closure;
	struct json_object *found;

	ck_assert_int_ge(visit, 0);
	ck_assert_int_lt((int)visit, (int)sizeof visit_names);

	ck_assert_ptr_nonnull(path);
	rc = rp_jsonc_path_valid(path, 1);
	ck_assert_int_ne(rc, 0);

	pointer = rp_jsonc_path_pointer(path);
	ck_assert_ptr_nonnull(pointer);

	printf("%d %d%s '%s'\n", clp->cnt, visit, visit_names[visit], pointer);
	ck_assert_str_eq(pointer, clp->ref[clp->cnt].pointer);
	ck_assert_int_eq(visit, clp->ref[clp->cnt].visit);

	rc = rp_jsonc_path_search(clp->root, pointer, &found);
	ck_assert_int_eq(rc, 1);
	ck_assert_ptr_eq(object, found);

	free(pointer);
	clp->cnt++;
	return 1;
}

void tu(const char *jstr, struct desc *ref, rp_jsonc_iter_want_t want)
{
	int rc;
	json_object *jo = NULL;
	enum json_tokener_error jerr = json_tokener_success;
	struct cloprt clp = { 0, ref, NULL };

	jo = json_tokener_parse_verbose(jstr, &jerr);
	ck_assert_int_eq(jerr, json_tokener_success);
	ck_assert_ptr_nonnull(jo);

	printf("\nWANT %d\n", want);
	clp.root = jo;
	rc = rp_jsonc_iter_walk(jo, want, prtpath, &clp);
	ck_assert_int_eq(rc, 0);
	json_object_put(jo);
}

START_TEST(test)
{
	tu(jsons[0], dscall, RP_JSONC_ITER_WANT_ALL);
	tu(jsons[0], dscleafcon, RP_JSONC_ITER_WANT_ALL & ~RP_JSONC_ITER_WANT_ELEMENT);
	tu(jsons[0], dscleaf, RP_JSONC_ITER_WANT_LEAF);
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
	mksuite("json-it-ptr");
		addtcase("json-ptr");
			addtest(test);
	return !!srun();
}

