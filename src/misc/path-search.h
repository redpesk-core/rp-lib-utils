/*
 * Copyright (C) 2020-2022 IoT.bzh Company
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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, something express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

struct path_search;

extern struct path_search *path_search_addref(struct path_search *paths);
extern void path_search_unref(struct path_search *paths);

extern int path_search_make_dirs(struct path_search **paths, const char *dirs);
extern int path_search_make_env(struct path_search **paths, const char *var);

extern int path_search_add_dirs(struct path_search **paths, const char *dirs, int before, struct path_search *other);
extern int path_search_add_env(struct path_search **paths, const char *var, int before, struct path_search *other);

extern int path_search_extend_dirs(struct path_search **paths, const char *dirs, int before);
extern int path_search_extend_env(struct path_search **paths, const char *var, int before);

extern void path_search_list(struct path_search *paths, void (*callback)(void*,const char*), void *closure);

#if WITH_DIRENT

/**
 * searching flags
 */
#define PATH_SEARCH_RECURSIVE   1
#define PATH_SEARCH_FILE        2
#define PATH_SEARCH_DIRECTORY   4
#define PATH_SEARCH_FLEXIBLE    8

/**
 * structure received by the callback
 */
struct path_search_item
{
  /** path to the item */
  const char *path;

  /** filename (last part of path) */
  const char *name;

  /** length of the path */
  short pathlen;

  /** length of the name */
  short namelen;

  /** is item a directory */
  short isDir;
};

extern void path_search_filter(struct path_search *paths, int flags, int (*callback)(void*,struct path_search_item*), void *closure, int (*filter)(void*,struct path_search_item*));
extern void path_search(struct path_search *paths, int flags, int (*callback)(void*,struct path_search_item*), void *closure);
extern int path_search_get_path(struct path_search *paths, char **path, int rec, const char *name, const char *extension);
extern int path_search_get_content(struct path_search *paths, char **content, int rec, const char *name, const char *extension);

#endif
