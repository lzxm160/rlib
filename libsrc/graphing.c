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
#include "pcode.h"
#include "rlib_input.h"
#include "util.h"

#define GOOD_CONTRAST_THRESHOLD 0.5
#define FALSE_ORGIN_CONTRAST_THRESHOLD 0.2
#define MAXIMUM_SIGNIFICANT_DIGITS 10

#define MAX_COLOR_POOL 4
gchar *color_pool[MAX_COLOR_POOL] = {"blue", "yellow", "green", "red"};

static gint dynamic_range(gfloat a, gfloat b) {
	gfloat min, max, d;
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

static gint num_ticks(gfloat a, gfloat b) {
	gfloat min, max;
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

r_error("nice_limit:: %lf %d\n", z, ud);
	
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

static gfloat round_sd(gfloat z, gint m, gint ud) {
	gint n = floor(log10(fabs(z))) + 1 - m;
	gint t = sd(z, m, ud);
	return t * pow(10, n);
}

static gfloat change_limit(gdouble min, gdouble max, gint ud) {
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

static void find_y_range(gdouble a, gdouble b, gdouble *y_min, gdouble *y_max, gint graph_type) {
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
		gfloat mint = change_limit(min, max, -1);		
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

gint determine_graph_type(gchar *type, gchar *subtype) {
	if(strcmp(type, "line") == 0) {
		if(strcmp(subtype, "stacked") == 0)
			return RLIB_GRAPH_TYPE_LINE_STACKED;
		else if(strcmp(subtype, "percent") == 0)
			return RLIB_GRAPH_TYPE_LINE_PERCENT;
		else
			return RLIB_GRAPH_TYPE_LINE_NORMAL;
	} else if(strcmp(type, "area") == 0) {
		if(strcmp(subtype, "stacked") == 0)
			return RLIB_GRAPH_TYPE_AREA_STACKED;
		else if(strcmp(subtype, "percent") == 0)
			return RLIB_GRAPH_TYPE_AREA_PERCENT;
		else
			return RLIB_GRAPH_TYPE_AREA_NORMAL;
	} else if(strcmp(type, "column") == 0) {
		if(strcmp(subtype, "stacked") == 0)
			return RLIB_GRAPH_TYPE_COLUMN_STACKED;
		else if(strcmp(subtype, "percent") == 0)
			return RLIB_GRAPH_TYPE_COLUMN_PERCENT;
		else
			return RLIB_GRAPH_TYPE_COLUMN_NORMAL;
	} else if(strcmp(type, "row") == 0) {
		if(strcmp(subtype, "stacked") == 0)
			return RLIB_GRAPH_TYPE_ROW_STACKED;
		else if(strcmp(subtype, "percent") == 0)
			return RLIB_GRAPH_TYPE_ROW_PERCENT;
		else
			return RLIB_GRAPH_TYPE_ROW_NORMAL;
	} else if(strcmp(type, "pie") == 0) {
		if(strcmp(subtype, "ring") == 0)
			return RLIB_GRAPH_TYPE_PIE_RING;
		else if(strcmp(subtype, "offset") == 0)
			return RLIB_GRAPH_TYPE_PIE_OFFSET;
		else
			return RLIB_GRAPH_TYPE_PIE_NORMAL;	
	} else if(strcmp(type, "xy") == 0) {
		if(strcmp(subtype, "lines with symbols") == 0)
			return RLIB_GRAPH_TYPE_XY_LINES_WITH_SUMBOLS;
		else if(strcmp(subtype, "lines only") == 0)
			return RLIB_GRAPH_TYPE_XY_LINES_ONLY;
		else if(strcmp(subtype, "cubic spline") == 0)
			return RLIB_GRAPH_TYPE_XY_CUBIC_SPLINE;
		else if(strcmp(subtype, "cubic spline with symbols") == 0)
			return RLIB_GRAPH_TYPE_XY_CUBIC_SPLINE_WIHT_SYMBOLS;
		else if(strcmp(subtype, "bsplibe") == 0)
			return RLIB_GRAPH_TYPE_XY_BSPLINE;
		else if(strcmp(subtype, "bspline with symbols") == 0)
			return RLIB_GRAPH_TYPE_XY_BSPLINE_WITH_SYMBOLS;
		else
			return RLIB_GRAPH_TYPE_XY_SYMBOLS_ONLY;
	}
	return RLIB_GRAPH_TYPE_ROW_NORMAL;
}

#define POSITIVE 1
#define POSITIVE_AND_NEGATIVE 2
#define NEGATIVE 3

void rlib_graph(rlib *r, struct rlib_part *part, struct rlib_report *report, gfloat left_margin_offset, gfloat top_margin_offset) {
	struct rlib_graph_plot *plot;
	struct rlib_graph *graph = &report->graph;
	GSList *list;
	gchar axis[MAXSTRLEN], x_axis_label[MAXSTRLEN];
	gfloat y_value;
	gdouble y_min = 0;
	gdouble y_max = 0;
	gint data_plot_count = 0;
	gint row_count = 0;
	gint i;
	gint y_ticks = 10;
	gfloat tmp;
	gfloat graph_width=300, graph_height=300;
	gfloat y_origin = 0;
	gchar data_type = POSITIVE; 
	
	struct rlib_rgb color[MAX_COLOR_POOL];
	gchar type[MAXSTRLEN], subtype[MAXSTRLEN], title[MAXSTRLEN], x_axis_title[MAXSTRLEN], y_axis_title[MAXSTRLEN];
	gint graph_type;
		
	if(rlib_execute_as_float(r, graph->width_code, &tmp))
		graph_width = tmp;
	if(rlib_execute_as_float(r, graph->height_code, &tmp))
		graph_height = tmp;
	if(!rlib_execute_as_string(r, graph->type_code, type, MAXSTRLEN))
		type[0] = 0;
	if(!rlib_execute_as_string(r, graph->subtype_code, type, MAXSTRLEN))
		subtype[0] = 0;
	if(!rlib_execute_as_string(r, graph->title_code, title, MAXSTRLEN))
		title[0] = 0;
	if(!rlib_execute_as_string(r, graph->x_axis_title_code, x_axis_title, MAXSTRLEN))
		x_axis_title[0] = 0;
	if(!rlib_execute_as_string(r, graph->y_axis_title_code, y_axis_title, MAXSTRLEN))
		y_axis_title[0] = 0;

	graph_type = determine_graph_type(type, subtype);	
			
	OUTPUT(r)->graph_start(r, left_margin_offset, rlib_layout_get_next_line(r, part, part->position_top[0]+top_margin_offset, 0), graph_width, graph_height);
	OUTPUT(r)->graph_title(r, title);
	OUTPUT(r)->graph_x_axis_title(r, x_axis_title);
	OUTPUT(r)->graph_y_axis_title(r, y_axis_title);
	
	rlib_fetch_first_rows(r);
	if(!INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result)) {
		while (1) {
			data_plot_count = 0;
			rlib_process_input_metadata(r);
			for(list=graph->plots;list != NULL; list = g_slist_next(list)) {
				plot = list->data;
				if(rlib_execute_as_string(r, plot->axis_code, axis, MAXSTRLEN)) {
					if(strcmp(axis, "x") == 0) {
				
					} else if(strcmp(axis, "y") == 0) {
						if(rlib_execute_as_float(r, plot->field_code, &y_value)) {
							rlib_parsecolor(&color[data_plot_count], color_pool[data_plot_count]);
							data_plot_count++;
							if(row_count == 0) {
								y_min = y_value;
								y_max = y_value;
							} else {
								if(y_value < y_min)
									y_min = y_value;
								if(y_value > y_max)
									y_max = y_value;
							}
						
						}
					}				
				}
			}
			row_count++;

			if(rlib_navigate_next(r, r->current_result) == FALSE) 
				break;
		}
	}
	OUTPUT(r)->graph_do_grid(r);
	OUTPUT(r)->graph_tick_x(r, row_count);
	OUTPUT(r)->graph_set_data_plot_count(r, data_plot_count);
	rlogit("Me have %d rows AND THE MIN IS %f and the MAX IS %f\n", row_count, y_min, y_max);
	find_y_range(y_min, y_max, &y_min, &y_max, graph_type);
	y_ticks = num_ticks(y_min, y_max);
	rlogit("Mike.. We have %d rows AND THE MIN IS %f and the MAX IS %f and ticks is %d\n", row_count, y_min, y_max, y_ticks);
	OUTPUT(r)->graph_tick_y(r, y_ticks);
	
	if(y_max >= 0 && y_min >= 0) {
		y_origin = y_min;
		data_type = POSITIVE;
	} else if(y_max > 0 && y_min < 0) {
		y_origin = 0;
		data_type = POSITIVE_AND_NEGATIVE;
	} else if(y_max <= 0 && y_min < 0) {
		y_origin = y_max;
		data_type = NEGATIVE;
	}
	OUTPUT(r)->graph_set_limits(r, y_min, y_max, y_origin);
	
	for(i=0;i<y_ticks+1;i++) {
		gboolean special = FALSE;
		gdouble val = y_min + (((y_max-y_min)/y_ticks)*i);
		gchar label[MAXSTRLEN];
		if((gint)(val * 100.0) % 100 > 0)
			sprintf(label, "%.02f", val);
		else if((gint)(val * 10000.0) % 10000 > 0)
			sprintf(label, "%.04f", val);
		else
			sprintf(label, "%.f", val);

		if(val == y_origin) {
			special = TRUE;
		}
		OUTPUT(r)->graph_label_y(r, i, label, special);	
	}

	row_count = 0;

	rlib_fetch_first_rows(r);
	if(!INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result)) {
		while (1) {
			rlib_process_input_metadata(r);
			data_plot_count = 0;
			for(list=graph->plots;list != NULL; list = g_slist_next(list)) {
				plot = list->data;
				if(rlib_execute_as_string(r, plot->axis_code, axis, MAXSTRLEN)) {
					if(strcmp(axis, "x") == 0) {
						if(rlib_execute_as_string(r, plot->field_code, x_axis_label, MAXSTRLEN)) {
							OUTPUT(r)->graph_label_x(r, row_count, x_axis_label);						
						
						}																
					} else if(strcmp(axis, "y") == 0) {
						if(rlib_execute_as_float(r, plot->field_code, &y_value)) {
							gfloat value = 0;
							if(data_type == POSITIVE_AND_NEGATIVE) {
								if(value >= 0)
									value = (y_value / y_max) * (fabs(y_max)/(fabs(y_min)+fabs(y_max)));
								else
									value = (y_value / y_min) * (fabs(y_min)/(fabs(y_min)+fabs(y_max)));
							} else if(data_type == NEGATIVE) {
								value = -((y_value - y_origin) / (y_min - y_origin));
							} else {
								value = (y_value - y_origin) / (y_max - y_origin);
							}
							r_error("PLOTTING %lf %d\n", value, data_type);
							OUTPUT(r)->graph_draw_bar(r, row_count, data_plot_count, value, &color[data_plot_count]);
						}
						data_plot_count++;
					}				
				}
			}
			row_count++;

			if(rlib_navigate_next(r, r->current_result) == FALSE)
				break;
		}
	}
	report->position_top[0] += graph_height / RLIB_PDF_DPI;
}
