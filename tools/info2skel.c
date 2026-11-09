/*
 * Copyright (C) 2016-2026 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

#include <json-c/json.h>

#include "d2c.c"
#include "rp-jsonc-path.h"
#include "rp-jsonc-default-expand.h"

#define VERSION "0.1"
#define PROG "info2skel"
#define TEST(x)   ((x) && *(x))

#define OOM(x) do{if(!(x)){fprintf(stderr,"out of memory\n");exit(1);}}while(0)

/**
 * root of the JSON being parsed
 */
int version = 4;
int version_set = 1;
struct json_object *root = NULL;
struct json_object *options = NULL;
struct json_object *d_perms = NULL;
struct json_object *a_perms = NULL;
const char *mainctl = NULL;
const char *api = NULL;
const char *scope = NULL;
const char *noexcpt = NULL;
const char *prefix = NULL;
const char *postfix = NULL;
const char *provideclass = NULL;
const char *requireclass = NULL;
const char *requireapi = NULL;
char *capi = NULL;
int priv = -1;
int noconc = -1;
int cpp = 0;
int cpp_set = 1;
int declare = 0;
enum idl {
	idl_unset = 0,
	idl_info2 = 2
} idl = idl_unset;

/* create c name by replacing non alpha numeric characters with underscores */
char *cify(const char *str)
{
	char *r = strdup(str);
	int i = 0;
	while (r && r[i]) {
		if (!isalnum(r[i]))
			r[i] = '_';
		i++;
	}
	return r;
}

int get(void *data, uint8_t *val)
{
	const char **pstr = data;
	char c = **pstr;
	if (c == 0)
		return 0;
	*val = (uint8_t)c;
	(*pstr)++;
	return 1;
}

struct put { char *base; size_t length; };

int put(void *data, uint8_t val)
{
	struct put *ps = data;
	if ((ps->length & 255) == 0) {
		char *s = realloc(ps->base, ps->length + 256);
		OOM(s);
		ps->base = s;
	}
	ps->base[ps->length++] = (char)val;
	return 0;
}

int putend(struct put *ps)
{
	char *s = realloc(ps->base, ps->length + 1);
	OOM(s);
	ps->base = s;
	ps->base[ps->length] = 0;
	return 0;
}

char *str2c(const char *str, const char *prefix, int width)
{
	struct put ps = { NULL, 0 };
	int rc = d2c(get, (void*)&str, put, &ps, prefix, width);
	if (rc >= 0)
		rc = putend(&ps);
	return ps.base;
}

char *json2c_std(struct json_object *obj)
{
	const char *str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_NOSLASHESCAPE);
	return str2c(str, "    ", 75);
}

/* this function advance the cursor to the beginning of the next token
 * (it skips spaces) and return the length of the pointed token if any
 * or zero if at end */
unsigned parse_token(const char **scan)
{
	const char *p = *scan;
	/* skip spaces */
	while (p[0] == ' ') p++;
	/* update the head of parse */
	*scan = p;
	/* compute length according to (, ), >, >= */
	switch (p[0]) {
	case 0:
		return 0;
	case '(':
	case ')':
		return 1;
	case '>':
		return p[1] == '=' ? 2 : 1;
	default:
		for(;;) {
			switch (*++p) {
			case 0:
			case '(':
			case ')':
			case '>':
			case ' ':
				return (unsigned)(p - *scan);
			}
		}
	}
}

/* get the current token and check if it matches the given value
 * if it fits, consume it and returns true
 * otherwise, leave the token as is and return false */
bool parse_is(const char **scan, const char *value)
{
	unsigned i, len = parse_token(scan);
	const char *p = *scan;
	/* compute in i the longest match */
	for (i = 0 ; i < len && p[i] == value[i] ; i++);
	/* check if fitting using the length of the match */
	if (i < len || value[len])
		return false;
	/* consume the token */
	*scan = p + len;
	return true;
}

/* returns the json object for { tag: obj } */
struct json_object *make_single(const char *tag, struct json_object *obj)
{
	struct json_object *resu = json_object_new_object();
	json_object_object_add(resu, tag, obj);
	return resu;
}

