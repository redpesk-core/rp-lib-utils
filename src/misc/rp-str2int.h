/*
 Copyright (C) 2015-2025 IoT.bzh Company

 Author: José Bollo <jose.bollo@iot.bzh>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is furnished
 to do so, subject to the following conditions:

 The above copyright notice and this permission notice (including the next
 paragraph) shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#pragma once

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * Get the 64 bits absolute value of the string 'str'. It interprets strings
 * beginning with 0o, 0, 0x, 0b, 0d, [1-9] as  octal, octal, hexa, binary,
 * decimal, decimal representation of the expected number.
 *
 * @param str the string whose value is expected
 * @param value pointer to the uint64 receiving the absolute value
 *
 * @return 1 if successfully matching a positive number
 *      or -1 if successfully matching a negative number
 *      or 0 when object doesn't match a number or a 64 bits value
 */
extern int rp_str2u64(const char *str, uint64_t *value);

/**
 * Get the 64 bits signed integer value of the string 'str'. It interprets strings
 * beginning with 0o, 0, 0x, 0b, 0d, [1-9] as  octal, octal, hexa, binary,
 * decimal, decimal representation of the expected number.
 *
 * @param str the string whose value is expected
 * @param value pointer to the int64 receiving the value
 *
 * @return 1 if successful
 *      or 0 when object doesn't match a number or a 64 bits signed integer
 */
extern int rp_str2int64(const char *str, int64_t *value);

/**
 * Get the 64 bits unsigned integer value of the string 'str'. It interprets strings
 * beginning with 0o, 0, 0x, 0b, 0d, [1-9] as  octal, octal, hexa, binary,
 * decimal, decimal representation of the expected number.
 *
 * @param str the string whose value is expected
 * @param value pointer to the uint64 receiving the value
 *
 * @return 1 if successful
 *      or 0 when object doesn't match a number or a 64 bits unsigned integer
 */
extern int rp_str2uint64(const char *str, uint64_t *value);

/**
 * Get the 32 bits signed integer value of the string 'str'. It interprets strings
 * beginning with 0o, 0, 0x, 0b, 0d, [1-9] as  octal, octal, hexa, binary,
 * decimal, decimal representation of the expected number.
 *
 * @param str the string whose value is expected
 * @param value pointer to the int32 receiving the value
 *
 * @return 1 if successful
 *      or 0 when object doesn't match a number or a 32 bits signed integer
 */
extern int rp_str2int32(const char *str, int32_t *value);

/**
 * Get the 32 bits unsigned integer value of the string 'str'. It interprets strings
 * beginning with 0o, 0, 0x, 0b, 0d, [1-9] as  octal, octal, hexa, binary,
 * decimal, decimal representation of the expected number.
 *
 * @param str the string whose value is expected
 * @param value pointer to the uint32 receiving the value
 *
 * @return 1 if successful
 *      or 0 when object doesn't match a number or a 32 bits unsigned integer
 */
extern int rp_str2uint32(const char *str, uint32_t *value);

#ifdef	__cplusplus
}
#endif

