/*
 Copyright (C) 2015-2023 IoT.bzh Company

 Author: José Bollo <jose.bollo@iot.bzh>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is furnished
 to do so, subject to the following conditions:

 The above copyright notice and this permission notice (including the next
 paragraph) shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#include <stdio.h>
#include <stdarg.h>

#include "rp-verbose.h"

#ifndef RP_VERBOSE_CONTEXT_DEPTH
#define RP_VERBOSE_CONTEXT_DEPTH 8
#endif

static const char *contexts[RP_VERBOSE_CONTEXT_DEPTH];
static unsigned short contexts_depth;

/**********************************************************************************
* Log with SYSLOG or SYSTEMD
**********************************************************************************/
#if defined(VERBOSE_WITH_SYSLOG) || defined(VERBOSE_WITH_SYSTEMD)

#if !defined(MAXIMAL_LOGLEVEL)
# define MAXIMAL_LOGLEVEL	rp_Log_Level_Debug
#endif

void rp_verbose_colorize(int value)
{
}

int rp_verbose_is_colorized()
{
	return 0;
}

#endif

/**********************************************************************************
* Log with SYSLOG
**********************************************************************************/
#if defined(VERBOSE_WITH_SYSLOG)

#include <syslog.h>

static void _vverbose_(int loglevel, const char *file, int line, const char *function, const char *fmt, va_list args, int saverr)
{
	char *p;

	if (file == NULL || vasprintf(&p, fmt, args) < 0)
		vsyslog(loglevel, fmt, args);
	else {
		syslog(loglevel, "%s [%s:%d, %s]", p, file, line, function?:"?");
		free(p);
	}
}

/**********************************************************************************
* Log with SYSTEMD
**********************************************************************************/
#elif defined(VERBOSE_WITH_SYSTEMD)

#define SD_JOURNAL_SUPPRESS_LOCATION

#include <systemd/sd-journal.h>

static void _vverbose_(int loglevel, const char *file, int line, const char *function, const char *fmt, va_list args, int saverr)
{
	char lino[20];

	if (file == NULL) {
		sd_journal_printv(loglevel, fmt, args);
	} else {
		sprintf(lino, "%d", line);
		sd_journal_printv_with_location(loglevel, file, lino, function, fmt, args);
	}
}

/**********************************************************************************
* Log with STDERR
**********************************************************************************/
#else

#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "x-uio.h"
#include "x-mutex.h"

static const char *prefixes[] = {
	"<0> EMERGENCY",
	"<1> ALERT",
	"<2> CRITICAL",
	"<3> ERROR",
	"<4> WARNING",
	"<5> NOTICE",
	"<6> INFO",
	"<7> DEBUG",
	"<7> DEBUG"
};

static const char *colored_prefixes[] = {
	"<0> " RP_VERBOSE_COLOR_EMERGENCY	"EMERGENCY"	RP_VERBOSE_COLOR_DEFAULT,
	"<1> " RP_VERBOSE_COLOR_ALERT		"    ALERT"	RP_VERBOSE_COLOR_DEFAULT,
	"<2> " RP_VERBOSE_COLOR_CRITICAL	" CRITICAL"	RP_VERBOSE_COLOR_DEFAULT,
	"<3> " RP_VERBOSE_COLOR_ERROR		"    ERROR"	RP_VERBOSE_COLOR_DEFAULT,
	"<4> " RP_VERBOSE_COLOR_WARNING		"  WARNING"	RP_VERBOSE_COLOR_DEFAULT,
	"<5> " RP_VERBOSE_COLOR_NOTICE		"   NOTICE"	RP_VERBOSE_COLOR_DEFAULT,
	"<6> " RP_VERBOSE_COLOR_INFO		"     INFO"	RP_VERBOSE_COLOR_DEFAULT,
	"<7> " RP_VERBOSE_COLOR_DEBUG		"    DEBUG"	RP_VERBOSE_COLOR_DEFAULT,
	"<7> " RP_VERBOSE_COLOR_DEBUG		"    DEBUG"	RP_VERBOSE_COLOR_DEFAULT
};

static const char chars[] =  { '\n', '?', ':', ' ', '[', ']', ',', ' ' };
#define CHAR_EOL		(&chars[0])
#define CHAR_QUESTION		(&chars[1])
#define CHAR_COLON		(&chars[2])
#define CHAR_SPACE		(&chars[3])
#define CHAR_OBRACE		(&chars[4])
#define CHAR_CBRACE		(&chars[5])
#define CHAR_COMMA		(&chars[6])
#define CHARS_COLON_SPACE	CHAR_COLON
#define CHARS_SPACE_OBRACE	CHAR_SPACE
#define CHARS_COMMA_SPACE	CHAR_COMMA

