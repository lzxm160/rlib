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
 * 
 * $Id$s
 *
 * Definitions and prototypes for utility functions
 */
#ifndef RLIBUTIL_H
#define RLIBUTIL_H

#include <glib.h>

struct rlib_rgb {
	gfloat r;
	gfloat g;
	gfloat b;
};

struct rlib_string {
	gchar *string;
	gint slen;
	gint buf_size;
};

struct rlib_string * rlib_string_new();
void rlib_string_append(struct rlib_string *rs, gchar *str);
void rlib_string_free(struct rlib_string *rs);
struct rlib_string * rlib_string_new_with_string(gchar *string);

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
void rlib_parsecolor(struct rlib_rgb *color, gchar *strx);
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
long long rlib_safe_atoll(char *str);

#if DISABLE_UTF8
#define r_charcount(s) (strlen(s))
#define r_bytecount(s) (strlen(s))
#define r_strchr(s, len, chr) (strchr(s, chr))
#define r_nextchr(s) (s+1)
#define r_getchr(s) (*s)
#define r_strcmp(s1,s2) (strcmp(s1, s2))
#define r_strupr(s) (strupr(s))
#define r_strlwr(s) (strlwr(s))
#define r_ptrfromindex(s, idx) (s+idx)

#else
#define r_charcount(s) (g_utf8_strlen(s, -1))
#define r_bytecount(s) (strlen(s))
#define r_strchr(t, len, chr) (g_utf8_strchr(t, len, chr))
#define r_nextchr(t) (g_utf8_next_char(t))
#define r_getchr(t) (g_utf8_get_char(t))
#define r_strcmp(a,b) (g_utf8_collate(a, b))
#define r_strupr(a) (g_utf8_strup(a, -1))
#define r_strlwr(a) (g_utf8_strdown(a, -1))
#define r_ptrfromindex(s, idx) (g_utf8_offset_to_pointer(s, idx))

#endif

void make_all_locales_utf8();
char *make_utf8_locale(const char *encoding);

#endif

