/*
 Copyright (C) 2015-2021 IoT.bzh Company

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

#pragma once

#include <stdarg.h>

/*
  verbosity tune the count of reported messages

   verbosity value : reported messages
   ----------------+------------------------
    lesser than 0  : no message at all
         0         : ERROR
         1         : ERROR, WARNING
         2         : ERROR, WARNING, NOTICE
         3         : ERROR, WARNING, NOTICE, INFO
    greater than 3 : ERROR, WARNING, NOTICE, INFO, DEBUG

extern int verbosity;

enum verbosity_levels
{
	Verbosity_Level_Error = 0,
	Verbosity_Level_Warning = 1,
	Verbosity_Level_Notice = 2,
	Verbosity_Level_Info = 3,
	Verbosity_Level_Debug = 4
};
*/

extern void verbose_set_name(const char *name, int authority);

/*
 Log level is defined by syslog standard:
       KERN_EMERG             0        System is unusable
       KERN_ALERT             1        Action must be taken immediately
       KERN_CRIT              2        Critical conditions
       KERN_ERR               3        Error conditions
       KERN_WARNING           4        Warning conditions
       KERN_NOTICE            5        Normal but significant condition
       KERN_INFO              6        Informational
       KERN_DEBUG             7        Debug-level messages
*/

enum
{
	Log_Level_Emergency = 0,
	Log_Level_Alert = 1,
	Log_Level_Critical = 2,
	Log_Level_Error = 3,
	Log_Level_Warning = 4,
	Log_Level_Notice = 5,
	Log_Level_Info = 6,
	Log_Level_Debug = 7
};

extern int logmask;

extern void set_logmask(int lvl);

extern void verbose(int loglevel, const char *file, int line, const char *function, const char *fmt, ...) __attribute__((format(printf, 5, 6)));
extern void vverbose(int loglevel, const char *file, int line, const char *function, const char *fmt, va_list args);

#if defined(VERBOSE_NO_DATA)
# define __VERBOSE__(lvl,...)      do{if((lvl)<=Log_Level_Error) verbose(lvl, __FILE__, __LINE__, NULL, __VA_ARGS__)\
					else verbose(lvl, __FILE__, __LINE__, NULL, NULL);}while(0)
#elif defined(VERBOSE_NO_DETAILS)
# define __VERBOSE__(lvl,...)      verbose(lvl, NULL, 0, NULL, __VA_ARGS__)
#else
# define __VERBOSE__(lvl,...)      verbose(lvl, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#define _LOGMASK_(lvl)		((lvl) < 0 ? -1 : (1 << (lvl)))
#define _WANTLOG_(lvl)		(logmask & _LOGMASK_(lvl))
#define _VERBOSE_(lvl,...)	do{ if (_WANTLOG_(lvl)) __VERBOSE__((lvl), __VA_ARGS__); } while(0)

#define EMERGENCY(...)            _VERBOSE_(Log_Level_Emergency, __VA_ARGS__)
#define ALERT(...)                _VERBOSE_(Log_Level_Alert, __VA_ARGS__)
#define CRITICAL(...)             _VERBOSE_(Log_Level_Critical, __VA_ARGS__)
#define ERROR(...)                _VERBOSE_(Log_Level_Error, __VA_ARGS__)
#define WARNING(...)              _VERBOSE_(Log_Level_Warning, __VA_ARGS__)
#define NOTICE(...)               _VERBOSE_(Log_Level_Notice, __VA_ARGS__)
#define INFO(...)                 _VERBOSE_(Log_Level_Info, __VA_ARGS__)
#define DEBUG(...)                _VERBOSE_(Log_Level_Debug, __VA_ARGS__)

#define LOGUSER(app)              verbose_set_name(app,0)
#define LOGAUTH(app)              verbose_set_name(app,1)

extern void (*verbose_observer)(int loglevel, const char *file, int line, const char *function, const char *fmt, va_list args);

static inline int verbose_wants(int lvl) { return _WANTLOG_(lvl); }

extern void verbose_dec();
extern void verbose_inc();
extern void verbose_clear();
extern void verbose_add(int level);
extern void verbose_sub(int level);
extern void verbose_colorize(int value);
extern int verbose_is_colorized();

extern int verbose_level_of_name(const char *name);
extern const char *verbose_name_of_level(int level);

#define _DEVERBOSITY_(vlvl)	((vlvl) + Log_Level_Error)
#define _VERBOSITY_(llvl)	((llvl) - Log_Level_Error)
extern int verbosity_get();
extern void verbosity_set(int verbo);
extern int verbosity_from_mask(int mask);
extern int verbosity_to_mask(int verbo);

#define COLOR_EMERGENCY	"\x1B[101m"
#define COLOR_ALERT	"\x1B[43m"
#define COLOR_CRITICAL	"\x1B[41m"
#define COLOR_ERROR	"\x1B[91m"
#define COLOR_WARNING	"\x1B[93m"
#define COLOR_NOTICE	"\x1B[96m"
#define COLOR_INFO	"\x1B[94m"
#define COLOR_DEBUG	"\x1B[95m"
#define COLOR_API	"\x1B[1m"
#define COLOR_FILE	"\x1B[90m"
#define COLOR_DEFAULT	"\x1B[0m"