static x_mutex_t mutex = X_MUTEX_INITIALIZER;

static int is_tty()
{
	static int tty;

	if (!tty)
		tty = 1 + isatty(STDERR_FILENO);
	return tty - 1;
}

int rp_verbose_colorize(int value)
{
	static int colorized;

	if (value >= 0)
		colorized = 1 + (is_tty() && value);
	else if (!colorized)
		colorized = 1 + is_tty();
	return colorized - 1;
}

int rp_verbose_is_colorized()
{
	return rp_verbose_colorize(-1);
}

static void _vverbose_(int loglevel, const char *file, int line, const char *function, const char *fmt, va_list args, int saverr)
{
	char buffer[4000];
	char lino[40]; /* line number with more than 39 digits are difficult to find */
	int n, rc, addcolor;
	unsigned idx;
	struct iovec iov[20 + RP_VERBOSE_CONTEXT_DEPTH + RP_VERBOSE_CONTEXT_DEPTH];
	int tty;

	/* check if tty (2) or not (1) */
	tty = is_tty();

	/* prefix */
	addcolor = rp_verbose_is_colorized();
	if (addcolor)
		iov[0].iov_base = (void*)(colored_prefixes[loglevel] + (tty ? 4 : 0));
	else
		iov[0].iov_base = (void*)(prefixes[loglevel] + (tty ? 4 : 0));
	iov[0].iov_len = strlen(iov[0].iov_base);

	/* " " */
	iov[1].iov_base = (void*)CHARS_COLON_SPACE;
	iov[1].iov_len = 2;

	n = 2;
	for (idx = 0 ; idx < contexts_depth && idx < RP_VERBOSE_CONTEXT_DEPTH ; idx++) {
		iov[n].iov_base = (void*)contexts[idx];
		iov[n++].iov_len = strlen(contexts[idx]);
		iov[n].iov_base = (void*)CHARS_COMMA_SPACE;
		iov[n++].iov_len = 0;
	}
	if (fmt) {
		iov[n].iov_base = buffer;
		errno = saverr;
		rc = vsnprintf(buffer, sizeof buffer, fmt, args);
		if (rc < 0)
			rc = 0;
		else if ((size_t)rc > sizeof buffer) {
			/* if too long, ellipsis the end with ... */
			rc = (int)sizeof buffer;
			buffer[rc - 1] = buffer[rc - 2]  = buffer[rc - 3] = '.';
		}
		iov[n++].iov_len = (size_t)rc;
	}
	if (file && (!fmt || !tty || loglevel <= rp_Log_Level_Warning)) {

		if (addcolor)
		{
			iov[n].iov_base = (void*)RP_VERBOSE_COLOR_FILE;
			iov[n++].iov_len = strlen(RP_VERBOSE_COLOR_FILE);
		}

		/* "[" (!fmt) or " [" (fmt) */
		iov[n].iov_base = (void*)(CHARS_SPACE_OBRACE + !fmt);
		iov[n++].iov_len = 2 - !fmt;
		/* file */
		iov[n].iov_base = (void*)file;
		iov[n++].iov_len = strlen(file);
		/* ":" */
		iov[n].iov_base = (void*)CHAR_COLON;
		iov[n++].iov_len = 1;
		if (line) {
			/* line number */
			iov[n].iov_base = lino;
			iov[n++].iov_len = (size_t)snprintf(lino, sizeof lino, "%d", line);
		} else {
			/* "?" */
			iov[n].iov_base = (void*)CHAR_QUESTION;
			iov[n++].iov_len = 1;
		}
		/* "," */
		iov[n].iov_base = (void*)CHAR_COMMA;
		iov[n++].iov_len = 1;
		if (function) {
			/* function name */
			iov[n].iov_base = (void*)function;
			iov[n++].iov_len = strlen(function);
		} else {
			/* "?" */
			iov[n].iov_base = (void*)CHAR_QUESTION;
			iov[n++].iov_len = 1;
		}
		iov[n].iov_base = (void*)CHAR_CBRACE;
		iov[n++].iov_len = 1;

		if (addcolor)
		{
			iov[n].iov_base = (void*)RP_VERBOSE_COLOR_DEFAULT;
			iov[n++].iov_len = strlen(RP_VERBOSE_COLOR_DEFAULT);
		}
	}
	if (n == 2) {
		/* "?" */
		iov[n].iov_base = (void*)CHAR_QUESTION;
		iov[n++].iov_len = 1;
	}
	/* "\n" */
	iov[n].iov_base = (void*)CHAR_EOL;
	iov[n++].iov_len = 1;

	/* emit the message */
	x_mutex_lock(&mutex);
	writev(STDERR_FILENO, iov, n);
	x_mutex_unlock(&mutex);
}

