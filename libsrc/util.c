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

#include "ralloc.h"
#include "rlib.h"

//man 3 llabs says the prototype is in stdlib.. no it aint!
long long int llabs(long long int j);


static void myFaultHandler (int signum, siginfo_t *si, void *aptr) {
	struct rlimit rlim;
	rlogit("** NUTS.. WE CRASHED\n");
	getrlimit (RLIMIT_CORE, &rlim); //POSIBLY NOT NECESSARY
	rlim.rlim_cur = 1024000000; //NECESSARY
	setrlimit (RLIMIT_CORE, &rlim); //NECESSARY
	signal (SIGQUIT, SIG_DFL); //KEEP THIS!!!!!
	kill (getpid(), SIGQUIT); //IMPORTANT
	exit (5); //THEORETICALLY IN THEORY THIS WILL NEVER GET CALLED... but lets play it safe
}

void init_signals() {
	struct sigaction sa;
	bzero(&sa, sizeof(struct sigaction));
	sa.sa_handler = (void(*)(int))myFaultHandler;
	sigaction (SIGILL, &sa, NULL);
	sigaction (SIGBUS, &sa, NULL);
	sigaction (SIGSEGV, &sa, NULL);
	sigaction (SIGABRT, &sa, NULL);
	sigaction (SIGIOT, &sa, NULL);
	sigaction (SIGTRAP, &sa, NULL);
	signal (SIGQUIT, SIG_DFL);
}

int vasprintf(char **, const char *, va_list);

char *strlwrexceptquoted (char *s) {
	char c; 
	char *ptr = s;
	int quote=0;
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


char *rmwhitespacesexceptquoted(char *s) {
	char *backptr = s;
	char *orig = s;
	int spacecount=0;
	int quote=0;
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

void rlogit(const char *fmt, ...) {
	va_list vl;
	char *result = NULL;

	va_start(vl, fmt);
	vasprintf(&result, fmt, vl);
	va_end(vl);
	fprintf(stderr, result);
	if (result != NULL) free(result);
	return;
}

long long tentothe(int n) {
	long long ten[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000, 10000000000, 100000000000, 1000000000000};
	return ten[n];
}

char hextochar(char c) {
	c = toupper(c);
	if(isalpha(c))
		return c-'A'+10;
	else
		return c-'0';

}

char *colornames(char *str) {
	if(!isalpha(*str))
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


void parsecolor(struct rgb *color, char *strx) {
	char *str = colornames(strx);
	if(str != NULL && strlen(str) == 8) {
		unsigned char r;
		unsigned char g;
		unsigned char b;
		r = (hextochar(str[2]) << 4) | hextochar(str[3]);
		g = (hextochar(str[4]) << 4) | hextochar(str[5]);
		b = (hextochar(str[6]) << 4) | hextochar(str[7]);
		color->r = (float)r/(float)0xFF;
		color->g = (float)g/(float)0xFF;
		color->b = (float)b/(float)0xFF;
	} else {
		color->r = -1;
		color->g = -1;
		color->b = -1;
	}
}

struct tm * stod(struct tm *tm_date, char *str) {
	int year, month, day;
	sscanf(str, "%4d-%2d-%2d", &year, &month, &day);
	bzero(tm_date, sizeof(struct tm));
	tm_date->tm_year = year-1900;
	tm_date->tm_mon = month-1;
	tm_date->tm_mday = day;
	return tm_date;
}

char month_dates[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

int daysinmonth(int year, int month) {
	int maxday;
	maxday = month_dates[month];
	if(month == 1 && year % 4 == 0 && year % 100 != 0)
		maxday++;
	return maxday;
}

void bumpday(int *year, int *month, int *day) {
	int maxday;
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

void bumpdaybackwords(int *year, int *month, int *day) {
	int maxday;
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

char *strupr (char *s) {
	char c;
	char *ptr = s;
	while ((c = toupper(*s)) != '\0')
		*s++ = c;
	return ptr;
}

char *strlwr (char *s) {
	char c;
	char *ptr = s;
	while ((c = tolower(*s)) != '\0')
		*s++ = c;
	return ptr;
}

char *strproper (char *s) {
	char c;
	char *ptr = s;
	*s = toupper(*s);
	s++;
	while ((c = tolower(*s)) != '\0')
		*s++ = c;
	return ptr;
}

void make_more_space_if_necessary(char **str, int *size, int *total_size, int len) {
	if(*total_size == 0) {
		*str = rcalloc(MAXSTRLEN, 1);
		*total_size = MAXSTRLEN;
	} else if((*size) + len > (*total_size)) {
		*str = rrealloc(*str, (*total_size)*2);
		*total_size = (*total_size) * 2;
	}		
}
