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

#define MAX_COLOR_POOL 32
gchar *color_pool[MAX_COLOR_POOL] = {"0x9999ff", "0x993366", "0xffffcc", "0xccffff", "0x660066", "0xff8080", "0x0066cc", "0xccccff", "0x000080", 
	"0xff00ff", "0xffff00", "0x00ffff", "0x800080", "0x800000", "0x008080", "0x0000ff", "0x00ccff", "0xccffff", "0xccffcc", "0xffff99", "0x99ccff", 
	"0xff99cc", "0xcc99ff", "0xffcc99", "0x3366ff", "0x33cccc", "0x99cc00", "0xffcc00", "0xff9900", "0xff6600", "0x666699", "0x969696"};

static gboolean is_row_graph(gint graph_type) {
	if(graph_type == RLIB_GRAPH_TYPE_ROW_NORMAL || graph_type == RLIB_GRAPH_TYPE_ROW_PERCENT || graph_type == RLIB_GRAPH_TYPE_ROW_STACKED)
		return TRUE;
	return FALSE;
}

static gboolean is_line_graph(gint graph_type) {
	if(graph_type == RLIB_GRAPH_TYPE_LINE_NORMAL || graph_type == RLIB_GRAPH_TYPE_LINE_PERCENT || graph_type == RLIB_GRAPH_TYPE_LINE_STACKED)
		return TRUE;
	return FALSE;
}

static gboolean is_pie_graph(gint graph_type) {
	if(graph_type == RLIB_GRAPH_TYPE_PIE_NORMAL || graph_type == RLIB_GRAPH_TYPE_PIE_RING || graph_type == RLIB_GRAPH_TYPE_PIE_OFFSET)
		return TRUE;
	return FALSE;
}

static gboolean is_percent_graph(gint graph_type) {
	if(graph_type == RLIB_GRAPH_TYPE_ROW_PERCENT || graph_type == RLIB_GRAPH_TYPE_LINE_PERCENT)
		return TRUE;
	return FALSE;
}

static gboolean is_stacked_graph(gint graph_type) {
	if(graph_type == RLIB_GRAPH_TYPE_ROW_STACKED || graph_type == RLIB_GRAPH_TYPE_LINE_STACKED)
		return TRUE;
	return FALSE;
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

static void rlib_graph_label_y_axis(rlib *r, gboolean for_real, gint y_ticks, gdouble y_min, gdouble y_max, gdouble y_origin) {
	gint i;
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

		if(val == y_origin)
			special = TRUE;
		if(for_real) 
			OUTPUT(r)->graph_label_y(r, i, label, special);	
		else
			OUTPUT(r)->graph_hint_label_y(r, label);	
	}
}

#define POSITIVE 1
#define POSITIVE_AND_NEGATIVE 2
#define NEGATIVE 3

