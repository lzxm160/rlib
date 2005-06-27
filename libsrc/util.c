/*
 *  Copyright (C) 2003-2005 SICOM Systems, INC.
 *
 *  Authors: Bob Doan <bdoan@sicompos.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include "config.h"
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#include <locale.h>

#include "rlib.h"

#ifndef LC_MESSAGES
#define LC_MESSAGES 5
#endif

#ifndef LC_PAPER
#define LC_PAPER 7
#endif

#ifndef LC_NAME
#define LC_NAME 8
#endif

#ifndef LC_ADDRESS
#define LC_ADDRESS 9
#endif

#ifndef LC_TELEPHONE
#define LC_TELEPHONE 10
#endif

#ifndef LC_MEASUREMENT
#define LC_MEASUREMENT 11
#endif

#ifndef LC_IDENTIFICATION
#define LC_IDENTIFICATION 12
#endif

#ifndef SIGIOT
#define SIGIOT SIGABRT
#endif		

int locale_codes[] = {
	LC_ALL,
	LC_COLLATE,
	LC_CTYPE,
	LC_MESSAGES,
	LC_MONETARY,
	LC_NUMERIC,
	LC_TIME,
	LC_PAPER,
	LC_NAME,
	LC_ADDRESS,
	LC_TELEPHONE,
	LC_MEASUREMENT,
	LC_IDENTIFICATION,
	-1
};

#ifdef HAVE_SYS_RESOURCE_H
#ifdef ENABLE_CRASH
static void myFaultHandler (gint signum, siginfo_t *si, gpointer aptr) {
	struct rlimit rlim;
	rlogit("** NUTS.. WE CRASHED\n");
	getrlimit (RLIMIT_CORE, &rlim); //POSIBLY NOT NECESSARY
	rlim.rlim_cur = 1024000000; //NECESSARY
	setrlimit (RLIMIT_CORE, &rlim); //NECESSARY
	signal (SIGQUIT, SIG_DFL); //KEEP THIS!!!!!
	kill (getpid(), SIGQUIT); //IMPORTANT
	exit (5); //THEORETICALLY IN THEORY THIS WILL NEVER GET CALLED... but lets play it safe
}
#endif
#endif

static gint useMyHandler = TRUE;

void init_signals(void) {
#ifdef HAVE_SYS_RESOURCE_H
#ifdef ENABLE_CRASH
	struct sigaction sa;
	if (useMyHandler) {
		memset(&sa, 0, sizeof(struct sigaction));
		sa.sa_handler = (void(*)(int))myFaultHandler;
		sigaction (SIGILL, &sa, NULL);
		sigaction (SIGBUS, &sa, NULL);
		sigaction (SIGSEGV, &sa, NULL);
		sigaction (SIGABRT, &sa, NULL);
		sigaction (SIGIOT, &sa, NULL);
		sigaction (SIGTRAP, &sa, NULL);
		signal (SIGQUIT, SIG_DFL);
	}
#endif
#endif
}

gint rutil_enableSignalHandler(gint trueorfalse) {
	gint whatitwas = useMyHandler;
	useMyHandler = trueorfalse;
	return whatitwas;
}

gchar *strlwrexceptquoted (char *s) {
	gchar c; 
	gchar *ptr = s;
	gint quote=0;
	while ((c = tolower(*s)) != '\0') {
		if(*s == '\'') {
			if(quote)
				quote=0;
			else
				quote=1;
		}
		if(!quote)
			*s++ = c;
		else
			s++;
	}
	return ptr;
}

gchar *rmwhitespacesexceptquoted(gchar *s) {
	gchar *backptr = s;
	gchar *orig = s;
	gint spacecount=0;
	gint quote=0;
	while(*s != '\0') {
		if(*s == '\'') {
			if(quote)
				quote=0;
			else
				quote=1;
		}

		if(*s != ' ' || quote==1)
			backptr++;
		else 
			spacecount++;		
		s++;
		*backptr = *s;
	}
	backptr++;
	if(spacecount)
		*backptr = '\0';
	return orig;
}

static void local_rlogit(const gchar *message) {
	fputs(message, stderr);
	return;
}

static void (*logMessage)(const gchar *msg) = local_rlogit;


void rlogit_setmessagewriter(void (*msgwriter)(const gchar *msg)) {
	logMessage = msgwriter;
}

void rlogit(const gchar *fmt, ...) {
	va_list vl;
	gchar *result = NULL;

	va_start(vl, fmt);
	result = g_strdup_vprintf(fmt, vl);
	va_end(vl);
	if (result != NULL) {
		logMessage(result);
		g_free(result);
	}
	return;
}


void r_error(const gchar *fmt, ...) {
	va_list vl;
	gchar *result = NULL;

	va_start(vl, fmt);
	result = g_strdup_vprintf(fmt, vl);
	va_end(vl);
	if (result != NULL) {
		logMessage(result);
		g_free(result);
	}
	return;
}


#if ! DISABLERINFO
void r_info(const gchar *fmt, ...) {
	va_list vl;
	gchar *result = NULL;

	va_start(vl, fmt);
	result = g_strdup_vprintf(fmt, vl);
	va_end(vl);
	if (result != NULL) {
		logMessage(result);
		g_free(result);
	}
	return;
}
#endif


#if ! DISABLERDEBUG
void r_debug(const gchar *fmt, ...) {
	va_list vl;
	gchar *result = NULL;

	va_start(vl, fmt);
	result = g_strdup_vprintf(fmt, vl);
	va_end(vl);
	if (result != NULL) {
		logMessage(result);
		g_free(result);
	}
	return;
}
#endif


void r_warning(const gchar *fmt, ...) {
	va_list vl;
	gchar *result = NULL;

	va_start(vl, fmt);
	result = g_strdup_vprintf(fmt, vl);
	va_end(vl);
	if (result != NULL) {
		logMessage(result);
		g_free(result);
	}
	return;
}

gint64 tentothe(gint n) {
	/* Previously we kept an array here with 12 elements.
	 * double->gint64 conversion can give corrects values up to 10^18.
	 */
	if (n > 18)
		return 0;
	return (gint64)pow(10.0, (double)n);
}

