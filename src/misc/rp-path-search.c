/*
 * Copyright (C) 2020-2024 IoT.bzh Company
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

#include "rp-path-search.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include "rp-expand-vars.h"
#include "rp-file.h"

#include "../sys/x-errno.h"

#ifndef WITH_DIRENT
#  define WITH_DIRENT 1
#endif

#ifndef PATH_SEPARATOR_CHARACTER
#  define PATH_SEPARATOR_CHARACTER ':'
#endif
#ifndef DIRECTORY_SEPARATOR_CHARACTER
#  define DIRECTORY_SEPARATOR_CHARACTER '/'
#endif

/**
 * structure for searching files
 */
struct rp_path_search
{
	/** if not nul the path to search before or after */
	rp_path_search_t *parent;
	/** the reference count */
	unsigned short refcount;
	/**
	 * if positive length of the path or
	 * opposite of the length of the length
	 * When positive the path before
	 * When negative path after
	 * */
	short length;
	/** the path itself */
	char path[];
};

/* add one reference if not NULL */
rp_path_search_t *rp_path_search_addref(rp_path_search_t *paths) {
	if (paths != NULL)
		paths->refcount++;
	return paths;
}

/* remove one reference if not NULL */
void rp_path_search_unref(rp_path_search_t *paths) {
	while (paths != NULL && !--paths->refcount) {
		rp_path_search_t *parent = paths->parent;
		free(paths);
		paths = parent;
	}
}

/* check if the value must be added */
static int must_add(rp_path_search_t *paths, const char *value, int length, int before)
{
	for ( ; paths != NULL ; paths = paths->parent) {
		short plen = paths->length >= 0 ? paths->length : (short)-paths->length;
		if (plen == length && !memcmp(paths->path, value, (size_t)length))
			return 0;
		if (before)
			break;
	}
	return 1;
}

/* add an entry in front of the path list */
static int add(rp_path_search_t **paths, const char *value, int length, int before)
{
	rp_path_search_t *path;
	int rc = 0;

	/* check lengths */
	if ((unsigned)length > SHRT_MAX) {
		rc = X_EINVAL;
	}
	/* avoiding empty strings and duplications */
	else if (length > 0 && must_add(*paths, value, length, before)) {
		path = malloc(sizeof *path + 1 + (size_t)length);
		if (path == NULL)
			rc = X_ENOMEM;
		else {
			/* initialize */
			path->length = (short)(before ? length : -length);
			memcpy(path->path, value, (size_t)length);
			path->path[length] = 0;
			path->parent = *paths;
			path->refcount = 1;
			*paths = path;
		}
	}
	return rc;
}

/* make the search path from dirs */
static int make(rp_path_search_t **paths, const char *dirs, int before)
{
	int rc;
	const char *where;

	/* drop any leading collon */
	while (*dirs == PATH_SEPARATOR_CHARACTER)
		dirs++;
	/* search the first colon */
	for (where = dirs ; *where && *where != PATH_SEPARATOR_CHARACTER ; where++);
	if (!*where) {
		/* no colon just add it */
		rc = add(paths, dirs, (int)(where - dirs), before);
	}
	else {
		/* create recursively in direct or reverse order */
		rc = before ? make(paths, &where[1], before) : 0;
		rc = rc >= 0 ? add(paths, dirs, (int)(where - dirs), before) : rc;
		rc = rc >= 0 && !before ? make(paths, &where[1], before) : rc;
	}
	return rc;
}

/* Add 'dirs' before or after 'other' and returns it in 'paths'. */
static
int add_dir_expanded(
		rp_path_search_t **paths,
		const char *dirs,
		int before,
		rp_path_search_t *parent
) {
	int rc;
	rp_path_search_t *head;

	head = rp_path_search_addref(parent);
	rc = dirs == NULL ? X_EINVAL : make(&head, dirs, before);
	if (rc < 0) {
		/* case of error */
		rp_path_search_unref(head);
		head = NULL;
	}
	*paths = head;
	return rc;
}

/* creates a path list for dirs */
int rp_path_search_make_dirs(rp_path_search_t **paths, const char *dirs)
{
	return rp_path_search_add_dirs(paths, dirs, 1, NULL);
}

