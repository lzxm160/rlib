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
gchar *color_pool[MAX_COLOR_POOL] = {"0x9999ff", "0x993366", "0xfcfa44", "0xccffff", "0x660066", "0xff8080", "0x0066cc", "0xccccff", "0x000080", 
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


static void rlib_graph_label_y_axis(rlib *r, gint side, gboolean for_real, gint y_ticks, gdouble y_min, gdouble y_max, gdouble y_origin) {
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
			OUTPUT(r)->graph_label_y(r, side, i, label, special);	
		else
			OUTPUT(r)->graph_hint_label_y(r, side, label);	
	}
}

#define POSITIVE 1
#define POSITIVE_AND_NEGATIVE 2
#define NEGATIVE 3

gfloat rlib_graph(rlib *r, struct rlib_part *part, struct rlib_report *report, gfloat left_margin_offset, gfloat *top_margin_offset) {
	struct rlib_graph_plot *plot;
	struct rlib_graph *graph = &report->graph;
	GSList *list;
	gchar axis[MAXSTRLEN], x_axis_label[MAXSTRLEN], legend_label[MAXSTRLEN];
	gfloat y_value;
	gfloat stacked_y_value_max[2] = {0,0};
	gfloat stacked_y_value_min[2] = {0,0};
	gfloat y_value_try_max[2];
	gfloat y_value_try_min[2];
	gfloat col_sum = 0;
	gdouble tmi;
	gfloat running_col_sum = 0;
	gdouble y_min[2] = {0,0};
	gdouble y_max[2] = {0,0};
	gfloat *row_sum = NULL, *last_row_values=NULL, *last_row_height=NULL;
	gint data_plot_count = 0;
	gint plot_count = 0;
	gint row_count = 0;
	gint y_ticks = 10, fake_y_ticks;
	gint i = 0;
	gboolean have_right_side = FALSE;
	gint side = RLIB_SIDE_LEFT;
	gfloat tmp;
	gfloat graph_width=300, graph_height=300;
	gfloat last_height,last_height_neg,last_height_pos;
	gfloat y_origin[2] = {0,0};
	gchar data_type[2] = {POSITIVE, POSITIVE}; 
	struct rlib_rgb color[MAX_COLOR_POOL];
	gchar type[MAXSTRLEN], subtype[MAXSTRLEN], title[MAXSTRLEN], x_axis_title[MAXSTRLEN], y_axis_title[MAXSTRLEN], y_axis_title_right[MAXSTRLEN], side_str[MAXSTRLEN];
	gint graph_type;
	gboolean divide_iterations = TRUE;
	gboolean should_label_under_tick = FALSE;
	gfloat value = 0;
	gint did_set = FALSE;
	
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
	if(!rlib_execute_as_string(r, graph->y_axis_title_right_code, y_axis_title_right, MAXSTRLEN))
		y_axis_title_right[0] = 0;

	if(!rlib_will_this_fit(r, part, report, graph_height / RLIB_PDF_DPI, 1)) {
		*top_margin_offset = 0;
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
			

	row_count = 0;
	rlib_fetch_first_rows(r);
	if(!INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result)) {
		while(1) {
			row_count++;
			if(rlib_navigate_next(r, r->current_result) == FALSE) 
				break;
		}
	}

/*	if(row_count <= 1)
		return 0;*/
	
	rlib_fetch_first_rows(r);
	row_count = 0;
	OUTPUT(r)->graph_start(r, left_margin_offset, rlib_layout_get_next_line_by_font_point(r, part, part->position_top[0]+(*top_margin_offset), 0), graph_width, graph_height, should_label_under_tick);
	
	rlib_fetch_first_rows(r);
	if(!INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result)) {
		while (1) {
			data_plot_count = 0;
			stacked_y_value_max[RLIB_SIDE_LEFT] = 0;;
			stacked_y_value_max[RLIB_SIDE_RIGHT] = 0;;
			stacked_y_value_min[RLIB_SIDE_LEFT] = 0;
			stacked_y_value_min[RLIB_SIDE_RIGHT] = 0;
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
							side = RLIB_SIDE_LEFT;
							if(rlib_execute_as_string(r, plot->side_code, side_str, MAXSTRLEN)) {
								if(strcmp(side_str, "right") == 0) {
									side = RLIB_SIDE_RIGHT;
									have_right_side = TRUE;
								}
							}
							
							if(rlib_execute_as_string(r, plot->label_code, legend_label, MAXSTRLEN)) {
								OUTPUT(r)->graph_hint_legend(r, legend_label);
							}
							if(row_count == 0 && did_set == FALSE) {
								y_min[side] = y_max[side] = y_value;
								did_set = TRUE;
							}
							
							if(is_percent_graph(graph_type) || is_pie_graph(graph_type)) {
								y_value = fabs(y_value);
								row_sum[row_count] += y_value;
							}
							
							if(is_stacked_graph(graph_type)) { 
								if( y_value >= 0 ) {
									if( stacked_y_value_max < 0 ) 
										stacked_y_value_max[side] = y_value;
									else
										stacked_y_value_max[side] += y_value;
									stacked_y_value_min[side] = y_value;
								} else { 
									if( stacked_y_value_min[side] > 0 ) {
										stacked_y_value_min[side] = 0;
									}
									stacked_y_value_min[side] += y_value;
									stacked_y_value_max[side] = y_value;
								}
								y_value_try_max[side] = stacked_y_value_max[side];
								y_value_try_min[side] = stacked_y_value_min[side];
							} else {
								y_value_try_max[side] = y_value;
								y_value_try_min[side] = y_value;
							}
							rlib_parsecolor(&color[data_plot_count%MAX_COLOR_POOL], color_pool[data_plot_count%MAX_COLOR_POOL]);
							data_plot_count++;
							if(y_value_try_min[side] < y_min[side])
								y_min[side] = y_value_try_min[side];
							if(y_value_try_max[side] > y_max[side])
								y_max[side] = y_value_try_max[side];
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
		y_min[RLIB_SIDE_LEFT] = y_min[RLIB_SIDE_RIGHT] = 0;
		y_max[RLIB_SIDE_LEFT] = y_max[RLIB_SIDE_RIGHT] = 100;
		y_ticks = 10;
	} else {
		if(y_min[RLIB_SIDE_LEFT] == y_max[RLIB_SIDE_LEFT]) {
			y_min[RLIB_SIDE_LEFT] = 0;
			y_ticks = 1;
		} else {
			adjust_limits(y_min[RLIB_SIDE_LEFT], y_max[RLIB_SIDE_LEFT], is_row_graph(graph_type), 5, 11, &y_ticks, &tmi, &y_min[RLIB_SIDE_LEFT], &y_max[RLIB_SIDE_LEFT]);
		}
		if(have_right_side) {
			if(y_min[RLIB_SIDE_RIGHT] == y_max[RLIB_SIDE_RIGHT]) {
				y_min[RLIB_SIDE_RIGHT] = 0;
				y_ticks = 1;
			} else {
				adjust_limits(y_min[RLIB_SIDE_RIGHT], y_max[RLIB_SIDE_RIGHT], is_row_graph(graph_type), 2, y_ticks, &fake_y_ticks, &tmi, &y_min[RLIB_SIDE_RIGHT], &y_max[RLIB_SIDE_RIGHT]);
			}
		}

	}

	rlib_graph_label_y_axis(r, RLIB_SIDE_LEFT, FALSE, y_ticks, y_min[RLIB_SIDE_LEFT], y_max[RLIB_SIDE_LEFT], y_origin[RLIB_SIDE_LEFT]);
	if(have_right_side)
		rlib_graph_label_y_axis(r, RLIB_SIDE_RIGHT, FALSE, y_ticks, y_min[RLIB_SIDE_RIGHT], y_max[RLIB_SIDE_RIGHT], y_origin[RLIB_SIDE_RIGHT]);

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
		OUTPUT(r)->graph_y_axis_title(r, RLIB_SIDE_LEFT, y_axis_title);
		OUTPUT(r)->graph_y_axis_title(r, RLIB_SIDE_RIGHT, y_axis_title_right);
		OUTPUT(r)->graph_set_x_iterations(r, row_count);
		OUTPUT(r)->graph_do_grid(r, FALSE);
		OUTPUT(r)->graph_tick_x(r);
		OUTPUT(r)->graph_tick_y(r, y_ticks);
	} else {
		OUTPUT(r)->graph_do_grid(r, TRUE);
	}
	
	for(i=0;i<=1;i++) {
		gint use_side;
		if(i == 0)
			use_side = RLIB_SIDE_LEFT;
		else
			use_side = RLIB_SIDE_RIGHT;
		
		if(i == 1 && have_right_side == FALSE)
			break;
			
		if(y_max[use_side] >= 0 && y_min[use_side] >= 0) {
			y_origin[use_side] = y_min[use_side];
			data_type[use_side] = POSITIVE;
		} else if(y_max[use_side] > 0 && y_min[use_side] < 0) {
			y_origin[use_side] = 0;
			data_type[use_side] = POSITIVE_AND_NEGATIVE;
		} else if(y_max[use_side] <= 0 && y_min[use_side] < 0) {
			y_origin[use_side] = y_max[use_side];
			data_type[use_side] = NEGATIVE;
		}
	}
		
	if(!is_pie_graph(graph_type)) {
		OUTPUT(r)->graph_set_limits(r, RLIB_SIDE_LEFT, y_min[RLIB_SIDE_LEFT], y_max[RLIB_SIDE_LEFT], y_origin[RLIB_SIDE_LEFT]);
		rlib_graph_label_y_axis(r, RLIB_SIDE_LEFT, TRUE, y_ticks, y_min[RLIB_SIDE_LEFT], y_max[RLIB_SIDE_LEFT], y_origin[RLIB_SIDE_LEFT]);
		if(have_right_side) {
			OUTPUT(r)->graph_set_limits(r, RLIB_SIDE_RIGHT, y_min[RLIB_SIDE_RIGHT], y_max[RLIB_SIDE_RIGHT], y_origin[RLIB_SIDE_RIGHT]);
			rlib_graph_label_y_axis(r, RLIB_SIDE_RIGHT, TRUE, y_ticks, y_min[RLIB_SIDE_RIGHT], y_max[RLIB_SIDE_RIGHT], y_origin[RLIB_SIDE_RIGHT]);		
		}
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
							side = RLIB_SIDE_LEFT;
							if(rlib_execute_as_string(r, plot->side_code, side_str, MAXSTRLEN)) {
								if(strcmp(side_str, "right") == 0) {
									side = RLIB_SIDE_RIGHT;
									have_right_side = TRUE;
								}
							}
							if(is_percent_graph(graph_type) || is_pie_graph(graph_type)) {
								y_value = fabs(y_value);
								if(is_pie_graph(graph_type))
									y_max[RLIB_SIDE_LEFT] = col_sum;
								else
									y_max[RLIB_SIDE_LEFT] = row_sum[row_count];
								y_min[RLIB_SIDE_LEFT] = 0;
							}

							value = 0;
						
							if(data_type[side] == POSITIVE_AND_NEGATIVE) {
								if(value >= 0)
									value = (y_value / y_max[side]) * (fabs(y_max[side])/(fabs(y_min[side])+fabs(y_max[side])));
								else
									value = (y_value / y_min[side]) * (fabs(y_min[side])/(fabs(y_min[side])+fabs(y_max[side])));
							} else if(data_type[side] == NEGATIVE) {
								value = -((y_value - y_origin[side]) / (y_min[side] - y_origin[side]));
							} else {
								value = (y_value - y_origin[side]) / (y_max[side] - y_origin[side]);
							}
							plot_count = data_plot_count;
							if(is_percent_graph(graph_type) || is_stacked_graph(graph_type) || is_pie_graph(graph_type)) { 
								plot_count = 0;
								if( value >= 0 ) 
									last_height = last_height_pos;
								else
									last_height = last_height_neg;
							} 
							if(is_row_graph(graph_type)) {
								OUTPUT(r)->graph_plot_bar(r, side, row_count, plot_count, value, &color[data_plot_count],last_height, divide_iterations);
							} else if(is_line_graph(graph_type)) {
								if(row_count > 0)
									OUTPUT(r)->graph_plot_line(r, side, row_count, last_row_values[i], last_row_height[i], value, last_height, &color[data_plot_count]);
							} else if(is_pie_graph(graph_type)) {
								gboolean offset = graph_type == RLIB_GRAPH_TYPE_PIE_OFFSET;
								OUTPUT(r)->graph_plot_pie(r, running_col_sum, value+running_col_sum, offset, &color[row_count]);
								running_col_sum += value;
								if(rlib_execute_as_string(r, plot->label_code, legend_label, MAXSTRLEN))
									OUTPUT(r)->graph_draw_legend_label(r, row_count, legend_label, &color[row_count]);
					
							}
								
							if(is_percent_graph(graph_type) || is_stacked_graph(graph_type) || is_pie_graph(graph_type)) { 
								if(value >= 0 ) 
									last_height_pos += value;
								else
									last_height_neg += value;
							}
							
							last_row_values[i] = value;
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
	OUTPUT(r)->graph_finalize(r);
	return graph_height / RLIB_PDF_DPI;
}