/* common routine for and and or, it matches list of
 *     EXPR [ key EXPR ]...
 * where EXPR is the result of calling 'fun'
 * When only one EXPR is found, it is returned
 * other wise { "tag": [ EXPR ... ] } is returned */
struct json_object *parse_info_auth_list(
		const char **scan,
		const char *key,
		const char *tag,
		struct json_object *(*fun)(const char **)
) {
	struct json_object *array, *obj;

	/* get first EXPR */
	obj = fun(scan);
	if (obj == NULL)
		return NULL; /* error */

	/* first follow by key ? */
	if (!parse_is(scan, key))
		return obj; /* no, return it */

	/* create the array listing the expressions */
	array = json_object_new_array();
	json_object_array_add(array, obj);
	do {
		obj = fun(scan);
		if (obj == NULL)
			return NULL; /* error */
		json_object_array_add(array, obj);
	}
	while (parse_is(scan, key));

	/* pack it with tag */
	return make_single(tag, array);
}

struct json_object *parse_info_auth_ors(const char **scan);

/* */
struct json_object *parse_info_auth_term(const char **scan)
{
	const char *tag = NULL;
	struct json_object *obj = NULL;
	if (parse_is(scan, "(")) {
		obj = parse_info_auth_ors(scan);
		if (obj != NULL && !parse_is(scan, ")")) {
			fprintf(stderr, "missing )\n");
			obj = NULL;
		}
	}
	else if (parse_is(scan, "not")) {
		obj = parse_info_auth_term(scan);
		tag = "not";
	}
	else if (parse_is(scan, "check-token")) {
		obj = json_object_new_string("check");
		tag = "session";
	}
	else if (parse_is(scan, "loa")) {
		int inc = -1;
		if (parse_is(scan, ">="))
			inc = 0;
		else if (parse_is(scan, ">"))
			inc = 1;
		if (inc < 0)
			fprintf(stderr, "missing > or >= after loa\n");
		else {
			unsigned len = parse_token(scan);
			if (len == 0)
				fprintf(stderr, "unexpected end\n");
			else {
				char *end;
				int val = (int)strtol(*scan, &end, 10);
				if (len != (unsigned)(end - *scan))
					fprintf(stderr, "integer value expected\n");
				else {
					val += inc;
					if (val < 0 || val > 3)
						fprintf(stderr, "invalid loa value %d\n", val - inc);
					else {
						*scan = end;
						obj = json_object_new_int(val);
						tag = "LOA";
					}
				}
			}
		}
	}
	else if (**scan == ')' || **scan == '>') {
		fprintf(stderr, "unexpected character %c", **scan);
	}
	else {
		unsigned len = parse_token(scan);
		if (len == 0)
			fprintf(stderr, "unexpected end\n");
		else {
			obj = json_object_new_string_len(*scan, (int)len);
			*scan += len;
			tag = "permission";
		}
	}
	return obj == NULL ? NULL : tag == NULL ? obj : make_single(tag, obj);
}

/* parse conjonction sequence */
struct json_object *parse_info_auth_ands(const char **scan)
{
	return parse_info_auth_list(scan, "and", "allOf", parse_info_auth_term);
}

/* parse disjonction sequence */
struct json_object *parse_info_auth_ors(const char **scan)
{
	return parse_info_auth_list(scan, "or", "anyOf", parse_info_auth_ands);
}

/* create a permission object from an auth description */
struct json_object *parse_info_auth(const char *auth)
{
	const char *scan = auth;
	struct json_object *resu = parse_info_auth_ors(&scan);
	if (resu != NULL && parse_token(&scan) > 0) {
		fprintf(stderr, "extra items detected: %s\n", scan);
		resu = NULL;
	}
	if (resu == NULL) {
		fprintf(stderr, "error while parsing auth string:\n");
		fprintf(stderr, "\"%s\"\n ", auth);
		while(scan != auth++) fprintf(stderr, " ");
		fprintf(stderr, "^\n");
		exit(EXIT_FAILURE);
	}
	return resu;
}