/* creates a path list for var */
int rp_path_search_make_env(rp_path_search_t **paths, const char *var)
{
	return rp_path_search_add_env(paths, var, 1, NULL);
}

/* add a path list for dir */
int rp_path_search_add_dirs(rp_path_search_t **paths, const char *dirs, int before, rp_path_search_t *other)
{
	char *ex = rp_expand_vars_env_only(dirs, 0);
	int rc = add_dir_expanded(paths, ex ?: dirs, before, other);
	free(ex);
	return rc;
}

/* add a path list for var */
int rp_path_search_add_env(rp_path_search_t **paths, const char *var, int before, rp_path_search_t *other)
{
	const char *value = getenv(var);
	if (value)
		return rp_path_search_add_dirs(paths, value, before, other);

	*paths = rp_path_search_addref(other);
	return 0;
}

/* add extend the path list with dir */
int rp_path_search_extend_dirs(rp_path_search_t **paths, const char *dirs, int before)
{
	int rc;
	rp_path_search_t *p;

	rc = rp_path_search_add_dirs(&p, dirs, before, *paths);
	if (rc >= 0) {
		rp_path_search_unref(*paths);
		*paths = p;
	}
	return rc;
}

/* add extend the path list with env */
int rp_path_search_extend_env(rp_path_search_t **paths, const char *var, int before)
{
	int rc;
	rp_path_search_t *p;

	rc = rp_path_search_add_env(&p, var, before, *paths);
	if (rc >= 0) {
		rp_path_search_unref(*paths);
		*paths = p;
	}
	return rc;
}

/* list the path of the path's list */
int rp_path_search_list(rp_path_search_t *paths, rp_path_search_cb callback, void *closure)
{
	int result = 0;
	if (paths->length > 0)
		result = callback(closure, paths->path, (size_t)paths->length);
	if (result == 0 && paths->parent)
		result = rp_path_search_list(paths->parent, callback, closure);
	if (result == 0 && paths->length < 0)
		result = callback(closure, paths->path, (size_t)-paths->length);
	return result;
}

/**
* structure for locating file
*/
struct locate {
	/** preallocated buffer ending with filename */
	char     filename[PATH_MAX + 1];
	/** remaining chars for prepending directory */
	size_t   remains;
	/** callback to call */
	rp_path_search_cb callback;
	/** closure of the callback */
	void *closure;
};

/**
* find or read callback
*/
static int find_or_read_cb(void *closure, const char *path, size_t length)
{
	struct locate *loc = closure;
	size_t pos;
	char *filename;
	int rc = 0;

	/* normally a filename must fit the buffer */
	if (length <= loc->remains) {
		/* build the filename */
		pos = loc->remains - length;
		filename = &loc->filename[pos];
		memcpy(filename, path, length);
		/* check access */
		if (0 == access(filename, F_OK))
			rc = loc->callback(loc->closure, filename, sizeof loc->filename - 1 - pos);
	}
	return rc;
}

/* find files of filename */
int rp_path_search_find(rp_path_search_t *paths, const char *filename, rp_path_search_cb callback, void *closure)
{
	struct locate loc;
	size_t len;

	/* check parameter */
	if (filename == NULL)
		return X_EINVAL;

	len = strlen(filename);
	if (len == 0 || len + 1 >= sizeof loc.filename)
		return X_EINVAL;

	/* init filename buffer */
	loc.remains = sizeof loc.filename - len - 2;
	memcpy(&loc.filename[loc.remains + 1], filename, len + 1);
	loc.filename[loc.remains] = DIRECTORY_SEPARATOR_CHARACTER;

	/* call exploration */
	loc.callback = callback;
	loc.closure = closure;
	return rp_path_search_list(paths, find_or_read_cb, &loc);
}

#if !WITH_DIRENT

static void nothing() {}
static int zero() { return 0; }
int rp_path_search_can_list_entries()
	__attribute__((alias("zero")));
void rp_path_search_filter(rp_path_search_t *paths, int flags,
		rp_path_search_item_cb callback, void *closure, rp_path_search_filter_cb filter)
	__attribute__((alias("nothing")));
void rp_path_search(rp_path_search_t *paths, int flags,
		rp_path_search_item_cb callback, void *closure)
	__attribute__((alias("nothing")));
