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
 * 
 * $Id$s
 *
 * Definitions and prototypes for utility functions
 */
#ifndef RLIBUTIL_H
#define RLIBUTIL_H

#include <glib.h>


struct rgb {
	gfloat r;
	gfloat g;
	gfloat b;
};


gchar *strlwrexceptquoted (gchar *s);
gchar *rmwhitespacesexceptquoted(gchar *s);
void rlogit(const gchar *fmt, ...);
#if DISABLERDEBUG
#define r_debug(...)
#else
void r_debug(const gchar *fmt, ...);
#endif
#if DISABLERINFO
#define R_info(...)
#else
void r_info(const gchar *fmt, ...);
#endif
void r_warning(const gchar *fmt, ...);
void r_error(const gchar *fmt, ...);
//coming soon: void r_fatal(const gchar *fmt, ...);
void rlogit_setmessagewriter(void(*writer)(const gchar *msg));
gint rutil_enableSignalHandler(gint trueorfalse);
gint64 tentothe(gint n);
gchar hextochar(gchar c);
gchar *colornames(gchar *str);
void parsecolor(struct rgb *color, gchar *strx);
struct rlib_datetime * stod(struct rlib_datetime *tm_date, gchar *str);
void bumpday(gint *year, gint *month, gint *day);
void bumpdaybackwords(gint *year, gint *month, gint *day);
gchar *strupr (gchar *s);
gchar *strlwr (gchar *s);
gchar *strproper (gchar *s);
gint daysinmonth(gint year, gint month);
void init_signals(void);
void make_more_space_if_necessary(gchar **str, gint *size, gint *total_size, gint len);
gchar *str2hex(const gchar *str);

#define charcount(s) g_utf8_strlen(s, -1)
#define bytelength(s) strlen(s)

void make_all_locales_utf8();
char *make_utf8_locale(const char *encoding);


#endif

