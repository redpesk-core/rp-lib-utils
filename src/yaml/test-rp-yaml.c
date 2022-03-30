
#include "rp-yaml.h"
#include "../json/rp-jsonc-locator.h"

static void p(unsigned lino, unsigned depth, const char *text0, const char *text1, const char *text2)
{
	printf("%-5u %.*s%s%s%s\n", lino, depth<<1, "                                                             ", text0, text1, text2);
}

static void dump(struct json_object *jso, unsigned depth, const char *text0, const char *text1)
{
#if JSON_C_VERSION_NUM >= 0x000d00
	size_t idx, len;
#else
	int idx, len;
#endif
	struct json_object_iterator it, end;

	unsigned lino;
	const char *file = rp_jsonc_locator_locate(jso, &lino);

	/* inspect type of the jso */
	switch (json_object_get_type(jso)) {
	case json_type_object:
		it = json_object_iter_begin(jso);
		end = json_object_iter_end(jso);
		while (!json_object_iter_equal(&it, &end)) {
			dump(json_object_iter_peek_value(&it), depth+1, json_object_iter_peek_name(&it), ": ");
			json_object_iter_next(&it);
		}
		break;
	case json_type_array:
		len = json_object_array_length(jso);
		for (idx = 0 ; idx < len ; idx++) {
			dump(json_object_array_get_idx(jso, idx), depth+1, "-", " ");
		}
		break;
	default:
		p(lino, depth, text0, text1, json_object_get_string(jso));
		break;
	}
}

int main(int ac, char **av)
{
	int rc;
	json_object *obj;

	while (*++av) {
		printf("\nREAD %s\n", *av);
		rc = rp_yaml_path_to_json_c(&obj, *av, *av);
		printf(" = %d\n", rc);
		if (rc == 0) {
			printf(" => %s\n", json_object_to_json_string(obj));
			dump(obj, 0, "", "");
			json_object_put(obj);
		}	
	}
	return 0;
}