int rp_path_search_get_path(rp_path_search_t *paths, char **path,
		int rec, const char *name, const char *extension)
	__attribute__((alias("zero")));
int rp_path_search_get_content(rp_path_search_t *paths, char **content,
		int rec, const char *name, const char *extension)
	__attribute__((alias("zero")));

#else

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

/**
 * back link to detect loops
 */
struct direntlist {
	/** the directory entry */
	struct dirent *ent;
	/** the previous entry in the list */
	struct direntlist *previous;
};

/**
 * search data
 */
struct search {
	/** Original search object */
	rp_path_search_t *paths;

	/** flags of the search */
	int flags;

	/** Callback to be invoked for each entry found */
	rp_path_search_item_cb callback;

	/** Closure of the callback */
	void *callback_closure;

	/** Callback to be invoked before entring a directory */
	rp_path_search_filter_cb filter;

	/** Closure of the filter */
	void *filter_closure;

	/** Item passed to the callback */
	rp_path_search_entry_t entry;

	/** path of the currently scanned file */
	char path[PATH_MAX];
};

/**
 * recursive search
 *
 * @param search the search structure
 */
static int search_in_dir(struct search *search, DIR *dir, struct direntlist *previous, int flags)
{
	struct direntlist dl, *idl;
	struct stat st;
	short pos, len;
	short oplen, onlen;
	int stop, type;
	char *name;
	DIR *subdir;

	/* manage recursive search */
	if (flags >= RP_PATH_SEARCH_DEPTH_BASE) {
		flags -= RP_PATH_SEARCH_DEPTH_BASE;
		if (flags > RP_PATH_SEARCH_DEPTH_BASE)
			flags |= RP_PATH_SEARCH_RECURSIVE;
		else
			flags &= ~RP_PATH_SEARCH_RECURSIVE;
	}

	/* entering the directory */
	stop = 0;
	search->entry.isDir = 1;
	search->entry.action = rp_path_search_directory_before;
	if ((flags & RP_PATH_SEARCH_DIRECTORY)
		&& (!search->filter || search->filter(search->filter_closure, &search->entry))) {
		/* regular file, use the callback */
		stop = search->callback(search->callback_closure, &search->entry);
	}

	search->entry.action = rp_path_search_directory_enter;
	if (!stop && (flags & RP_PATH_SEARCH_RECURSIVE)
		&& (!(flags & RP_PATH_SEARCH_DIRECTORY)
			|| !search->filter
			|| search->filter(search->filter_closure, &search->entry))) {

		onlen = search->entry.namelen;
		oplen = search->entry.pathlen;

		/* prepare to name files in path */
		dl.previous = previous;
		pos = search->entry.pathlen;
		if (pos && search->path[pos - 1] != DIRECTORY_SEPARATOR_CHARACTER)
			search->path[pos++] = DIRECTORY_SEPARATOR_CHARACTER;
		name = &search->path[pos];

		/* loop on each entry */
		while (!stop && (dl.ent = readdir(dir))) {

			/* detection of loops */
			for (idl = previous ; idl != NULL && idl->ent->d_ino != dl.ent->d_ino ; idl = idl->previous);
			if (idl)
				continue;

			/* copy name if no overflow */
			len = (short)strlen(dl.ent->d_name);
			if (len + pos >= (int)sizeof(search->path)) {
				/* overflow detected */
				continue;
			}
			memcpy(name, dl.ent->d_name, 1 + (size_t)len);

			/* get type after dereferencing if link */
			type = dl.ent->d_type;
			if (type == DT_LNK && stat(search->path, &st) == 0) {
				switch (st.st_mode & S_IFMT) {
				case S_IFREG:
					type = DT_REG;
					break;
				case S_IFDIR:
					type = DT_DIR;
					break;
				default:
					break;
				}
			}

			/* prepare entry */
			search->entry.name = name;
			search->entry.namelen = len;
			search->entry.pathlen = (short)(pos + len);

			/* inspect type */
			if (type == DT_REG) {
				search->entry.isDir = 0;
				search->entry.action = rp_path_search_file;
				if ((flags & RP_PATH_SEARCH_FILE)
					&& (!search->filter || search->filter(search->filter_closure, &search->entry))) {
					/* regular file, use the callback */
					search->entry.isDir = 0;
					stop = search->callback(search->callback_closure, &search->entry);
				}
			}
			else if (type == DT_DIR && (name[0] != '.' || (len > 1 && (name[1] != '.' || len > 2)))) {
				subdir = opendir(search->path);
				if (subdir != NULL)
					stop = search_in_dir(search, subdir, &dl, flags);
			}
		}
		search->entry.namelen = onlen;
		search->entry.pathlen = oplen;
	}

	search->entry.action = rp_path_search_directory_after;
	if (!stop && (flags & RP_PATH_SEARCH_DIRECTORY)
		&& search->filter
		&& search->filter(search->filter_closure, &search->entry)) {
		/* regular file, use the callback */
		stop = search->callback(search->callback_closure, &search->entry);
	}

	closedir(dir);
	return stop;
}

