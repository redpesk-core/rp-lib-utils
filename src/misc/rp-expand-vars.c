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


#include "rp-expand-vars.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#if !defined(RP_EXPAND_VARS_LIMIT)
#    define  RP_EXPAND_VARS_LIMIT           16384
#endif
#if !defined(RP_EXPAND_VARS_DEPTH_MAX)
#    define  RP_EXPAND_VARS_DEPTH_MAX       10
#endif
#if !defined(RP_EXPAND_VARS_CHAR)
#    define RP_EXPAND_VARS_CHAR             '$'
#endif

extern char **environ;

/**
 * Internal expansion of the variables of the given value
 * using values returned by function.
 *
 * @param value the value whose variables are to be expanded
 * @param function the name resolution function
 * @param closure the closure to give to the function
 *
 * @return NULL if no variable expansion was performed or if
 * memory allocation failed or if maximum recusion was reached
 */
static char *expand(const char *value, rp_expand_vars_fun_t function, void *closure)
{
	char *result, *write, *previous, c;
	const char *begin, *end;
	int drop, again, depth, found;
	size_t remove, add, i, len;
	rp_expand_vars_result_t expval;

	depth = 0;
	write = result = previous = NULL;
	begin = value;
	while (begin) {
		drop = again = 0;
		remove = add = 0;
		/* scan/expand the input */
		while ((c = *begin++)) {
			if (c != RP_EXPAND_VARS_CHAR) {
				/* not a variable to expand */
				if (write)
					*write++ = c;
			}
			else {
				/* search name of the variable to expand */
				switch(*begin) {
					case '(': c = ')'; break;
					case '{': c = '}'; break;
					default: c = 0; break;
				}
				if (c) {
					for (end = ++begin ; *end && *end != c ; end++);
					len = (size_t)(end - begin);
					if (*end) {
						end++;
						remove += 3 + len; /* length to remove */
					}
					else {
						remove += 2 + len; /* length to remove */
						drop = 1;
					}
				}
				else {
					for (end = begin ; isalnum(*end) || *end == '_' ; end++);
					len = (size_t)(end - begin);
					remove += 1 + len; /* length to remove */
				}
				/* search the value of the variable in vars and env */
				if (drop) {
					drop = 0;
				}
				else {
					expval.value = 0;
					expval.length = 0;
					expval.dispose.function = 0;
					expval.dispose.closure = 0;
					found = function(closure, begin, len, &expval);
					if (found && expval.value) {
						/* expand value of found variable */
						if (!expval.length)
							expval.length = strlen(expval.value);
						if (write) {
							memcpy(write, expval.value, expval.length);
							write += expval.length;
							again += NULL != memchr(expval.value, RP_EXPAND_VARS_CHAR, expval.length);
						}
						add += expval.length;
					}
					if (expval.dispose.function)
						expval.dispose.function(expval.dispose.closure);
				}
				begin = end;
			}
		}
		/* scan/expand done */
		if (again) {
			/* expansion done but must iterate */
			free(previous);
			begin = value = previous = result;
			*write = 0;
			result = write = NULL;
			if (++depth >= RP_EXPAND_VARS_DEPTH_MAX) {
				/* limit recursivity effect */
				begin = NULL;
			}
		}
		else if (write) {
			/* expansion done */
			*write = 0;
			begin = NULL;
		}
		else if (!remove) {
			/* no expansion to do after first scan */
			begin = NULL;
		}
		else {
			/* prepare expansion after scan */
			i = (size_t)(begin - value) + add - remove;
			if (i >= RP_EXPAND_VARS_LIMIT) {
				/* limit expansion size */
				begin = NULL;
			}
			else {
				result = write = malloc(i);
				begin = write ? value : NULL;
			}
		}
	}
	free(previous);
	return result;
}

struct adaptor
{
	rp_expand_vars_cb_t function;
	void *closure;
};

static int adaptor(void *closure, const char *name, size_t len, rp_expand_vars_result_t *result)
{
	struct adaptor *cfa = closure;
	result->value = cfa->function(cfa->closure, name, len);
	return result->value != NULL;
}

/**
 * Internal routine used to get variables from arrays
 */
static int getvar(void *closure, const char *name, size_t len, rp_expand_vars_result_t *result)
{
	char ***varsarray = closure;
	char **ivar;
	const char *res;

	for (ivar = *varsarray ; ivar ; ivar = *++varsarray) {
		res = rp_expand_vars_search(ivar, name, len);
		if (res) {
			result->value = res;
			return 1;
		}
	}
	return 0;
}

/* search in vars */
const char *rp_expand_vars_search(char **vars, const char *name, size_t len)
{
	char *var;

	for (var = *vars ; var ; var = *++vars) {
		if (!memcmp(var, name, len) && var[len] == '=')
			return &var[len + 1];
	}
	return NULL;
}

/* search in env */
const char *rp_expand_vars_search_env(const char *name, size_t len)
{
	return rp_expand_vars_search(environ, name, len);
}

/* expand using function */
char *rp_expand_vars_function(const char *value, int copy, rp_expand_vars_fun_t function, void *closure)
{
	char *expanded = expand(value, function, closure);
	return expanded ?: copy ? strdup(value) : 0;
}

/* expand using function */
char *rp_expand_vars_callback(const char *value, int copy, rp_expand_vars_cb_t function, void *closure)
{
	struct adaptor ada = { function, closure };
	char *expanded = expand(value, adaptor, &ada);
	return expanded ?: copy ? strdup(value) : 0;
}

/* expand using array of definitions */
char *rp_expand_vars_array(const char *value, int copy, char ***varsarray)
{
	return rp_expand_vars_function(value, copy, getvar, varsarray);
}

/* expand using variables definition */
char *rp_expand_vars_only(const char *value, int copy, char **vars)
{
	char **array[] = { vars, 0 };
	return rp_expand_vars_array(value, copy, array);
}

/* expand using environment */
char *rp_expand_vars_env_only(const char *value, int copy)
{
	char **array[] = { environ, 0 };
	return rp_expand_vars_array(value, copy, array);
}

/* expand using variables and environment */
char *rp_expand_vars(const char *value, int copy, char **before, char **after)
{
	char **array[] = { before, environ, after, 0 };
	return rp_expand_vars_array(value, copy, &array[!before]);
}

/* expand using variables and environment */
char *rp_expand_vars_first(const char *value, int copy, char **vars)
{
	char **array[] = { vars, environ, 0 };
	return rp_expand_vars_array(value, copy, &array[!vars]);
}

/* expand using variables and environment */
char *rp_expand_vars_last(const char *value, int copy, char **vars)
{
	char **array[] = { environ, vars, 0 };
	return rp_expand_vars_array(value, copy, array);
}
