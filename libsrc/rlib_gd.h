/*
 *  Copyright (C) 2003-2004 SICOM Systems, INC.
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

#include "config.h"

#ifdef HAVE_GD
#include <gd.h>
#include <gdfontt.h>
#include <gdfonts.h>
#include <gdfontmb.h>
#include <gdfontl.h>
#include <gdfontg.h>
#endif

struct rlib_gd {
#ifdef HAVE_GD
	gdImagePtr im;
#else
	void *im;
#endif
	gchar *file_name;
	int white;
	int black;
};


/***** PROTOTYPES: gd.c ********************************************************/
struct rlib_gd * rlib_gd_new(gint width, gint height);
int rlib_gd_free(struct rlib_gd *rgd);
int rlib_gd_spool(struct rlib_gd *rgd);
int rlib_gd_text(struct rlib_gd *rgd, char *text, int x, int y, int rotate);
int rlib_gd_get_string_width(struct rlib_gd *rgd, char *text);
int rlib_gd_get_string_height(struct rlib_gd *rgd);
int rlib_gd_line(struct rlib_gd *rgd, gint x1, gint y1, gint x2, gint y2, struct rlib_rgb *color);
int rlib_gd_rectangle(struct rlib_gd *rgd, gint x1, gint y1, gint x2, gint y2, struct rlib_rgb *color);
int rlib_gd_arc(struct rlib_gd *rgd, gint x, gint y, gint radius, gint start_angle, gint end_angle, struct rlib_rgb *color);
