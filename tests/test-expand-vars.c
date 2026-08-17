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

/*********************************************************************/

START_TEST (check_expand)
{
	char *r;

	clearenv();
	putenv("X=$Y:$Y");
	putenv("Y=$A:$(unnom):tres:$long");
	putenv("A=a");
	putenv("unnom=hum:${long}");
	putenv("long=rien:$rien:rien");
	putenv("TEST=debut:$X:fin");
	unsetenv("rien");

	// check expansion
	r = rp_expand_vars_env_only("$TEST", 0);
	ck_assert_ptr_nonnull(r);
	printf("%s\n",r);
	ck_assert_str_eq(r, "debut:a:hum:rien::rien:tres:rien::rien:a:hum:rien::rien:tres:rien::rien:fin");
	free(r);

	// check robust to infinite expansion
	putenv("V=xxx");
	putenv("Z=$Z:$V:$Z");
	r = rp_expand_vars_env_only("$Z", 0);
	ck_assert_ptr_null(r);
	r = rp_expand_vars_env_only("$Z", 1);
	ck_assert_ptr_nonnull(r);
	ck_assert_str_eq(r, "$Z");
	free(r);

	// check robust to null
	r = rp_expand_vars_env_only(0, 0);
	ck_assert_ptr_null(r);
	free(r);
}
END_TEST

/*********************************************************************/

char **before;
char **after;

void mc(const char *in, const char *out)
{
	char *r = rp_expand_vars(in, 1, before, after);
	printf("mc got %s\n", r);
	ck_assert_ptr_nonnull(r);
	ck_assert_str_eq(r, out);
	free(r);
}

START_TEST (check_order)
{
	char *x_before[] = { "X=before", "B=before", 0 };
	char *x_after[] = { "X=after", "A=after", "Z=last", 0 };

	clearenv();
	putenv("X=env");
	putenv("A=env");
	putenv("B=env");

	before = 0;
	after = 0;
	mc("$A $B $X $Z", "env env env ");

	before = x_before;
	after = 0;
	mc("$A $B $X $Z", "env before before ");

	before = 0;
	after = x_after;
	mc("$A $B $X $Z", "env env env last");

	before = x_before;
	after = x_after;
	mc("$A $B $X $Z", "env before before last");

	mc("$$", "$");
	mc("\\\\", "\\");
	mc("\\$", "$");
	mc("a\\\\b\\$c$$d", "a\\b$c$d");
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
	mksuite("expand-vars");
		addtcase("expand-vars");
			addtest(check_expand);
			addtest(check_order);
	return !!srun();
}
