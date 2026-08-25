
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../src/misc/rp-base64.h"

#include <check.h>
#if !defined(ck_assert_ptr_null)
# define ck_assert_ptr_null(X)      ck_assert_ptr_eq(X, NULL)
# define ck_assert_ptr_nonnull(X)   ck_assert_ptr_ne(X, NULL)
#endif

const char *base[][2] = {
        { "a", "YQ==" },
        { "az", "YXo=" },
        { "aze", "YXpl" },
        { "azer", "YXplcg==" },
        { "azert", "YXplcnQ=" },
        { "azerty", "YXplcnR5" },
        { "azertyu", "YXplcnR5dQ==" },
        { "azertyui", "YXplcnR5dWk=" },
        { "azertyuio", "YXplcnR5dWlv" },
        { "azertyuiop", "YXplcnR5dWlvcA==" },
};

void test(int (*fun)(const char*, char**), int b)
{
	int i = 0, n = sizeof base / sizeof *base;
	while(i < n) {
		char *r;
		int c;
		c = fun(base[i][b], &r);
		printf("%s -> %s\n", base[i][b], r);
		ck_assert_int_eq(c, rp_base64_ok);
		ck_assert_ptr_nonnull(&r);
		ck_assert_str_eq(r, base[i][1-b]);
		free(r);
		i++;
	}
}

int enc(const char *from, char **to)
{
	size_t len;
	return rp_base64_encode(
			(const uint8_t*)from, strlen(from),
			to, &len,
			0, 1, 0);
}

int dec(const char *from, char **to)
{
	size_t len;
	return rp_base64_decode(
			from, strlen(from),
			(uint8_t**)to, &len,
			0);
}

START_TEST (encode)
{
	test(enc, 0);
}

START_TEST (decode)
{
	test(dec, 1);
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
	mksuite("base64");
		addtcase("base64");
			addtest(encode);
			addtest(decode);
	return !!srun();
}