/* get the permission odescription if set */
struct json_object *permissions_of_verb(struct json_object *obj)
{
	struct json_object *x;

	if (idl == idl_info2) {
		if (!json_object_object_get_ex(obj, "auth", &x))
			return NULL;
		if (json_object_is_type(x, json_type_string)) {
			x = parse_info_auth(json_object_get_string(x));
			json_object_object_add(obj, "auth", x);
		}
		return x;
	}

	return NULL;
}

/* output the array of permissions */
void print_perms()
{
	size_t i, n;

	n = a_perms ? json_object_array_length(a_perms) : 0;
	if (n) {
		printf("static const struct afb_auth _afb_auths_%s[] = {\n" , capi);
		i = 0;
		while (i < n) {
			const char *perm = json_object_get_string(json_object_array_get_idx(a_perms, i));
			printf("/*%2d*/", (int)i);
			printf("\t%s%s", perm, ",\n"+(++i == n));
		}
		printf("};\n\n");
	}
}

/*
 * search in the global object 'd_perm' the computed representation
 * of the permission described either by 'obj' or 'desc'
 */
struct json_object *new_perm(struct json_object *obj, const char *desc)
{
	const char *tag;
	char *b;
	struct json_object *x, *y;

	tag = obj ? json_object_to_json_string_ext(obj, 0) : desc;
	if (!json_object_object_get_ex(d_perms, tag, &y)) {

		/* creates the d_perms dico and the a_perms array */
		if (!d_perms) {
			d_perms = json_object_new_object();
			a_perms = json_object_new_array();
		}

		/* creates the reference in the structure */
		asprintf(&b, "&_afb_auths_%s[%d]", capi, (int)json_object_array_length(a_perms));
		OOM(b);
		x = json_object_new_string(desc);
		y = json_object_new_string(b);
		json_object_array_add(a_perms, x);
		json_object_object_add(d_perms, tag, y);
		free(b);
	}
	return y;
}

struct json_object *decl_perm(struct json_object *obj);

enum optype { And, Or };

/* recursive declare and/or permissions */
struct json_object *decl_perm_a(enum optype op, struct json_object *obj)
{
	size_t i, n;
	char *a;
	const char *opstr, *fmtstr;
	struct json_object *x, *y;

	if (cpp) {
		fmtstr = "afb::auth_%s(%s, %s)";
		opstr = op==And ? "and" : "or";
	} else {
		fmtstr = "{ .type = afb_auth_%s .first = %s, .next = %s }";
		opstr = op==And ? "And," : "Or, ";
	}
	x = NULL;
	i = n = obj ? json_object_array_length(obj) : 0;
	while (i) {
		y = decl_perm(json_object_array_get_idx(obj, --i));
		if (!y)
			;
		else if (!x)
			x = y;
		else if (x != y) {
			asprintf(&a, fmtstr, opstr, json_object_get_string(y), json_object_get_string(x));
			OOM(a);
			x = new_perm(NULL, a);
			free(a);
		}
	}
	return x;
}

/* declare the permission for obj */
struct json_object *decl_perm(struct json_object *obj)
{
	char *a;
	const char *fmt, *perm;
	struct json_object *x, *y;

	if (json_object_object_get_ex(d_perms, json_object_to_json_string_ext(obj, 0), &x))
		return x;

