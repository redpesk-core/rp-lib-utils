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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "d2c.c"

int put(void *data, uint8_t byte)
{
	FILE *file = (FILE*)data;
	size_t sz = fwrite(&byte, 1, 1, file);
	return sz == 1 ? 1 : -!feof(file);
}

int get(void *data, uint8_t *byte)
{
	FILE *file = (FILE*)data;
	size_t sz = fread(byte, 1, 1, file);
	return sz == 1 ? 1 : -!feof(file);
}

/**
 * process a file and prints its expansion on stdout
 */
void process(char *filename)
{
	FILE *file;

	/* translate - */
	if (!strcmp(filename, "-"))
		filename = "/dev/stdin";

	/* check access */
	file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "can't read file %s\n", filename);
		exit(1);
	}

	/* create the description */
	fprintf(stdout, "\t/* generated from %s */\n", filename);
	d2c(get, file, put, stdout, "\t", 71);

	/* clean up */
	fclose(file);
}

/** process the list of files or stdin if none */
int main(int ac, char **av)
{
	if (!*++av)
		process("-");
	else {
		do { process(*av); } while(*++av);
	}
	return 0;
}

