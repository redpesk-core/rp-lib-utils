/*
 * Copyright (C) 2015-2021 IoT.bzh Company
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
 */


#if WITH_SYSTEMD

#include <unistd.h>

#include <systemd/sd-event.h>
#include <systemd/sd-bus.h>
#include <systemd/sd-daemon.h>

#include "systemd.h"
#include "x-errno.h"

static int sdbusopen(struct sd_bus **p, int (*f)(struct sd_bus **))
{
	int rc;
	struct sd_bus *r;

	r = *p;
	if (r)
		rc = 0;
	else {
		rc = f(&r);
		if (rc >= 0) {
			rc = sd_bus_attach_event(r, systemd_get_event_loop(), 0);
			if (rc < 0)
				sd_bus_unref(r);
			else
				*p = r;
		}
	}
	return rc;
}

struct sd_event *systemd_get_event_loop()
{
	static struct sd_event *result = 0;
	int rc;

	if (!result) {
		rc = sd_event_new(&result);
		if (rc < 0)
			result = NULL;
	}
	return result;
}

struct sd_bus *systemd_get_user_bus()
{
	static struct sd_bus *result = 0;
	sdbusopen((void*)&result, (void*)sd_bus_open_user);
	return result;
}

struct sd_bus *systemd_get_system_bus()
{
	static struct sd_bus *result = 0;
	sdbusopen((void*)&result, (void*)sd_bus_open_system);
	return result;
}

static int fds_names(char ***result)
{
	static char *null;
	static char **names;

	int rc;

	if (names)
		rc = 0;
	else {
		rc = sd_listen_fds_with_names(1, &names);
		if (rc <= 0)
			names = &null;
	}
	*result = names;
	return rc;
}

int systemd_fds_init()
{
	char **names;
	return fds_names(&names);
}

int systemd_fds_for(const char *name)
{
	int idx;
	char **names;

	fds_names(&names);
	for (idx = 0 ; names[idx] != NULL ; idx++)
		if (!strcmp(name, names[idx]))
			return idx + SD_LISTEN_FDS_START;

	return X_ENOENT;
}

#endif
