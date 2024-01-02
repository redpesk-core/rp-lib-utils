/*
 Copyright (C) 2015-2024 IoT.bzh Company

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


#include <stdlib.h>

#include "subpath.h"

/* a valid subpath is a relative path not looking deeper than root using .. */
int subpath_is_valid(const char *path)
{
	int l = 0, i = 0;

	/* absolute path is not valid */
	if (path[i] == '/')
		return 0;

	/* inspect the path */
	while(path[i]) {
		switch(path[i++]) {
		case '.':
			if (!path[i])
				break;
			if (path[i] == '/') {
				i++;
				break;
			}
			if (path[i++] == '.') {
				if (!path[i]) {
					l--;
					break;
				}
				if (path[i++] == '/') {
					l--;
					break;
				}
			}
		default:
			while(path[i] && path[i] != '/')
				i++;
			if (l >= 0)
				l++;
		case '/':
			break;
		}
	}
	return l >= 0;
}

/*
 * Return the path or NULL is not valid.
 * Ensure that the path doesn't start with '/' and that
 * it does not contains sequence of '..' going deeper than
 * root.
 * Returns the path or NULL in case of
 * invalid path.
 */
const char *subpath(const char *path)
{
	return path && subpath_is_valid(path) ? (path[0] ? path : ".") : NULL;
}

/*
 * Normalizes and checks the 'path'.
 * Removes any starting '/' and checks that 'path'
 * does not contains sequence of '..' going deeper than
 * root.
 * Returns the normalized path or NULL in case of
 * invalid path.
 */
const char *subpath_force(const char *path)
{
	while(path && *path == '/')
		path++;
	return subpath(path);
}

#if defined(TEST_subpath)
#include <stdio.h>
void t(const char *subpath, int validity) {
  printf("%s -> %d = %d, %s\n", subpath, validity, subpath_is_valid(subpath), subpath_is_valid(subpath)==validity ? "ok" : "NOT OK");
}
int main() {
  t("/",0);
  t("..",0);
  t(".",1);
  t("../a",0);
  t("a/..",1);
  t("a/../////..",0);
  t("a/../b/..",1);
  t("a/b/c/..",1);
  t("a/b/c/../..",1);
  t("a/b/c/../../..",1);
  t("a/b/c/../../../.",1);
  t("./..a/././..b/..c/./.././.././../.",1);
  t("./..a/././..b/..c/./.././.././.././..",0);
  t("./..a//.//./..b/..c/./.././/./././///.././.././a/a/a/a/a",1);
  return 0;
}
#endif