	if (json_object_object_get_ex(obj, "permission", &x)) {
		perm = json_object_get_string(x);
		if (0 == strcmp(perm, "no")) {
			y = new_perm(obj, cpp ? "afb::auth_no()"
				: "{ .type = afb_auth_No,  .text = NULL, .next = NULL }");
		}
		else if (0 == strcmp(perm, "yes")) {
			y = new_perm(obj, cpp ? "afb::auth_yes()"
				: "{ .type = afb_auth_Yes, .text = NULL, .next = NULL }");
		}
		else {
			if (cpp)
				fmt = "afb::auth_permission(\"%s\")";
			else
				fmt = "{ .type = afb_auth_Permission, .text = \"%s\", .next = NULL }";
			asprintf(&a, fmt, perm);
			OOM(a);
			y = new_perm(obj, a);
			free(a);
		}
	}
	else if (json_object_object_get_ex(obj, "anyOf", &x)) {
		y = decl_perm_a(Or, x);
	}
	else if (json_object_object_get_ex(obj, "allOf", &x)) {
		y = decl_perm_a(And, x);
	}
	else if (json_object_object_get_ex(obj, "not", &x)) {
		x = decl_perm(x);
		if (cpp)
			fmt = "afb::auth_not(%s)";
		else
			fmt = "{ .type = afb_auth_Not, .first = %s, .next = NULL }";
		asprintf(&a, fmt, json_object_get_string(x));
		OOM(a);
		y = new_perm(obj, a);
		free(a);
	}
	else if (json_object_object_get_ex(obj, "LOA", &x))
		y = NULL;
	else if (json_object_object_get_ex(obj, "session", &x))
		y = NULL;
	else
		y = NULL;

	return y;
}

void declare_permissions(const char *name, struct json_object *obj)
{
	struct json_object *p;

	p = permissions_of_verb(obj);
	if (p)
		decl_perm(p);
}


#define SESSION_CLOSE  0x000001
#define SESSION_RENEW  0x000010
#define SESSION_CHECK  0x000100
#define SESSION_LOA_1  0x001000
#define SESSION_LOA_2  0x011000
#define SESSION_LOA_3  0x111000
#define SESSION_MASK   0x111111


int get_session(struct json_object *obj);

int get_session_a(int and, struct json_object *obj)
{
	size_t i, n;
	int x, y;

	n = obj ? json_object_array_length(obj) : 0;
	if (n == 0)
		return 0;

	i = n;
	x = get_session(json_object_array_get_idx(obj, --i));
	while (i) {
		y = get_session(json_object_array_get_idx(obj, --i));
		if (and)
			x &= y;
		else
			x |= y;
	}
	return x;
}

int get_session(struct json_object *obj)
{
	int y;
	const char *a;
	struct json_object *x;

	y = 0;
	if (json_object_object_get_ex(obj, "anyOf", &x)) {
		y = get_session_a(1, x);
	}
	else if (json_object_object_get_ex(obj, "allOf", &x)) {
		y = get_session_a(0, x);
	}
	else if (json_object_object_get_ex(obj, "not", &x)) {
		y = get_session(x);
		y = 0;
	}
	else if (json_object_object_get_ex(obj, "LOA", &x)) {
		switch (json_object_get_int(x)) {
		case 3: y = SESSION_LOA_3; break;
		case 2: y = SESSION_LOA_2; break;
		case 1: y = SESSION_LOA_1; break;
		default: break;
		}
	}
	else if (json_object_object_get_ex(obj, "session", &x)) {
		a = json_object_get_string(x);
		if (!strcmp(a, "check"))
			y = SESSION_CHECK;
		else if (!strcmp(a, "close"))
			y = SESSION_CLOSE;
	}
	else if (json_object_object_get_ex(obj, "token", &x)) {
		a = json_object_get_string(x);
		if (!strcmp(a, "refresh"))
			y = SESSION_RENEW;
	}

	return y;
}

void print_session(struct json_object *p)
{
	int s, c, l;

	s = p ? get_session(p) : 0;
	c = 1;
	if (s & SESSION_CHECK) {
		printf("%s", "|AFB_SESSION_CHECK" + c);
		c = 0;
	}
	if (s & SESSION_LOA_3 & ~SESSION_LOA_2)
		l = 3;
	else if (s & SESSION_LOA_2 & ~SESSION_LOA_1)
		l = 2;
	else if (s & SESSION_LOA_1)
		l = 1;
	else
		l = 0;
	if (l) {
		printf("%s%d", "|AFB_SESSION_LOA_" + c, l);
		c = 0;
	}
	if (s & SESSION_CLOSE) {
		printf("%s", "|AFB_SESSION_CLOSE" + c);
		c = 0;
	}
	if (s & SESSION_RENEW) {
		printf("%s", "|AFB_SESSION_REFRESH" + c);
		c = 0;
	}
	if (c)
		printf("AFB_SESSION_NONE");
}