static int searchcb(void *closure, const char *path, size_t length)
{
	struct search *search = closure;
	int stop;
	DIR *dir;
	struct stat st;

	search->entry.pathlen = (short)length;
	memcpy(search->path, path, 1 + length);
	search->entry.namelen = 0;

	/* init and open directory */
	stop = 0;
	dir = opendir(search->path);
	if (dir)
		stop = search_in_dir(search, dir, NULL, search->flags);
	else if ((search->flags & (RP_PATH_SEARCH_FLEXIBLE|RP_PATH_SEARCH_FILE)) == (RP_PATH_SEARCH_FLEXIBLE|RP_PATH_SEARCH_FILE)
	      && stat(search->path, &st) == 0
	      && (st.st_mode & S_IFMT) == S_IFREG) {
		/* flexible and regular file, use the callback */
		search->entry.isDir = 0;
		search->entry.action = rp_path_search_file;
		stop = search->callback(search->callback_closure, &search->entry);
	}

	return stop;
}

int rp_path_search_can_list_entries()
{
	return 1;
}

int rp_path_search_filter(
	rp_path_search_t *paths,
	int flags,
	rp_path_search_item_cb callback,
	void *callback_closure,
	rp_path_search_filter_cb filter,
	void *filter_closure
) {
	struct search search = {
		.paths = paths,
		.flags = flags & (RP_PATH_SEARCH_FILE|RP_PATH_SEARCH_DIRECTORY) ? flags : flags|RP_PATH_SEARCH_FILE,
		.callback = callback,
		.callback_closure = callback_closure,
		.filter = filter,
		.filter_closure = filter_closure,
		.entry.pathlen = 0,
		.entry.path = search.path
	};

	return rp_path_search_list(paths, searchcb, &search);
}

int rp_path_search(rp_path_search_t *paths, int flags, rp_path_search_item_cb callback, void *closure)
{
	return rp_path_search_filter(paths, flags, callback, closure, NULL, NULL);
}

/* for search filenames */
struct matchname {
	size_t len_extension;
	const char *extension;
	size_t len_filename;
	const char *filename;
};

/** simple callback for matching the name */
static int matchnamecb(void *parameter, const rp_path_search_entry_t *entry)
{
	const struct matchname *mn = parameter;
	size_t len = entry->namelen;
	if (mn->len_extension != 0) {
		if (len < mn->len_extension)
			return 0;
		len -= mn->len_extension;
		if (memcmp(mn->extension, &entry->name[len], mn->len_extension) != 0)
			return 0;
		if (mn->extension[0] != '.' && len && entry->name[len - 1] == '.')
			len--;
	}
	if (mn->len_filename != 0) {
		if (len != mn->len_filename)
			return 0;
		if (memcmp(mn->filename, entry->name, mn->len_filename) != 0)
			return 0;
	}
	return 1;
}

int rp_path_search_match(rp_path_search_t *paths, int flags, const char *name, const char *extension, rp_path_search_item_cb callback, void *closure)
{
	struct matchname mn = {
		extension == NULL ? 0 : strlen(extension),
		extension,
		name == NULL ? 0 : strlen(name),
		name
	};
	return rp_path_search_filter(paths, flags, callback, closure, matchnamecb, &mn);
}

#endif
