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

#include <stdint.h>

typedef int (*d2c_get_cb)(void*, uint8_t*);
typedef int (*d2c_put_cb)(void*, uint8_t);

int d2c(d2c_get_cb get, void *getdata, d2c_put_cb put, void *putdata, const char *prefix, int width)
{
	size_t len;
	int pos;
	uint8_t in;

#define P(x)    do { int s = put(putdata, (uint8_t)(x)); if(s < 0) return s; len++; pos++; } while(0)
#define PS(x)   do { const char *p=(x); while(*p) { P(*p); p++; } } while(0)
#define BOL()   do { if(prefix != NULL && pos == 0) { PS(prefix); P('"'); } } while(0)
#define EOL()   do { if(prefix != NULL && pos > 0) { P('"'); P('\n'); pos = 0; } } while(0)

	/* translate the string */
	len = 0;
	pos = 0;
	BOL();
	for(;;) {
		/* get next byte */
		int s = get(getdata, &in);

		/* error? */
		if (s < 0)
			return s; /* yes, return */

		/* end ? */
		if (s == 0) {
			/* yes, end */
			EOL();
			return 0;
		}

		/* set buf with next char */
		BOL();
		switch((char)in) {
		case '\\':
			P('\\');
			P('\\');
			break;
		case '"':
			P('\\');
			P('"');
			break;
		case '\n':
			P('\\');
			P('n');
			break;
		case '\t':
			P('\\');
			P('t');
			break;
		default:
			if (32 <= in && in < 127)
				P(in);
			else {
				P('\\');
				P((uint8_t)('0' + (3 & (in >> 6))));
				P((uint8_t)('0' + (7 & (in >> 3))));
				P((uint8_t)('0' + (7 & (in >> 3))));
			}
			break;
		}
		/* add the char in the output */
		if (pos >= width - 1)
			EOL();
	}
#undef P
#undef PS
#undef BOL
#undef EOL
}

