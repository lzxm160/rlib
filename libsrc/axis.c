/*
 *  Copyright (C) 2003-2004 SICOM Systems, INC.
 *
 *  Authors: Bob Doan <bdoan@sicompos.com>
 *           Michael Ibison <ibison@earthtech.org> (The Math Behind the Graphs and some PSEUDO Code)
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
 * $Id$
 * 
 * This module generates a report from the information stored in the current
 * report object.
 * The main entry point is called once at report generation time for each
 * report defined in the rlib object.
 *
 */
 

#include <stdlib.h>
#include <string.h>
#include <langinfo.h>
#include <math.h>

#include "config.h"
#include "rlib.h"

#define GOOD_CONTRAST_THRESHOLD 0.5
#define FALSE_ORGIN_CONTRAST_THRESHOLD 0.2
#define MAXIMUM_SIGNIFICANT_DIGITS 10

#define MAX_COLOR_POOL 4

static gint dynamic_range(gdouble a, gdouble b) {
	gdouble min, max, d;
	gint n;
	
	if(a == b)
		return 0;
	if(a > b) {
		min = b;
		max = a;
	} else {
		min = a;
		max = b;
	}

	d = fabs(max - min);
	n = floor(log10(d));
	return d* pow(10, 1-n);
}

#define SMALL .00000001

static gint nsd(gdouble x) {
	gint n;
	gdouble y,z;
	gdouble close;
	gint i;
	if(x == 0)
		return 0;
	n = floor(log10(fabs(x)));
	y = fabs(x) / pow(10, n);
	for(i=1;i<=MAXIMUM_SIGNIFICANT_DIGITS;i++) {
		z = trunc(y * pow(10, i)) * pow(10, -i);
		close = fabs((z-y)/y);
		if(close <= SMALL) {
			return i + 1;
		}
	}
	return 0;
}

static gint sd(gdouble x, gdouble m, gint ud) {
	gdouble n;
	n = floor(log10(fabs(x))) - m + 1;
	if(ud == -1)
		return trunc(x/pow(10, n));
	else
		return ceil(x/pow(10,n));
}

static gdouble nice_limit(gdouble z, gint ud) {
	gint t, abs_t, n;
	gint s;

	n = floor(log10(fabs(z))) - 1;
	t = sd(z, 2, ud);
	abs_t = abs(t);

	if(abs_t < 20) {
		if(abs_t == 13 || abs_t == 11 || abs_t == 17 || abs_t == 19)
			t = t + ud;
		return t * pow(10, n);
	}
		
	if(ud == 1) {
		s = ceil(t / 5.0);	
	} else {
		s = floor(t / 5.0);
	}

	if(s == 19)
		s = 20;
	return 5.0 * pow(10, n) * s;
}

static gdouble round_sd(gdouble z, gint m, gint ud) {
	gint n = floor(log10(fabs(z))) + 1 - m;
	gint t = sd(z, m, ud);
	return t * pow(10, n);
}

static gdouble change_limit(gdouble min, gdouble max, gint ud) {
	gdouble pdr = 1.0/15.0;
	gdouble t;
	gint sd;
	if(min == 0)
		return 0;
		
	sd = MAX(nsd(min), nsd(max));
	
	if(ud == 1) 
		t = max + (pdr * (max - min));
	else
		t = min - (pdr * (max - min));
		
	if(((min > 0 && t < 0) || (min == 0)) && ud == -1)
		return 0;
	return round_sd(t,sd,ud);
}

gint rlib_graph_num_ticks(rlib *r, gdouble a, gdouble b) {
	gdouble min, max;
	gint d = dynamic_range(a, b);
	gint sd;
	if(a > b) {
		min = b;
		max = a;	
	} else {
		min = a;
		max = b;
	}
	
	sd = nsd(max - min);
	
	if(sd <= 2) {	
		if(d == 1 || d == 2 || d == 5 || d == 20 || d == 50 || d == 100 || d == 10)
			return 10;
		else if(d == 4 || d == 8 || d == 40 || d == 80 || d == 16)
			return 8;
		else if(d == 3 || d == 6 || d == 30 || d == 60 || d == 12 || d == 24)
			return 12;
		else if(d == 7 || d == 70 || d == 35 || d == 14 || d == 21 || d == 28)
			return 7;
		else if(d == 18 || d == 90 || d == 45 || d == 9 || d == 27)
			return 9;
		else if(d == 25)
			return 5;
		else if(d == 11 || d == 55 || d == 22 || d == 33)
			return 11;
		else if(d == 13 || d == 65 || d == 26)
			return 13;
		else if(d == 15 || d == 75)
			return 15;
		else if(d == 85)
			return 17;
		else if(d == 95)
			return 19;		
		return d;
	}
	return 10;
}

void rlib_graph_find_y_range(rlib *r, gdouble a, gdouble b, gdouble *y_min, gdouble *y_max, gint graph_type) {
	gdouble contrast;
	gdouble min, max;

	if(a == b) {
		*y_min = *y_max = a;
		return;
	}
	if(a > b) {
		min = b;
		max = a;
	} else {
		min = a;
		max = b;	
	}

	if(graph_type == RLIB_GRAPH_TYPE_ROW_NORMAL || graph_type == RLIB_GRAPH_TYPE_ROW_STACKED || graph_type == RLIB_GRAPH_TYPE_ROW_PERCENT) {
		gdouble mint = change_limit(min, max, -1);		
		max = change_limit(min, max, 1);		
		min = mint;

	}
		
	contrast = fabs((max - min) / (max + min));

	if((max == 0) || (min < 0 && contrast > FALSE_ORGIN_CONTRAST_THRESHOLD)) {
		if(max <= 0)
			*y_max = 0;
		else
			*y_max = nice_limit(max, 1);
		*y_min = nice_limit(min, -1);
		return;
	}
	if((min == 0) || (max > 0 && contrast > FALSE_ORGIN_CONTRAST_THRESHOLD)) {
		if(min >= 0)
			*y_min = 0;
		else
			*y_min = nice_limit(min, -1);
		*y_max = nice_limit(max, 1);
		return;
	}
	*y_min = min;
	*y_max = max;
	return;
}	