void print_verb_cbname(const char *name)
{
	printf("%s%s%s" , prefix, cify(name), postfix);
}

void print_declare_verb(const char *name, struct json_object *obj)
{
	if (TEST(scope))
		printf("%s ", scope);
	printf("void ");
	print_verb_cbname(name);

	if (cpp <= 1)
		printf("(afb_req_t request, unsigned argc, afb_data_t const argv[]);\n");
	else
		printf("(afb::req request, afb::received_data params)%s;\n", noexcpt);
}

void print_struct_verb(const char *name, struct json_object *obj)
{
	struct json_object *p;

	p = permissions_of_verb(obj);
	if (cpp >= 2) {
		printf("    afb::verb<");
		print_verb_cbname(name);
		printf(">(\n        \"%s\"", name);
		printf(",\n        nullptr");
		if (p) {
			struct json_object *dp = decl_perm(p);
			printf(",\n        ");
			print_session(p);
			if (dp)
				printf(",\n        %s", json_object_get_string(dp));
		}
		printf("),\n");
	}
	else {
		printf(
			"    {\n"
			"        .verb = \"%s\",\n"
			"        .callback = "
			, name
		);
		print_verb_cbname(name);
		printf(
			",\n"
			"        .auth = %s,\n"
			, p && decl_perm(p) ? json_object_get_string(decl_perm(p)) : "NULL"
		);
		printf(
			"        .info    = NULL,\n"
			"        .vcbdata = NULL,\n"
			"        .session = "
		);
		print_session(p);
		printf(
			",\n"
			"        .glob = 0\n"
			"    },\n"
		);
	}
}

/******************************************************************************/

void getvarint(struct json_object *obj, int *var, const char *pointer, int defval, int set)
{
	struct json_object *o;

	if (set) {
		rp_jsonc_path_search(obj, pointer, &o);
		if (o && json_object_is_type(o, json_type_int))
			*var = json_object_get_int(o);
		else
			*var = defval;
	}
}

void getvarbool(struct json_object *obj, int *var, const char *pointer, int defval)
{
	struct json_object *o;

	rp_jsonc_path_search(obj, pointer, &o);
	if (o && json_object_is_type(o, json_type_boolean))
		*var = json_object_get_boolean(o);
	else
		*var = !!defval;
}

void getvar(struct json_object *obj, const char **var, const char *pointer, const char *defval)
{
	struct json_object *o;

	rp_jsonc_path_search(obj, pointer, &o);
	if (o && json_object_is_type(o, json_type_string))
		*var = json_object_get_string(o);
	else if (*var == NULL)
		*var = defval;
}

void optvarint(int *var, const char *pointer, int defval, int set)
{
	getvarint(options, var, pointer, defval, set);
}

void optvarbool(int *var, const char *pointer, int defval)
{
	getvarbool(options, var, pointer, defval);
}

void optvar(const char **var, const char *pointer, const char *defval)
{
	getvar(options, var, pointer, defval);
}

/******************************************************************************/

void afbinfo_get_options()
{
	int nexpt;

	optvar(&mainctl, "/mainctl", NULL);
	optvar(&scope, "/scope", "static");
	optvar(&prefix, "/prefix", "afb_verb_");
	optvar(&postfix, "/postfix", "_cb");
	optvar(&provideclass, "/provide-class", NULL);
	optvar(&requireclass, "/require-class", NULL);
	optvar(&requireapi, "/require-api", NULL);
	optvarbool(&priv, "/private", 0);
	optvarbool(&noconc, "/noconcurrency", 0);
	optvarbool(&nexpt, "/noexcept", 0);
	noexcpt = nexpt ? " noexcept" : "";
	optvarint(&version, "/version", 4, version_set);
	optvarint(&cpp, "/c++", 0, cpp_set);
	optvarbool(&declare, "/declare", 1);
	getvar(root, &api, "/name", "?");
}

