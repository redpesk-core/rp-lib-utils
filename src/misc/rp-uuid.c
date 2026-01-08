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


#include "rp-uuid.h"

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if WITH_LIBUUID

#include <uuid/uuid.h>

#else

static void b2h(const unsigned char *uu, char *out, int count, char term)
{
	int x, y;

	do {
		x = *uu++;
		y = (x >> 4) & 15;
		*out++ = (char)(y < 10 ? ('0' + y) : (('a' - 10) + y));
		y = x & 15;
		*out++ = (char)(y < 10 ? ('0' + y) : (('a' - 10) + y));
	} while(--count);
	*out = term;
}

static void uuid_unparse_lower(const rp_uuid_binary_t uu, rp_uuid_stringz_t out)
{
	/* 01234567-9012-4567-9012-456789012345 */
	b2h(&uu[0], &out[0], 4, '-');
	b2h(&uu[4], &out[9], 2, '-');
	b2h(&uu[6], &out[14], 2, '-');
	b2h(&uu[8], &out[19], 2, '-');
	b2h(&uu[10], &out[24], 6, 0);
}

static int h2b(const char *uu, unsigned char *out, int count, char term)
{
	int x, y;

	do {
		x = *uu++;
		if ('0' <= x && x <= '9')
			x -= '0';
		else if ('a' <= x && x <= 'f')
			x -= 'a' - 10;
		else if ('A' <= x && x <= 'F')
			x -= 'A' - 10;
		else
			return 0;
		y = x << 4;
		x = *uu++;
		if ('0' <= x && x <= '9')
			x -= '0';
		else if ('a' <= x && x <= 'f')
			x -= 'a' - 10;
		else if ('A' <= x && x <= 'F')
			x -= 'A' - 10;
		else
			return 0;
		y |= x;
		*out++ = (char)y;
	} while(--count);
	return *uu == term;
}

static int uuid_parse(const rp_uuid_stringz_t from, rp_uuid_binary_t to)
{
	return (h2b(&from[0], &to[0], 4, '-')
	 && h2b(&from[9], &to[4], 2, '-')
	 && h2b(&from[14], &to[6], 2, '-')
	 && h2b(&from[19], &to[8], 2, '-')
	 && h2b(&from[24], &to[10], 6, 0)) - 1;
}

#endif

/**
 * generate a new fresh 'uuid'
 */
void rp_uuid_new_binary(rp_uuid_binary_t uuid)
{
#if defined(USE_UUID_GENERATE)
	uuid_generate(uuid);
#else
	static uint16_t pid;
	static uint16_t counter;

#if WITH_RANDOM_R
	static char state[32];
	static struct random_data rdata;
#define INIRND(x) do{ rdata.state = NULL; initstate_r((unsigned)(x), state, sizeof state, &rdata); }while(0)
#define GETRND(x) random_r(&rdata, (x))
#else
#define INIRND(x) srand((unsigned)(x))
#define GETRND(x) (*(x) = rand())
#endif
	int32_t x;

#if WITH_CLOCK_GETTIME
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	ts.tv_nsec ^= (long)ts.tv_sec;
	x = (int32_t)((int32_t)ts.tv_sec ^ (int32_t)ts.tv_nsec);
#else
	x = (int32_t)time(NULL);
#endif
	if (pid == 0) {
		pid = (uint16_t)getpid();
		counter = (uint16_t)(x);
		INIRND((((unsigned)pid) << 16) + (unsigned)counter);
	}
	if (++counter == 0)
		counter = 1;

	uuid[0] = (unsigned char)(x >> 24);
	uuid[1] = (unsigned char)(x >> 16);
	uuid[2] = (unsigned char)(x >> 8);
	uuid[3] = (unsigned char)(x);

	uuid[4] = (unsigned char)(pid >> 8);
	uuid[5] = (unsigned char)(pid);

	GETRND(&x);
	uuid[6] = (unsigned char)(((x >> 16) & 0x0f) | 0x40); /* pseudo-random version */
	uuid[7] = (unsigned char)(x >> 8);

	GETRND(&x);
	uuid[8] = (unsigned char)(((x >> 16) & 0x3f) | 0x80); /* variant RFC4122 */
	uuid[9] = (unsigned char)(x >> 8);

	GETRND(&x);
	uuid[10] = (unsigned char)(x >> 16);
	uuid[11] = (unsigned char)(x >> 8);

	GETRND(&x);
	uuid[12] = (unsigned char)(x >> 16);
	uuid[13] = (unsigned char)(x >> 8);

	uuid[14] = (unsigned char)(counter >> 8);
	uuid[15] = (unsigned char)(counter);
#undef INIRND
#undef GETRND
#endif
}

void rp_uuid_new_stringz(rp_uuid_stringz_t uuid)
{
	rp_uuid_binary_t newuuid;
	rp_uuid_new_binary(newuuid);
	uuid_unparse_lower(newuuid, uuid);
}

void rp_uuid_bin_to_text(const rp_uuid_binary_t from, rp_uuid_stringz_t to)
{
	uuid_unparse_lower(from, to);
}

int rp_uuid_text_to_bin(const char *from, rp_uuid_binary_t to)
{
	return !uuid_parse(from, (unsigned char*)to);
}

int rp_uuid_check_text(const char *text)
{
	rp_uuid_binary_t tmp;
	return rp_uuid_text_to_bin(text, tmp);
}