gchar hextochar(gchar c) {
	gint	c1;
	c1 = toupper(c);
	if(isalpha(c1))
		return c1-'A'+10;
	else
		return c1-'0';

}

const gchar *colornames(const char *str) {
	if(str == NULL)
		return "0x000000";
	if(!isalpha((int)*str))
		return str;
	if(!strcasecmp(str, "black"))
		return "0x000000";
	if(!strcasecmp(str, "silver"))
		return "0xC0C0C0";
	if(!strcasecmp(str, "gray"))
		return "0x808080";
	if(!strcasecmp(str, "white"))
		return "0xFFFFFF";
	if(!strcasecmp(str, "maroon"))
		return "0x800000";
	if(!strcasecmp(str, "red"))
		return "0xFF0000";
	if(!strcasecmp(str, "purple"))
		return "0x800080";
	if(!strcasecmp(str, "fuchsia"))
		return "0xFF00FF";
	if(!strcasecmp(str, "green"))
		return "0x008000";
	if(!strcasecmp(str, "lime"))
		return "0x00FF00";
	if(!strcasecmp(str, "olive"))
		return "0x808000";
	if(!strcasecmp(str, "yellow"))
		return "0xFFFF00";
	if(!strcasecmp(str, "navy"))
		return "0x000080";
	if(!strcasecmp(str, "blue"))
		return "0x0000FF";
	if(!strcasecmp(str, "teal"))
		return "0x008080";
	if(!strcasecmp(str, "aqua"))
		return "0x00FFFF";
	if(!strcasecmp(str, "bobkratz")) //Easter egg.. a pink color to match the shirts he wears
		return "0xffc59f";
	if(!strcasecmp(str, "everton")) //Easter egg.. an ideal background color
		return "0xd3d3d3";
	return str;
}

void rlib_parsecolor(struct rlib_rgb *color, const gchar *strx) {
	const gchar *str = colornames(strx);
	if(str != NULL && r_strlen(str) == 8) {
		guchar r;
		guchar g;
		guchar b;
		r = (hextochar(str[2]) << 4) | hextochar(str[3]);
		g = (hextochar(str[4]) << 4) | hextochar(str[5]);
		b = (hextochar(str[6]) << 4) | hextochar(str[7]);
		color->r = (gfloat)r/(gfloat)0xFF;
		color->g = (gfloat)g/(gfloat)0xFF;
		color->b = (gfloat)b/(gfloat)0xFF;
	} else {
		color->r = -1;
		color->g = -1;
		color->b = -1;
	}
}

struct rlib_datetime * stod(struct rlib_datetime *dt, gchar *str) {
	int year, month, day;
	rlib_datetime_clear(dt);
	if (sscanf(str, "%4d-%2d-%2d", &year, &month, &day) == 3) {
		rlib_datetime_set_date(dt, year, month, day);
	}
	return dt;
}