void afbinfo_enum_verbs(void (*func)(const char *name, struct json_object *obj))
{
	unsigned idx;
	struct json_object *verbs, *obj, *onam;
	const char *name;

	/* search the verbs */
	rp_jsonc_path_search(root, "/verbs", &verbs);
	if (!json_object_is_type(verbs, json_type_array))
		return;

	/* list the verbs */
	for (idx = 0 ; idx < json_object_array_length(verbs) ; idx++) {
		obj = json_object_array_get_idx(verbs, idx);
		if (json_object_is_type(obj, json_type_object)
		 && json_object_object_get_ex(obj, "name", &onam)) {
			name = json_object_get_string(onam);
			func(name, obj);
		}
	}
}

/******************************************************************************/

void extract_options()
{
#define GENSKEL "x-afb-genskel"
	options = json_object_object_get(root, GENSKEL);
	json_object_get(options);
	json_object_object_del(root, GENSKEL);
}

/******************************************************************************/

void detectidl()
{
	struct json_object *o;

	rp_jsonc_path_search(root, "/$schema", &o);
	if (o) {
		const char *schema = json_object_get_string(o);
		if (0 == strcmp(schema, "afb-api-info/v2")) {
			idl = idl_info2;
			return;
		}
	}
}

/**
 * process a file and prints its expansion on stdout
 */
void process(char *filename)
{
	char *desc;

	void (*get_options)();
	void (*enum_verbs)(void (*)(const char*, struct json_object*));

	/* translate - */
	if (!strcmp(filename, "-"))
		filename = "/dev/stdin";

	/* check access */
	if (access(filename, R_OK)) {
		fprintf(stderr, "can't access file %s\n", filename);
		exit(1);
	}

	/* read the file */
	root = json_object_from_file(filename);
	if (!root) {
		fprintf(stderr, "reading file %s produced null\n", filename);
		exit(1);
	}

	/* detect the idl */
	detectidl();
	switch(idl) {
	default:
		idl = idl_info2;
		/*@fallthrough@*/
	case idl_info2:
		get_options = afbinfo_get_options;
		enum_verbs = afbinfo_enum_verbs;
		break;
	}

	/* get some names */
	extract_options();
	get_options();

	/* create the description (before expanding $ref ) */
	desc = json2c_std(root);

	/* expand references */
	rp_jsonc_default_expand(&root, NULL);

	/* get the API name */
	capi = cify(api);
	printf(
		"/* generated by %s %s */\n"
		"\n"
		, PROG
		, VERSION
	);
	printf(
		"#ifndef AFB_BINDING_VERSION\n"
		"#define AFB_BINDING_VERSION %d\n"
		"#endif\n"
		"#include <afb/afb-binding%s>\n"
		"\n"
		, version
		, cpp ? "" : ".h"
	);
	printf(
		"static const char _afb_description_%s[] =\n"
		"%s"
		";\n"
		"\n"
		, capi
		, desc
	);
	enum_verbs(declare_permissions);
	print_perms();
	if (declare) {
		enum_verbs(print_declare_verb);
		printf("\n");
	}
	printf(
		"static const struct afb_verb_v%d _afb_verbs_%s[] = {\n"
                , version
		, capi
	);
	enum_verbs(print_struct_verb);
	if (cpp >= 2) {
		printf(
			"    afb::verbend()\n"
			"};\n"
		);
	}
	else {
		printf(
			"    {\n"
			"        .verb = NULL,\n"
			"        .callback = NULL,\n"
			"        .auth = NULL,\n"
			"        .info = NULL,\n"
			"        .vcbdata = NULL,\n"
			"        .session = 0,\n"
			"        .glob = 0\n"
			"    }\n"
			"};\n"
		);
	}

	if (declare && TEST(mainctl)) {
		printf("\n");
		if (TEST(mainctl)) {
			if (TEST(scope))
				printf("%s ", scope);
			if (cpp < 2)
				printf("int %s(afb_api_t api, afb_ctlid_t ctlid, const afb_ctlarg_t ctlarg, void *userdata);\n",
					mainctl);
			else
				printf("int %s(afb::api api, afb::ctlid ctlid, const afb::ctlarg ctlarg, void *userdata)%s;\n",
					mainctl, noexcpt);
		}
	}
	printf("\n");
	if (cpp >= 2) {
		printf(
			"%sconst afb_binding_t %s%s = afb::binding<%s>(\n"
			"    \"%s\",\n"
			"    _afb_verbs_%s,\n"
			"    nullptr,\n"
			"    _afb_description_%s%s);\n"
			/* var */
			, priv ? "static " : ""
			, priv ? "_afb_binding_" : "afbBindingV4"
			, priv ? capi : ""
			/* mainctl */
			, TEST(mainctl) ? mainctl : "nullptr"
			, api /* api */
			, capi /* verbs */
			, capi /* description */
			, noconc ? ",\n    true" : ""
		);
	}
	else {
		printf(
			"%sconst struct afb_binding_v%d %s%s = {\n"
			"    .api = \"%s\",\n"
			"    .specification = _afb_description_%s,\n"
			"    .info = NULL,\n"
			"    .verbs = _afb_verbs_%s,\n"
			, priv ? "static " : ""
			, version
			, priv ? "_afb_binding_" : "afbBindingV4"
			, priv ? capi : ""
			, api
			, capi
			, capi
		);

		printf(
			"    .mainctl = %s,\n"
			, TEST(mainctl) ? mainctl : "NULL"
		);

		printf(
			"    .userdata = NULL,\n"
			"    .provide_class = %s%s%s,\n"
			"    .require_class = %s%s%s,\n"
			"    .require_api = %s%s%s,\n"
			, TEST(provideclass) ? "\"" : "", TEST(provideclass) ? provideclass : "NULL", TEST(provideclass) ? "\"" : ""
			, TEST(requireclass) ? "\"" : "", TEST(requireclass) ? requireclass : "NULL", TEST(requireclass) ? "\"" : ""
			, TEST(requireapi) ? "\"" : "", TEST(requireapi) ? requireapi : "NULL", TEST(requireapi) ? "\"" : ""
		);

		printf(
			"    .noconcurrency = %d\n"
			"};\n"
			, !!noconc
		);
	}
	printf("\n");

	/* clean up */
	json_object_put(root);
	free(desc);
}

