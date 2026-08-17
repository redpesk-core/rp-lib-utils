
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include <json-c/json.h>

#include "../src/json/rp-jsonc.h"

#include <check.h>
#if !defined(ck_assert_ptr_null)
# define ck_assert_ptr_null(X)      ck_assert_ptr_eq(X, NULL)
# define ck_assert_ptr_nonnull(X)   ck_assert_ptr_ne(X, NULL)
#endif

char buffer[100];

void bin_u_max(unsigned bits, char *buf, size_t len)
{
	if (3 + bits + !bits  > len)
		exit(1);
	*buf++ = '0';
	*buf++ = 'b';
	if (!bits)
		*buf++ = '0';
	else
		while(bits--)
			*buf++ = '1';
	*buf = 0;
}

void ts(unsigned b, int rce, char *b0, const char *fmt, int64_t u)
{
	int rc;
	int64_t ou;
	json_object *obj;

	sprintf(b0, fmt, (uint64_t)-u);
	obj = json_object_new_string(buffer);
	rc = rp_jsonc_get_int64(obj, &ou, rp_jsonc_int_mode_number);
	printf("%u: %d %"PRIi64" %s\n", b, rc, ou, buffer);
	ck_assert_int_eq(rc, rce);
	ck_assert_int_eq(u, ou);
}

void tests(char char0)
{
	json_object *obj;
	int64_t u, ou;
	int rc;
	unsigned b;
	char *b0 = buffer + !!char0;
	size_t lb = sizeof buffer - !!char0;
	int rce = 1;

	buffer[0] = char0;
	for(b = 0 ; b <= 65 ; b++) {
		bin_u_max(b, b0, lb);
		obj = json_object_new_string(buffer);
		rc = rp_jsonc_get_int64(obj, &ou, rp_jsonc_int_mode_number);
		printf("%u: %d %"PRIi64" %s\n", b, rc, ou, buffer);
		if (rc == 0) {
			ck_assert_uint_ge(b, 64);
		}
		else {
			u = -(int64_t)((((int64_t)1) << b) - 1);
			ck_assert_int_eq(rc, rce);
			ck_assert_int_eq(u, ou);

			ts(b, rce, b0, "0o%"PRIo64, u);
			ts(b, rce, b0, "0O%"PRIo64, u);
			ts(b, rce, b0, "0%"PRIo64, u);
			ts(b, rce, b0, "0x%"PRIx64, u);
			ts(b, rce, b0, "0X%"PRIX64, u);
			ts(b, rce, b0, "%"PRIu64, u);
			ts(b, rce, b0, "0d%"PRIu64, u);
			ts(b, rce, b0, "0D%"PRIu64, u);
		}
	}
}

void tu(unsigned b, int rce, char *b0, const char *fmt, uint64_t u)
{
	int rc;
	uint64_t ou;
	json_object *obj;

	sprintf(b0, fmt, u);
	obj = json_object_new_string(buffer);
	rc = rp_jsonc_get_uint64(obj, &ou, rp_jsonc_int_mode_number);
	printf("%u: %d %"PRIu64" %s\n", b, rc, ou, buffer);
	ck_assert_int_eq(rc, rce);
	ck_assert_uint_eq(u, ou);
}

void testu(char char0)
{
	json_object *obj;
	uint64_t u, ou;
	int rc;
	unsigned b;
	char *b0 = buffer + !!char0;
	size_t lb = sizeof buffer - !!char0;
	int rce = 1;

	buffer[0] = char0;
	for(b = 0 ; b <= 65 ; b++) {
		bin_u_max(b, b0, lb);
		obj = json_object_new_string(buffer);
		rc = rp_jsonc_get_uint64(obj, &ou, rp_jsonc_int_mode_number);
		printf("%u: %d %"PRIu64" %s\n", b, rc, ou, buffer);
		if (rc == 0) {
			ck_assert_uint_ge(b, 65);
		}
		else {
			u = (uint64_t)((uint64_t)(b < 64 ? (((uint64_t)1) << b) : 0) - 1);
			ck_assert_int_eq(rc, rce);
			ck_assert_uint_eq(u, ou);

			tu(b, rce, b0, "0o%"PRIo64, u);
			tu(b, rce, b0, "0O%"PRIo64, u);
			tu(b, rce, b0, "0%"PRIo64, u);
			tu(b, rce, b0, "0x%"PRIx64, u);
			tu(b, rce, b0, "0X%"PRIX64, u);
			tu(b, rce, b0, "%"PRIu64, u);
			tu(b, rce, b0, "0d%"PRIu64, u);
			tu(b, rce, b0, "0D%"PRIu64, u);
		}
	}
}

START_TEST (str2int)
{
	testu(0);
}

START_TEST (str2int_plus)
{
	testu('+');
}

START_TEST (str2int_minus)
{
	tests('-');
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
	mksuite("str2int");
		addtcase("str2int");
			addtest(str2int);
			addtest(str2int_plus);
			addtest(str2int_minus);
	return !!srun();
}
