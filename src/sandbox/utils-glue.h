/*
 * Copyright (C) 2015-2026 IoT.bzh Company
 * Author "Fulup Ar Foll"
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

#include <json-c/json.h>

typedef struct {
    const char *label;
    const int  value;
} nsKeyEnumT;

typedef struct {
    char *str;
    u_int8_t sep;
    size_t index;
} str2TokenT;

#define SCANDIR_FULLNAME    0x01
#define SCANDIR_BASENAME    0x02
#define SCANDIR_IS_DIR      0x04
#define SCANDIR_IS_FILE     0x08
#define SCANDIR_GET_ALL     0x10
#define SCANDIR_CALL_ALL    0x20

int utilLabel2Value (const nsKeyEnumT *keyvals, const char *label);
const char* utilValue2Label (const nsKeyEnumT *keyvals, const int value);

typedef int (*utilScanDirCbT)   (int dirFd, const char *fullname, const char *basename, uint32_t ftype, void *context);
typedef int (*utilScanJsonCbT) (json_object *sourceJ, void *context);

int utilScanJson (json_object *sourceJ, utilScanJsonCbT callback, void *context);

int utilScanDir (const char* path, const char *filter, utilScanDirCbT callback, uint32_t flags, void *context);
int utilMatchExtention (const char *relpath, const char *extention);
int utilFileLoad (const char *filepath, char **buffer);
int utilGunzip (uint8_t *dest, size_t *destLen, const uint8_t *source, size_t sourceLen);
char *utilStr2Token (str2TokenT *handle, u_int8_t separator, const char* data);