/** process the list of files or stdin if none */
int main(int ac, char **av)
{
	int r, w;
	av++;

	r = w = 0;
	while (av[r]) {
		if (!strcmp(av[r], "-v") || !strcmp(av[r], "--version")) {
			printf("%s version %s (c) IoT.bzh\n", PROG, VERSION);
			exit(EXIT_SUCCESS);
		}
		else if (!strcmp(av[r], "-h") || !strcmp(av[r], "--help")) {
			printf(
			"\n"
			"Produce binding skeleton from info description\n"
			"\n"
			"usage:\n"
			"  " PROG " -h|--help\n"
			"  " PROG " -v|--version\n"
			"  " PROG " [-e|--extern] [-x|--cpp] [FILE ...]\n"
			"\n"
			"Options:\n"
			" -e, --extern   Make functions extern\n"
			" -x, --c++      Produce C++ (repeat it for more C++)\n"
			"\n"
			);
			exit(EXIT_SUCCESS);
		}
		else if (!strcmp(av[r], "-x") || !strcmp(av[r], "--c++")) {
			cpp_set = 0;
			cpp++;
		}
		else if (!strcmp(av[r], "-e") || !strcmp(av[r], "--extern")) {
			scope = "extern";
		}
		else if (!strcmp(av[r], "-4")) {
			version_set = 0;
			version = 4;
		}
		else {
			av[w++] = av[r];
		}
		r++;
	}
	av[w] = NULL;
	if (!*av)
		process("-");
	else {
		do { process(*av++); } while(*av);
	}
	return EXIT_SUCCESS;
}

