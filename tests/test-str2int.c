
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "../src/misc/rp-str2int.h"

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
	uint64_t u, ou;
	int rc;
	unsigned b;
	for(b = 1 ; b <= 65 ; b++) {
		bin_u_max(b, buffer, sizeof buffer);
		rc = rp_str2u64(buffer, &u);
		printf("%u: %d %llu %s\n", b, rc, u, buffer);
		if (rc > 0) {
			sprintf(buffer, "0o%llo", u);
			rc = rp_str2u64(buffer, &ou);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "0O%llo", u);
			rc = rp_str2u64(buffer, &ou);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "0%llo", u);
			rc = rp_str2u64(buffer, &ou);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "0x%llx", u);
			rc = rp_str2u64(buffer, &ou);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "0X%llX", u);
			rc = rp_str2u64(buffer, &ou);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "%llu", u);
			rc = rp_str2u64(buffer, &ou);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "0d%llu", u);
			rc = rp_str2u64(buffer, &ou);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);

			sprintf(buffer, "0D%llu", u);
			rc = rp_str2u64(buffer, &ou);
			printf("%u: %d %llu %s\n", b, rc, ou, buffer);
		}
	}
}

