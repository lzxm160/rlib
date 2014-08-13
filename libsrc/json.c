/*
 *  Copyright (C) 2003-2006 SICOM Systems, INC.
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

#include <stdlib.h>
#include <string.h> 

#include "config.h"
#include "rlib.h"

#define TEXT 1
#define DELAY 2

struct _packet {
	char type;
	gpointer data;
};

struct _private {
};


static gfloat json_get_string_width(rlib *r, const gchar *text) {
	return 1;
}

static void json_print_text(rlib *r, gfloat left_origin, gfloat bottom_origin, const gchar *text, gint backwards, gint col, gint rval_type) {
}

static void json_start_new_page(rlib *r, struct rlib_part *part) {
}

static void json_init_end_page(rlib *r) {}
static void json_init_output(rlib *r) {}
static void json_finalize_private(rlib *r) {}

static void json_spool_private(rlib *r) {
	
}

static void json_end_line(rlib *r, int backwards) {
}

static void json_start_part(rlib *r, struct rlib_part *part) {
	printf("<part>\n");
}

static void json_start_report(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	printf("<report>\n");
}

static void json_end_report(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	printf("</report>\n");
}

static void json_start_report_field_headers(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	printf("<field_headers>\n");
}

static void json_end_report_field_headers(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	printf("</field_headers>\n");
}

static void json_start_report_field_details(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	printf("<field_details>\n");
}

static void json_end_report_field_details(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	printf("</field_details>\n");
}

static void json_start_report_header(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	printf("<report_header>\n");
}

static void json_end_report_header(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	printf("</report_header>\n");
}

static void json_start_report_footer(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	printf("<report_footer>\n");
}

static void json_end_report_footer(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	printf("</report_footer>\n");
}


static void json_start_tr(rlib *r) {
	printf("<tr>\n");
}

static void json_end_tr(rlib *r) {
	printf("</tr>\n");
}

static void json_start_td(rlib *r, struct rlib_part *part, gfloat left_margin, gfloat top_margin, int width, int height, int border_width, struct rlib_rgb *color) {
	printf("<td>\n");
}

static void json_end_td(rlib *r) {
	printf("</td>\n");
}

static void json_end_part(rlib *r, struct rlib_part *part) {
	printf("</part>\n");
}

static void json_start_output_section(rlib *r) {
	printf("<output>\n");
}

static void json_end_output_section(rlib *r) {
	printf("</output>\n");	
}

static void json_start_part_header(rlib *r, struct rlib_part *part) {
	printf("<part_header>\n");
}

static void json_end_part_header(rlib *r, struct rlib_part *part) {
	printf("</part_header>\n");
}

static void json_start_part_page_header(rlib *r, struct rlib_part *part) {
	printf("<part_page_header>\n");
}

static void json_end_part_page_header(rlib *r, struct rlib_part *part) {
	printf("</part_page_header>\n");
}


static void json_start_part_page_footer(rlib *r, struct rlib_part *part) {
	printf("<part_footer>\n");
}

static void json_end_part_page_footer(rlib *r, struct rlib_part *part) {
	printf("</part_footer>\n");
}


static void json_print_text_delayed(rlib *r, struct rlib_delayed_extra_data *delayed_data, int backwards, int rval_type) {
}



static void json_end_page(rlib *r, struct rlib_part *part) {
	r->current_page_number++;
	r->current_line_number = 1;
}

static int json_free(rlib *r) {
	
	g_free(OUTPUT_PRIVATE(r));
	g_free(OUTPUT(r));
	return 0;
}

static char *json_get_output(rlib *r) {
	return "TODO";
}

static long json_get_output_length(rlib *r) {
	return 1;
}

static void json_set_working_page(rlib *r, struct rlib_part *part, int page) {
	
}

static void json_set_fg_color(rlib *r, gfloat red, gfloat green, gfloat blue) {}
static void json_set_bg_color(rlib *r, gfloat red, gfloat green, gfloat blue) {}
static void json_hr(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, 
struct rlib_rgb *color, gfloat indent, gfloat length) {}
static void json_start_draw_cell_background(rlib *r, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, 
struct rlib_rgb *color) {}
static void json_end_draw_cell_background(rlib *r) {}
static void json_start_boxurl(rlib *r, struct rlib_part * part, gfloat
left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, gchar *url, gint backwards) {}
static void json_end_boxurl(rlib *r, gint backwards) {}
static void json_background_image(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *nname, gchar *type, gfloat nwidth, gfloat nheight) {}
static void json_set_font_point(rlib *r, gint point) {}
static void json_start_line(rlib *r, gint backwards) {}
static void json_set_raw_page(rlib *r, struct rlib_part *part, gint page) {}
static void json_start_bold(rlib *r) {}
static void json_end_bold(rlib *r) {}
static void json_start_italics(rlib *r) {}
static void json_end_italics(rlib *r) {}

static void json_graph_start(rlib *r, gfloat left, gfloat top, gfloat width, gfloat height, gboolean x_axis_labels_are_under_tick) {}
static void json_graph_set_limits(rlib *r, gchar side, gdouble min, gdouble max, gdouble origin) {}
static void json_graph_set_title(rlib *r, gchar *title) {}
static void json_graph_x_axis_title(rlib *r, gchar *title) {}
static void json_graph_y_axis_title(rlib *r, gchar side, gchar *title) {}
static void json_graph_do_grid(rlib *r, gboolean just_a_box) {}
static void json_graph_tick_x(rlib *r) {}
static void json_graph_set_x_iterations(rlib *r, gint iterations) {}
static void json_graph_hint_label_x(rlib *r, gchar *label) {}
static void json_graph_label_x(rlib *r, gint iteration, gchar *label) {}
static void json_graph_tick_y(rlib *r, gint iterations) {}
static void json_graph_label_y(rlib *r, gchar side, gint iteration, gchar *label, gboolean false_x) {}
static void json_graph_hint_label_y(rlib *r, gchar side, gchar *label) {}
static void json_graph_set_data_plot_count(rlib *r, gint count) {}
static void json_graph_plot_bar(rlib *r, gchar side, gint iteration, gint plot, gfloat height_percent, struct rlib_rgb *color,gfloat last_height, gboolean divide_iterations) {}
static void json_graph_plot_line(rlib *r, gchar side, gint iteration, gfloat p1_height, gfloat p1_last_height, gfloat p2_height, gfloat p2_last_height, struct rlib_rgb * color) {}
static void json_graph_plot_pie(rlib *r, gfloat start, gfloat end, gboolean offset, struct rlib_rgb *color) {}
static void json_graph_hint_legend(rlib *r, gchar *label) {}
static void json_graph_draw_legend(rlib *r) {}
static void json_graph_draw_legend_label(rlib *r, gint iteration, gchar *label, struct rlib_rgb *color, gboolean is_line) {}
static void json_graph_finalize(rlib *r) {}
static void json_graph_draw_line(rlib *r, gfloat x, gfloat y, gfloat new_x, gfloat new_y, struct rlib_rgb *color) {}
static void json_graph_set_name(rlib *r, gchar *name) {}
static void json_graph_set_legend_bg_color(rlib *r, struct rlib_rgb *rgb) {}
static void json_graph_set_legend_orientation(rlib *r, gint orientation) {}
static void json_graph_set_draw_x_y(rlib *r, gboolean draw_x, gboolean draw_y) {}
static void json_graph_set_bold_titles(rlib *r, gboolean bold_titles) {}
static void json_graph_set_grid_color(rlib *r, struct rlib_rgb *rgb) {}

void rlib_json_new_output_filter(rlib *r) {
	OUTPUT(r) = g_malloc(sizeof(struct output_filter));
	r->o->private = g_malloc(sizeof(struct _private));
	memset(OUTPUT_PRIVATE(r), 0, sizeof(struct _private));
	
	OUTPUT(r)->do_align = TRUE;
	OUTPUT(r)->do_breaks = TRUE;
	OUTPUT(r)->do_grouptext = FALSE;	
	OUTPUT(r)->paginate = FALSE;
	OUTPUT(r)->trim_links = FALSE;
	OUTPUT(r)->do_graph = FALSE;
	
	OUTPUT(r)->get_string_width = json_get_string_width;
	OUTPUT(r)->print_text = json_print_text;
	OUTPUT(r)->set_fg_color = json_set_fg_color;
	OUTPUT(r)->set_bg_color = json_set_bg_color;
	OUTPUT(r)->print_text_delayed = json_print_text_delayed;	
	OUTPUT(r)->hr = json_hr;
	OUTPUT(r)->start_draw_cell_background = json_start_draw_cell_background;
	OUTPUT(r)->end_draw_cell_background = json_end_draw_cell_background;
	OUTPUT(r)->start_boxurl = json_start_boxurl;
	OUTPUT(r)->end_boxurl = json_end_boxurl;
	OUTPUT(r)->background_image = json_background_image;
	OUTPUT(r)->line_image = json_background_image;
	OUTPUT(r)->set_font_point = json_set_font_point;
	OUTPUT(r)->start_new_page = json_start_new_page;
	OUTPUT(r)->end_page = json_end_page;   
	OUTPUT(r)->init_end_page = json_init_end_page;
	OUTPUT(r)->init_output = json_init_output;
	OUTPUT(r)->start_report = json_start_report;
	OUTPUT(r)->end_report = json_end_report;
	OUTPUT(r)->start_report_field_headers = json_start_report_field_headers;
	OUTPUT(r)->end_report_field_headers = json_end_report_field_headers;
	OUTPUT(r)->start_report_field_details = json_start_report_field_details;
	OUTPUT(r)->end_report_field_details = json_end_report_field_details;
	OUTPUT(r)->start_part = json_start_part;
	OUTPUT(r)->end_part = json_end_part;
	OUTPUT(r)->start_report_header = json_start_report_header;
	OUTPUT(r)->end_report_header = json_end_report_header;
	OUTPUT(r)->start_report_footer = json_start_report_footer;
	OUTPUT(r)->end_report_footer = json_end_report_footer;
	OUTPUT(r)->start_part_header = json_start_part_header;
	OUTPUT(r)->end_part_header = json_end_part_header;
	OUTPUT(r)->start_part_page_header = json_start_part_page_header;
	OUTPUT(r)->end_part_page_header = json_end_part_page_header;
	OUTPUT(r)->start_part_page_footer = json_start_part_page_footer;
	OUTPUT(r)->end_part_page_footer = json_end_part_page_footer;


	OUTPUT(r)->finalize_private = json_finalize_private;
	OUTPUT(r)->spool_private = json_spool_private;
	OUTPUT(r)->start_line = json_start_line;
	OUTPUT(r)->end_line = json_end_line;
	OUTPUT(r)->start_output_section = json_start_output_section;   
	OUTPUT(r)->end_output_section = json_end_output_section; 
	OUTPUT(r)->get_output = json_get_output;
	OUTPUT(r)->get_output_length = json_get_output_length;
	OUTPUT(r)->set_working_page = json_set_working_page;  
	OUTPUT(r)->set_raw_page = json_set_raw_page; 
	OUTPUT(r)->start_tr = json_start_tr; 
	OUTPUT(r)->end_tr = json_end_tr; 
	OUTPUT(r)->start_td = json_start_td; 
	OUTPUT(r)->end_td = json_end_td; 
	OUTPUT(r)->start_bold = json_start_bold;
	OUTPUT(r)->end_bold = json_end_bold;
	OUTPUT(r)->start_italics = json_start_italics;
	OUTPUT(r)->end_italics = json_end_italics;

	OUTPUT(r)->graph_start = json_graph_start;
	OUTPUT(r)->graph_set_limits = json_graph_set_limits;
	OUTPUT(r)->graph_set_title = json_graph_set_title;
	OUTPUT(r)->graph_set_name = json_graph_set_name;
	OUTPUT(r)->graph_set_legend_bg_color = json_graph_set_legend_bg_color;
	OUTPUT(r)->graph_set_legend_orientation = json_graph_set_legend_orientation;
	OUTPUT(r)->graph_set_draw_x_y = json_graph_set_draw_x_y;
	OUTPUT(r)->graph_set_bold_titles = json_graph_set_bold_titles;
	OUTPUT(r)->graph_set_grid_color = json_graph_set_grid_color;
	OUTPUT(r)->graph_x_axis_title = json_graph_x_axis_title;
	OUTPUT(r)->graph_y_axis_title = json_graph_y_axis_title;
	OUTPUT(r)->graph_do_grid = json_graph_do_grid;
	OUTPUT(r)->graph_tick_x = json_graph_tick_x;
	OUTPUT(r)->graph_set_x_iterations = json_graph_set_x_iterations;
	OUTPUT(r)->graph_tick_y = json_graph_tick_y;
	OUTPUT(r)->graph_hint_label_x = json_graph_hint_label_x;
	OUTPUT(r)->graph_label_x = json_graph_label_x;
	OUTPUT(r)->graph_label_y = json_graph_label_y;
	OUTPUT(r)->graph_draw_line = json_graph_draw_line;
	OUTPUT(r)->graph_plot_bar = json_graph_plot_bar;
	OUTPUT(r)->graph_plot_line = json_graph_plot_line;
	OUTPUT(r)->graph_plot_pie = json_graph_plot_pie;
	OUTPUT(r)->graph_set_data_plot_count = json_graph_set_data_plot_count;
	OUTPUT(r)->graph_hint_label_y = json_graph_hint_label_y;
	OUTPUT(r)->graph_hint_legend = json_graph_hint_legend;
	OUTPUT(r)->graph_draw_legend = json_graph_draw_legend;
	OUTPUT(r)->graph_draw_legend_label = json_graph_draw_legend_label;
	OUTPUT(r)->graph_finalize = json_graph_finalize;

	OUTPUT(r)->free = json_free;  
}
