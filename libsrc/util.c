/*
 *  Copyright (C) 2003 SICOM Systems, INC.
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
#include <sys/resource.h>

#include "rlib.h"

//man 3 llabs says the prototype is in stdlib.. no it aint!
gint64 llabs(gint64 j);


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


static gint useMyHandler = TRUE;

void init_signals() {
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
}

gint rutil_enableSignalHandler(gint trueorfalse) {
	gint whatitwas = useMyHandler;
	useMyHandler = trueorfalse;
	return whatitwas;
}

gint vasprintf(gchar **, const gchar *, va_list);

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
	gint64 ten[] = {1LL, 10LL, 100LL, 1000LL, 10000LL, 100000LL, 1000000LL, 10000000LL, 100000000LL, 1000000000LL, 10000000000LL, 100000000000LL, 1000000000000LL};
	return ten[n];
}

gchar hextochar(gchar c) {
	c = toupper(c);
	if(isalpha((int)c))
		return c-'A'+10;
	else
		return c-'0';

}

gchar *colornames(char *str) {
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
	return str;
}


void parsecolor(struct rgb *color, gchar *strx) {
	gchar *str = colornames(strx);
	if(str != NULL && bytelength(str) == 8) {
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

#if 0
gchar month_dates[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

gint daysinmonth(gint year, gint month) {
	return g_date_get_days_in_month(year, month);
}
void bumpday(gint *year, gint *month, gint *day) {
	gint maxday;
	*day = *day + 1;
	maxday = month_dates[(*month)];
	if(*month == 1 && *year % 4 == 0 && *year % 100 != 0)
		maxday++;
	if(*day > maxday) {
		(*month)++;
		*day=1;
	}
	if(*month > 11) {
		*month=1;
		(*year)++;
	}
}

void bumpdaybackwords(gint *year, gint *month, gint *day) {
	gint maxday;
	*day = *day - 1;
	if(*day == 0) {
		maxday = month_dates[(*month)-1];
		if(*month == 1 && *year % 4 == 0 && *year % 100 != 0)
			maxday++;
		*day = maxday;
		*month = *month - 1;
		if(*month == -1) {
			*month=11;
			(*year)--;
		}
	}
}
#endif

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


const char *encode(iconv_t cd, const char *txt) {
	size_t len = MAXSTRLEN;
	size_t slen;
	static char encodebuf[MAXSTRLEN];
	char *dest = encodebuf;
	int result = 0;
	const char *ret = NULL;

	encodebuf[0] = '\0';
	if ((txt != NULL) && (*txt != '\0')) {
		if (cd != (iconv_t) -1) {
			slen = bytelength(txt);
#if ICONV_CONST_CHAR_PP
			result = iconv(cd, (const char **) &txt, &slen, &dest, &len);
#else
			result = iconv(cd, (char **)&txt, &slen, &dest, &len);
#endif
			*dest = '\0';
			ret = encodebuf;
		} else {
			ret = txt;
		}
	}
	return ret;
}
