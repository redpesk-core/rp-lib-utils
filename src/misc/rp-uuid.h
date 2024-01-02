/*
 * Copyright (C) 2015-2024 IoT.bzh Company
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

#pragma once

/**
 * @brief length of binary buffer for UUID in bytes
 */
#define RP_UUID_BINARY_LENGTH   16

/**
 * @brief length of text buffer for UUID in ASCII-chars (including tailing zero)
 */
#define RP_UUID_STRINGZ_LENGTH  37

/**
 * @brief buffer type for holding binary UUID
 */
typedef unsigned char rp_uuid_binary_t[RP_UUID_BINARY_LENGTH];

/**
 * @brief buffer type for holding text UUID
 */
typedef char rp_uuid_stringz_t[RP_UUID_STRINGZ_LENGTH];

/**
 * @brief creates a new binary UUID
 *
 * @param uuid buffer for storing the UUID
 */
void rp_uuid_new_binary(rp_uuid_binary_t uuid);

/**
 * @brief creates a new text UUID
 *
 * @param uuid buffer for storing the UUID
 */
void rp_uuid_new_stringz(rp_uuid_stringz_t uuid);

/**
 * @brief translate binary UUID to text UUID
 *
 * @param from  buffer of the binary UUID
 * @param to    buffer for storing the text UUID
 */
void rp_uuid_bin_to_text(const rp_uuid_binary_t from, rp_uuid_stringz_t to);

/**
 * @brief translate text UUID to binary UUID
 *
 * @param from  buffer of the text UUID
 * @param to    buffer for storing the binary UUID
 *
 * @return 1 on success or 0 on error
 */
int rp_uuid_text_to_bin(const char * from, rp_uuid_binary_t to);

/**
 * @brief check if a text is an text UUID
 *
 * @param text the text to check
 *
 * @return 1 on success or 0 on error
 */
int rp_uuid_check_text(const char *text);