void rlib_graph(rlib *r, struct rlib_part *part, struct rlib_report *report, gfloat left_margin_offset, gfloat top_margin_offset) {
	struct rlib_graph_plot *plot;
	struct rlib_graph *graph = &report->graph;
	GSList *list;
	gchar axis[MAXSTRLEN], x_axis_label[MAXSTRLEN], legend_label[MAXSTRLEN];
	gfloat y_value;
	gfloat stacked_y_value_max=0;
	gfloat stacked_y_value_min=0;
	gfloat y_value_try_max;
	gfloat y_value_try_min;
	gfloat col_sum = 0;
	gfloat running_col_sum = 0;
	gdouble y_min = 999999999;
	gdouble y_max = -999999999;
	gfloat *row_sum = NULL, *last_row_values=NULL, *last_row_height=NULL;
	gint data_plot_count = 0;
	gint plot_count = 0;
	gint row_count = 0;
	gint y_ticks = 10;
	gint i = 0;
	gfloat tmp;
	gfloat graph_width=300, graph_height=300;
	gfloat last_height,last_height_neg,last_height_pos;
	gfloat y_origin = 0;
	gchar data_type = POSITIVE; 
	struct rlib_rgb color[MAX_COLOR_POOL];
	gchar type[MAXSTRLEN], subtype[MAXSTRLEN], title[MAXSTRLEN], x_axis_title[MAXSTRLEN], y_axis_title[MAXSTRLEN];
	gint graph_type;
	gboolean divide_iterations = TRUE;
	gboolean should_label_under_tick = FALSE;

	left_margin_offset += part->left_margin;

		
	if(rlib_execute_as_float(r, graph->width_code, &tmp))
		graph_width = tmp;
	if(rlib_execute_as_float(r, graph->height_code, &tmp))
		graph_height = tmp;
	if(!rlib_execute_as_string(r, graph->type_code, type, MAXSTRLEN))
		type[0] = 0;
	if(!rlib_execute_as_string(r, graph->subtype_code, subtype, MAXSTRLEN))
		subtype[0] = 0;
	if(!rlib_execute_as_string(r, graph->title_code, title, MAXSTRLEN))
		title[0] = 0;
	if(!rlib_execute_as_string(r, graph->x_axis_title_code, x_axis_title, MAXSTRLEN))
		x_axis_title[0] = 0;
	if(!rlib_execute_as_string(r, graph->y_axis_title_code, y_axis_title, MAXSTRLEN))
		y_axis_title[0] = 0;

	if(!rlib_will_this_fit(r, part, report, graph_height / RLIB_PDF_DPI, 1)) {
		top_margin_offset = 0;
		rlib_layout_end_page(r, part, report);
		if(report->font_size != -1) {
			r->font_point = report->font_size;
			OUTPUT(r)->set_font_point(r, r->font_point);
		}
	}
	
	graph_type = determine_graph_type(type, subtype);	

	if(is_line_graph(graph_type))
		should_label_under_tick = TRUE;

	if(graph_type == RLIB_GRAPH_TYPE_ROW_STACKED || graph_type == RLIB_GRAPH_TYPE_ROW_PERCENT) 
		divide_iterations = FALSE;
			
	OUTPUT(r)->graph_start(r, left_margin_offset, rlib_layout_get_next_line(r, part, part->position_top[0]+top_margin_offset, 0), graph_width, graph_height, should_label_under_tick);
	
	rlib_fetch_first_rows(r);
	if(!INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result)) {
		while (1) {
			data_plot_count = 0;
			stacked_y_value_max = 0;
			stacked_y_value_min = 0;
			rlib_process_input_metadata(r);
			row_sum = g_realloc(row_sum, (row_count+1) * sizeof(gfloat));
			row_sum[row_count] = 0;
			for(list=graph->plots;list != NULL; list = g_slist_next(list)) {
				plot = list->data;
				if(rlib_execute_as_string(r, plot->axis_code, axis, MAXSTRLEN)) {
					if(strcmp(axis, "x") == 0) {
						if(rlib_execute_as_string(r, plot->field_code, x_axis_label, MAXSTRLEN)) {
							OUTPUT(r)->graph_hint_label_x(r, x_axis_label);
						}																
				
					} else if(strcmp(axis, "y") == 0) {
						if(rlib_execute_as_float(r, plot->field_code, &y_value)) {
							if(row_count == 0) {
								if(rlib_execute_as_string(r, plot->label_code, legend_label, MAXSTRLEN)) {
									OUTPUT(r)->graph_hint_legend(r, legend_label);
								}
							}
							
							if(is_percent_graph(graph_type) || is_pie_graph(graph_type)) {
								y_value = fabs(y_value);
								row_sum[row_count] += y_value;
							}
							
							if(is_stacked_graph(graph_type)) { 
								if( y_value >= 0 ) {
									if( stacked_y_value_max < 0 ) 
										stacked_y_value_max = y_value;
									else
										stacked_y_value_max += y_value;
									stacked_y_value_min = y_value;
								} else { 
									if( stacked_y_value_min > 0 ) {
										stacked_y_value_min = 0;
									}
									stacked_y_value_min += y_value;
									stacked_y_value_max = y_value;
								}
								y_value_try_max = stacked_y_value_max;
								y_value_try_min = stacked_y_value_min;
							} else {
								y_value_try_max = y_value;
								y_value_try_min = y_value;
							}
							rlib_parsecolor(&color[data_plot_count%MAX_COLOR_POOL], color_pool[data_plot_count%MAX_COLOR_POOL]);
							data_plot_count++;

							if(y_value_try_min < y_min)
								y_min = y_value_try_min;
							if(y_value_try_max > y_max)
								y_max = y_value_try_max;
						}
						if(is_pie_graph(graph_type)) {
							col_sum += fabs(y_value);
							rlib_parsecolor(&color[row_count%MAX_COLOR_POOL], color_pool[row_count%MAX_COLOR_POOL]);			
							if(rlib_execute_as_string(r, plot->label_code, legend_label, MAXSTRLEN)) {
								OUTPUT(r)->graph_hint_legend(r, legend_label);
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

	if(is_percent_graph(graph_type) || is_pie_graph(graph_type)) {
		y_min = 0;
		y_max = 100;
		y_ticks = 10;
	} else {
		rlib_graph_find_y_range(r, y_min, y_max, &y_min, &y_max, graph_type);
		y_ticks = rlib_graph_num_ticks(r, y_min, y_max);
	}

	rlib_graph_label_y_axis(r, FALSE, y_ticks, y_min, y_max, y_origin);

	if(is_pie_graph(graph_type)) {
		OUTPUT(r)->graph_set_data_plot_count(r, row_count);
		OUTPUT(r)->graph_draw_legend(r);
		i = 1;
	} else {	
		OUTPUT(r)->graph_set_data_plot_count(r, data_plot_count);
		OUTPUT(r)->graph_draw_legend(r);
		i=0;
		for(list=graph->plots;list != NULL; list = g_slist_next(list)) {
			plot = list->data;
			if(rlib_execute_as_string(r, plot->axis_code, axis, MAXSTRLEN)) {
				if(strcmp(axis, "y") == 0) {
					if(rlib_execute_as_string(r, plot->label_code, legend_label, MAXSTRLEN)) {
						OUTPUT(r)->graph_draw_legend_label(r, i, legend_label, &color[i]);
					}
					i++;
				}
			}
		}
	}
	
	last_row_values = g_malloc(i * sizeof(gfloat));
	last_row_height = g_malloc(i * sizeof(gfloat));

	OUTPUT(r)->graph_title(r, title);
	if(!is_pie_graph(graph_type)) {
		OUTPUT(r)->graph_x_axis_title(r, x_axis_title);
		OUTPUT(r)->graph_y_axis_title(r, y_axis_title);
		OUTPUT(r)->graph_set_x_iterations(r, row_count);
		OUTPUT(r)->graph_do_grid(r, FALSE);
		OUTPUT(r)->graph_tick_x(r);
		OUTPUT(r)->graph_tick_y(r, y_ticks);
	} else {
		OUTPUT(r)->graph_do_grid(r, TRUE);
	}
	
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
	
	if(!is_pie_graph(graph_type)) {
		OUTPUT(r)->graph_set_limits(r, y_min, y_max, y_origin);
		rlib_graph_label_y_axis(r, TRUE, y_ticks, y_min, y_max, y_origin);
	}

	row_count = 0;

	rlib_fetch_first_rows(r);
	if(!INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result)) {
		while (1) {
			rlib_process_input_metadata(r);
			data_plot_count = 0;
			last_height_pos=0;
			last_height_neg=0;
			last_height=0;
			i=0;
			for(list=graph->plots;list != NULL; list = g_slist_next(list)) {
				plot = list->data;
				if(rlib_execute_as_string(r, plot->axis_code, axis, MAXSTRLEN)) {
					if(strcmp(axis, "x") == 0) {
						if(rlib_execute_as_string(r, plot->field_code, x_axis_label, MAXSTRLEN)) {
							OUTPUT(r)->graph_label_x(r, row_count, x_axis_label);
						}																
					} else if(strcmp(axis, "y") == 0) {
						if(rlib_execute_as_float(r, plot->field_code, &y_value)) {
							if(is_percent_graph(graph_type) || is_pie_graph(graph_type)) {
								y_value = fabs(y_value);
								if(is_pie_graph(graph_type))
									y_max = col_sum;
								else
									y_max = row_sum[row_count];
								y_min = 0;
							}

							gfloat value_a = 0;
						
							if(data_type == POSITIVE_AND_NEGATIVE) {
								if(value_a >= 0)
									value_a = (y_value / y_max) * (fabs(y_max)/(fabs(y_min)+fabs(y_max)));
								else
									value_a = (y_value / y_min) * (fabs(y_min)/(fabs(y_min)+fabs(y_max)));
							} else if(data_type == NEGATIVE) {
								value_a = -((y_value - y_origin) / (y_min - y_origin));
							} else {
								value_a = (y_value - y_origin) / (y_max - y_origin);
							}
							plot_count = data_plot_count;
							if(is_percent_graph(graph_type) || is_stacked_graph(graph_type) || is_pie_graph(graph_type)) { 
								plot_count = 0;
								if( value_a >= 0 ) 
									last_height = last_height_pos;
								else
									last_height = last_height_neg;
							} 
							if(is_row_graph(graph_type)) {
								OUTPUT(r)->graph_plot_bar(r, row_count, plot_count, value_a, &color[data_plot_count],last_height, divide_iterations);
							} else if(is_line_graph(graph_type)) {
								if(row_count > 0)
									OUTPUT(r)->graph_plot_line(r, row_count, last_row_values[i], last_row_height[i], value_a, last_height, &color[data_plot_count]);
							} else if(is_pie_graph(graph_type)) {
								gboolean offset = graph_type == RLIB_GRAPH_TYPE_PIE_OFFSET;
								OUTPUT(r)->graph_plot_pie(r, running_col_sum, value_a+running_col_sum, offset, &color[row_count]);
								running_col_sum += value_a;
								if(rlib_execute_as_string(r, plot->label_code, legend_label, MAXSTRLEN))
									OUTPUT(r)->graph_draw_legend_label(r, row_count, legend_label, &color[row_count]);
					
							}
								
							if(is_percent_graph(graph_type) || is_stacked_graph(graph_type) || is_pie_graph(graph_type)) { 
								if(value_a >= 0 ) 
									last_height_pos += value_a;
								else
									last_height_neg += value_a;
							}
							
							last_row_values[i] = value_a;
							last_row_height[i] = last_height;
							i++;
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
	g_free(row_sum);
	g_free(last_row_values);
	g_free(last_row_height);
}