#endif

/**********************************************************************************
* COMMON PART
**********************************************************************************/

#if !defined(DEFAULT_LOGLEVEL)
# define DEFAULT_LOGLEVEL	rp_Log_Level_Warning
#endif

#if !defined(MINIMAL_LOGLEVEL)
# define MINIMAL_LOGLEVEL	rp_Log_Level_Error
#endif

#if !defined(MAXIMAL_LOGLEVEL)
# define MAXIMAL_LOGLEVEL	rp_Log_Level_Extra_Debug
#endif

#if !defined(DEFAULT_LOGMASK)
# define DEFAULT_LOGMASK	((1 << ((DEFAULT_LOGLEVEL) + 1)) - 1)
#endif

#if !defined(MINIMAL_LOGMASK)
# define MINIMAL_LOGMASK	((1 << ((MINIMAL_LOGLEVEL) + 1)) - 1)
#endif

#if !defined(MAXIMAL_LOGMASK)
# define MAXIMAL_LOGMASK	((1 << ((MAXIMAL_LOGLEVEL) + 1)) - 1)
#endif

#define CROP_LOGLEVEL(x) \
	((x) < rp_Log_Level_Emergency ? rp_Log_Level_Emergency \
	                           : (x) > MAXIMAL_LOGLEVEL ? MAXIMAL_LOGLEVEL : (x))


static const char *names[] = {
	"emergency",
	"alert",
	"critical",
	"error",
	"warning",
	"notice",
	"info",
	"debug",
	"extra",
};

int rp_logmask = DEFAULT_LOGMASK | MINIMAL_LOGMASK;

extern rp_verbose_observer_cb rp_verbose_observer;

void (*rp_verbose_observer)(int loglevel, const char *file, int line, const char *function, const char *fmt, va_list args);

void rp_vverbose(int loglevel, const char *file, int line, const char *function, const char *fmt, va_list args)
{
	int saverr = errno;
	void (*observer)(int loglevel, const char *file, int line, const char *function, const char *fmt, va_list args) = rp_verbose_observer;

	loglevel = CROP_LOGLEVEL(loglevel);
	if (!observer)
		_vverbose_(loglevel, file, line, function, fmt, args, saverr);
	else {
		va_list ap;
		va_copy(ap, args);
		_vverbose_(loglevel, file, line, function, fmt, args, saverr);
		errno = saverr;
		observer(loglevel, file, line, function, fmt, ap);
		va_end(ap);
	}

	/* restore errno */
	errno = saverr;
}

void rp_verbose(int loglevel, const char *file, int line, const char *function, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	rp_vverbose(loglevel, file, line, function, fmt, ap);
	va_end(ap);
}

void rp_set_logmask(int logmask)
{
	rp_logmask = (logmask | MINIMAL_LOGMASK) & MAXIMAL_LOGMASK;
}

void rp_verbose_add(int level)
{
	rp_set_logmask(rp_logmask | (1 << level));
}

void rp_verbose_sub(int level)
{
	rp_set_logmask(rp_logmask & (1 << level));
}

void rp_verbose_clear()
{
	rp_set_logmask(0);
}

static int get_mask()
{
	int m = MINIMAL_LOGMASK;
	while (rp_logmask != (rp_logmask & m))
		m = m + m + 1;
	return m;
}

void rp_verbose_dec()
{
	rp_set_logmask(rp_logmask & (get_mask() >> 1));
}

void rp_verbose_inc()
{
	rp_set_logmask(rp_logmask | (get_mask() + 1));
}

int rp_verbose_level_of_name(const char *name)
{
	int c, i = sizeof names / sizeof * names;
	while (i) {
		i--;
#if WITH_CASE_FOLDING
		c = strcasecmp(names[i], name);
#else
		c = strcmp(names[i], name);
#endif
		if (c == 0)
			return i;
	}
	return -1;
}

const char *rp_verbose_name_of_level(int level)
{
	return level == CROP_LOGLEVEL(level) ? names[level] : NULL;
}

void rp_verbose_push(const char *context)
{
	if (contexts_depth < RP_VERBOSE_CONTEXT_DEPTH)
		contexts[contexts_depth] = context;
	contexts_depth++;
}

void rp_verbose_pop()
{
	if (contexts_depth)
		contexts_depth--;
}

