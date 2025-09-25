
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <json-c/json.h>

#include "../src/json/rp-jsonc.h"

char buffer[100];

void bin_u_max(unsigned bits, char *buffer, size_t len)
{
	if (3 + bits > len)
		exit(1);
	*buffer++ = '0';
	*buffer++ = 'b';
	while(bits--)
		*buffer++ = '1';
	*buffer = 0;
}


int main()
{
	json_object *obj;
	uint64_t u, ou;
	int rc;
	unsigned b;
	for(b = 1 ; b <= 65 ; b++) {
		bin_u_max(b, buffer, sizeof buffer);
		obj = json_object_new_string(buffer);
		rc = rp_jsonc_get_uint64(obj, &u, rp_jsonc_int_mode_number);
		json_object_put(obj);
		printf("%u: %d %llu %s\n", b, rc, u, buffer);
		if (rc > 0) {
			sprintf(buffer, "0o%llo", u);
			obj = json_object_new_string(buffer);
			rc = rp_jsonc_get_uint64(obj, &ou, rp_jsonc_int_mode_number);
			json_object_put(obj);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "0O%llo", u);
			obj = json_object_new_string(buffer);
			rc = rp_jsonc_get_uint64(obj, &ou, rp_jsonc_int_mode_number);
			json_object_put(obj);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "0%llo", u);
			obj = json_object_new_string(buffer);
			rc = rp_jsonc_get_uint64(obj, &ou, rp_jsonc_int_mode_number);
			json_object_put(obj);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "0x%llx", u);
			obj = json_object_new_string(buffer);
			rc = rp_jsonc_get_uint64(obj, &ou, rp_jsonc_int_mode_number);
			json_object_put(obj);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "0X%llX", u);
			obj = json_object_new_string(buffer);
			rc = rp_jsonc_get_uint64(obj, &ou, rp_jsonc_int_mode_number);
			json_object_put(obj);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "%llu", u);
			obj = json_object_new_string(buffer);
			rc = rp_jsonc_get_uint64(obj, &ou, rp_jsonc_int_mode_number);
			json_object_put(obj);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "0d%llu", u);
			obj = json_object_new_string(buffer);
			rc = rp_jsonc_get_uint64(obj, &ou, rp_jsonc_int_mode_number);
			json_object_put(obj);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "0D%llu", u);
			obj = json_object_new_string(buffer);
			rc = rp_jsonc_get_uint64(obj, &ou, rp_jsonc_int_mode_number);
			json_object_put(obj);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);
		}
	}
}

