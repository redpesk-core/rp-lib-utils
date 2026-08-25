
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../src/misc/rp-enum-map.h"

#include <check.h>
#if !defined(ck_assert_ptr_null)
# define ck_assert_ptr_null(X)      ck_assert_ptr_eq(X, NULL)
# define ck_assert_ptr_nonnull(X)   ck_assert_ptr_ne(X, NULL)
#endif

rp_enum_map_t emap[] = {
	{ "a", 1 },
	{ "z", 2 },
	{ "e", 3 },
	{ "r", 4 },
	{ "t", 5 },
	{ "y", 6 },
	{ "u", 7 },
	{ "i", 8 },
	{ "o", 9 },
	{ "p", 10 },
	{ NULL, 0 }
};

char none[] = "none";
char any[] = "any";

START_TEST (basics)
{
	bool b;
	int i;
	const char *s;

	b = rp_enum_map_has_value(emap, 0);
	ck_assert_int_eq(0, (int)b);
	b = rp_enum_map_has_value(emap, 1);
	ck_assert_int_eq(1, (int)b);
	b = rp_enum_map_has_value(emap, 5);
	ck_assert_int_eq(1, (int)b);
	b = rp_enum_map_has_value(emap, 10);
	ck_assert_int_eq(1, (int)b);

	b = rp_enum_map_has_label(emap, "x");
	ck_assert_int_eq(0, (int)b);
	b = rp_enum_map_has_label(emap, "a");
	ck_assert_int_eq(1, (int)b);
	b = rp_enum_map_has_label(emap, "y");
	ck_assert_int_eq(1, (int)b);
	b = rp_enum_map_has_label(emap, "p");
	ck_assert_int_eq(1, (int)b);

	b = rp_enum_map_check_value(emap, 0, "PRINT ME");
	ck_assert_int_eq(0, (int)b);
	b = rp_enum_map_check_value(emap, 1, "don't print me");
	ck_assert_int_eq(1, (int)b);
	b = rp_enum_map_check_value(emap, 5, "don't print me");
	ck_assert_int_eq(1, (int)b);
	b = rp_enum_map_check_value(emap, 10, "don't print me");
	ck_assert_int_eq(1, (int)b);

	b = rp_enum_map_check_label(emap, "x", "PRINT ME");
	ck_assert_int_eq(0, (int)b);
	b = rp_enum_map_check_label(emap, "a", "don't print me");
	ck_assert_int_eq(1, (int)b);
	b = rp_enum_map_check_label(emap, "y", "don't print me");
	ck_assert_int_eq(1, (int)b);
	b = rp_enum_map_check_label(emap, "p", "don't print me");
	ck_assert_int_eq(1, (int)b);

	s = none;
	b = rp_enum_map_label(emap, 0, &s);
	ck_assert_int_eq(0, (int)b);
	ck_assert_ptr_eq(s, none);
	b = rp_enum_map_label(emap, 1, &s);
	ck_assert_int_eq(1, (int)b);
	ck_assert_str_eq(s, "a");
	b = rp_enum_map_label(emap, 5, &s);
	ck_assert_int_eq(1, (int)b);
	ck_assert_str_eq(s, "t");
	b = rp_enum_map_label(emap, 10, &s);
	ck_assert_int_eq(1, (int)b);
	ck_assert_str_eq(s, "p");

	i = 999;
	b = rp_enum_map_value(emap, "x", &i);
	ck_assert_int_eq(0, (int)b);
	ck_assert_int_eq(i, 999);
	b = rp_enum_map_value(emap, "a", &i);
	ck_assert_int_eq(1, (int)b);
	ck_assert_int_eq(i, 1);
	b = rp_enum_map_value(emap, "y", &i);
	ck_assert_int_eq(1, (int)b);
	ck_assert_int_eq(i, 6);
	b = rp_enum_map_value(emap, "p", &i);
	ck_assert_int_eq(1, (int)b);
	ck_assert_int_eq(i, 10);

	s = rp_enum_map_label_def(emap, 0, any);
	ck_assert_ptr_eq(s, any);
	s = rp_enum_map_label_def(emap, 1, any);
	ck_assert_str_eq(s, "a");
	s = rp_enum_map_label_def(emap, 5, any);
	ck_assert_str_eq(s, "t");
	s = rp_enum_map_label_def(emap, 10, any);
	ck_assert_str_eq(s, "p");

	i = rp_enum_map_value_def(emap, "x", 888);
	ck_assert_int_eq(i, 888);
	i = rp_enum_map_value_def(emap, "a", 888);
	ck_assert_int_eq(i, 1);
	i = rp_enum_map_value_def(emap, "y", 888);
	ck_assert_int_eq(i, 6);
	i = rp_enum_map_value_def(emap, "p", 888);
	ck_assert_int_eq(i, 10);
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
	mksuite("enum-map");
		addtcase("enum-map");
			addtest(basics);
	return !!srun();
}