gchar *strupr (gchar *s) {
	gchar c;
	gchar *ptr = s;
	while ((c = toupper(*s)) != '\0')
		*s++ = c;
	return ptr;
}

gchar *strlwr (gchar *s) {
	gchar c;
	gchar *ptr = s;
	while ((c = tolower(*s)) != '\0')
		*s++ = c;
	return ptr;
}

gchar *strproper (gchar *s) {
	gchar c;
	gchar *ptr = s;
	*s = toupper(*s);
	s++;
	while ((c = tolower(*s)) != '\0')
		*s++ = c;
	return ptr;
}


//TODO: Change this to use a g_string instead of this.. Bob agree's
void make_more_space_if_necessary(gchar **str, gint *size, gint *total_size, gint len) {
	if(*total_size == 0) {
		*str = g_malloc(MAXSTRLEN);
		memset(*str, 0, MAXSTRLEN);
		*total_size = MAXSTRLEN;
	} else if((*size) + len > (*total_size)) {
		*str = g_realloc(*str, (*total_size)*2);
		*total_size = (*total_size) * 2;
	}		
}


/**
 * Parses an encoding description such as en_GB.utf8@euro into it's 3 main parts
 * en_GB, utf8 and euro. Then it recombines the parts using a "utf8" encoding.
 */
gchar *make_utf8_locale(const gchar *encoding) {
	static char result[256];
	gchar *locale, *codeset = NULL, *extra = NULL;
	gchar buf[256];
	gchar *t;
	gint len = r_strlen(encoding);

	if ((encoding == NULL) || (r_strlen(encoding) < 2)) {
		r_warning("encoding is NULL or invalid [%s]... using en_US\n", encoding);
		return (char *)"en_US.utf8";
	}
	g_strlcpy(buf, encoding, sizeof(buf));
	locale = buf;
	t = g_strstr_len(buf, len, ".");
	if (t) {
		*t = '\0';
		codeset = t + 1;
		len = r_strlen(codeset);
	}
	if (codeset) {
		t = g_strstr_len(codeset, len, "@");
		if (t) {
			*t = '\0';
			extra = t + 1;
		}
	}
	codeset = (gchar *)"utf8";
	if (extra) {
		g_snprintf(result, sizeof(buf), "%s.%s@%s", locale, codeset, extra);
	} else {
		g_snprintf(result, sizeof(buf), "%s.%s", locale, codeset);
	}
	return result;
}

void make_all_locales_utf8(void) {
	int *lc = locale_codes;
	int i;
	while ((i = *lc) != -1) {
		char *t = setlocale(i, NULL);
		if (t) {
			if (!setlocale(i, make_utf8_locale(t))) {
				r_error("Setting locale to [%s] FAILED\n", t);
			}
		}
		++lc;
	}
}

//For debug purposes so I can see a hex dump of certain utf8 strings.
inline guint itox(guint i) { return (i < 10)?'0'+i:'A'+i-10; } 
gchar *str2hex(const gchar *str) {
	guint ch;
	gchar *result = g_malloc(2 * strlen(str) + 1);
	gchar *ptr;
	ptr = result;
	while ((ch = *str++)) {
		*ptr++ = itox((ch >> 4) & 0xF);
		*ptr++ = itox(ch & 0xF);
	}
	*ptr = '\0';
	return result;
}

gint64 rlib_safe_atoll(char *str) {
	if(str == NULL)
		return 0;
#if _64BIT_
	return atol(str);
#else
	return atoll(str);
#endif
}

struct rlib_string * rlib_string_new() {
	return g_new0(struct rlib_string, 1);
}

/* THIS IS NOT UTF8 ON PURPOSE */
void rlib_string_append(struct rlib_string *rs, const gchar *str) {
	gint slen = strlen(str);
	if((rs->slen + slen  + 1) > rs->buf_size) {
		rs->buf_size = (rs->buf_size * 2) + slen + 1;
		rs->string = g_realloc(rs->string, rs->buf_size);
	}
	memcpy(rs->string + rs->slen, str, slen+1);
	rs->slen += slen;
}

struct rlib_string * rlib_string_new_with_string(const gchar *string) {
	struct rlib_string *rs = rlib_string_new();
	rlib_string_append(rs, string);
	return rs;
}


void rlib_string_free(struct rlib_string *rs) {
	g_free(rs->string);
	g_free(rs);
}
