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

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "rlib.h"

#define MAX_COL	100

struct _private {
	gchar col[MAX_COL][MAXSTRLEN];
	gchar *top;
	gint top_size;
	gint top_total_size;
	gint length;
};

static void print_text(rlib *r, gchar *text, gint backwards, gint col) {
	if(col < MAX_COL) {
		if(OUTPUT_PRIVATE(r)->col[col][0] == 0)
			memcpy(OUTPUT_PRIVATE(r)->col[col], text, strlen(text)+1);
		else
			sprintf(OUTPUT_PRIVATE(r)->col[col], "%s %s", OUTPUT_PRIVATE(r)->col[col], text);
	}
}

static gfloat rlib_csv_get_string_width(rlib *r, gchar *text) {
	return 1;
}

static void rlib_csv_print_text(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *text, gint backwards, gint col) {
	if(col) {
		print_text(r, text, backwards, col-1);
	}
}


static void rlib_csv_set_fg_color(rlib *r, gfloat red, gfloat green, gfloat blue) {}
static void rlib_csv_set_bg_color(rlib *r, gfloat red, gfloat green, gfloat blue) {}
static void rlib_csv_hr(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, 
struct rlib_rgb *color, gfloat indent, gfloat length) {}
static void rlib_csv_draw_cell_background_start(rlib *r, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, struct rlib_rgb *color) {}
static void rlib_csv_draw_cell_background_end(rlib *r) {}
static void rlib_csv_boxurl_start(rlib *r, struct rlib_part *part, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, gchar *url) {}
static void rlib_csv_boxurl_end(rlib *r) {}
static void rlib_csv_drawimage(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *nname, gchar *type, gfloat nwidth, 
	float nheight) {}

static void rlib_csv_set_font_point(rlib *r, gint point) {}

static void rlib_csv_start_new_page(rlib *r, struct rlib_part *part) {
	part->position_bottom[0] = 11-part->bottom_margin;
}

static void rlib_csv_init_end_page(rlib *r) {}

static void rlib_csv_init_output(rlib *r) {}

static void rlib_csv_finalize_private(rlib *r) {
	OUTPUT_PRIVATE(r)->length = OUTPUT_PRIVATE(r)->top_size;
}

static void rlib_csv_spool_private(rlib *r) {
	ENVIRONMENT(r)->rlib_write_output(OUTPUT_PRIVATE(r)->top, strlen(OUTPUT_PRIVATE(r)->top));
}

static void rlib_csv_start_line(rlib *r, gint backwards) {}

static void really_print_text(rlib *r, gchar *text) {
	gchar buf[MAXSTRLEN];
	gchar *str_ptr;
	gint text_size = strlen(text);
	gint *size;

	if(text != NULL && text[0] != '\n') {
		sprintf(buf, "\"%s\",", text);
		text_size += 3;
	} else {
		strcpy(buf, text);
	}
	
	make_more_space_if_necessary(&OUTPUT_PRIVATE(r)->top, &OUTPUT_PRIVATE(r)->top_size, 
		&OUTPUT_PRIVATE(r)->top_total_size, text_size);
	str_ptr = OUTPUT_PRIVATE(r)->top;	
	size = &OUTPUT_PRIVATE(r)->top_size;
	memcpy(str_ptr + (*size), buf, text_size+1);
	*size = (*size) + text_size;

}

static void rlib_csv_end_line(rlib *r, gint backwards) {}
static void rlib_csv_start_report(rlib *r, struct rlib_part *part) {}
static void rlib_csv_end_report(rlib *r, struct rlib_part *part, struct rlib_report *report) {}

static void rlib_csv_start_output_section(rlib *r) {
	gint i;
	for(i=0;i<MAX_COL;i++) {
		OUTPUT_PRIVATE(r)->col[i][0] = 0;
	}
}

static void rlib_csv_end_output_section(rlib *r) {
	gint i;
	gint biggest = 0;
	for(i=0;i<MAX_COL;i++)
		if(OUTPUT_PRIVATE(r)->col[i][0] != 0)
			biggest = i;
	
	for(i=0;i<=biggest;i++)
		if(OUTPUT_PRIVATE(r)->col[i][0] != 0) 
			really_print_text(r, OUTPUT_PRIVATE(r)->col[i]);
		else
			really_print_text(r, "");
			
	really_print_text(r, "\n");	
}

static void rlib_csv_end_page(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	r->current_page_number++;
	r->current_line_number = 1;
}

static int rlib_csv_is_single_page(rlib *r) {
	return TRUE;
}

static char *rlib_csv_get_output(rlib *r) {
	return OUTPUT_PRIVATE(r)->top;
}

static long rlib_csv_get_output_length(rlib *r) {
	return OUTPUT_PRIVATE(r)->top_size;
}

static void rlib_csv_set_working_page(rlib *r, struct rlib_part *part, gint page) {
	return;
}

static int rlib_csv_free(rlib *r) {
	g_free(OUTPUT_PRIVATE(r)->top);
	g_free(OUTPUT_PRIVATE(r));
	g_free(OUTPUT(r));
	return 0;
}

void rlib_csv_new_output_filter(rlib *r) {
	OUTPUT(r) = g_malloc(sizeof(struct output_filter));
	r->o->private = g_malloc(sizeof(struct _private));
	memset(OUTPUT_PRIVATE(r), 0, sizeof(struct _private));
	OUTPUT_PRIVATE(r)->top = NULL;
	OUTPUT_PRIVATE(r)->top_size = 0;
	OUTPUT_PRIVATE(r)->top_total_size = 0;

	OUTPUT(r)->do_align = FALSE;	
	OUTPUT(r)->do_break = FALSE;	
	OUTPUT(r)->do_grouptext = FALSE;

	OUTPUT(r)->get_string_width = rlib_csv_get_string_width;
	OUTPUT(r)->print_text = rlib_csv_print_text;
	OUTPUT(r)->set_fg_color = rlib_csv_set_fg_color;
	OUTPUT(r)->set_bg_color = rlib_csv_set_bg_color;
	OUTPUT(r)->hr = rlib_csv_hr;
	OUTPUT(r)->draw_cell_background_start = rlib_csv_draw_cell_background_start;
	OUTPUT(r)->draw_cell_background_end = rlib_csv_draw_cell_background_end;
	OUTPUT(r)->boxurl_start = rlib_csv_boxurl_start;
	OUTPUT(r)->boxurl_end = rlib_csv_boxurl_end;
	OUTPUT(r)->drawimage = rlib_csv_drawimage;
	OUTPUT(r)->set_font_point = rlib_csv_set_font_point;
	OUTPUT(r)->start_new_page = rlib_csv_start_new_page;
	OUTPUT(r)->end_page = rlib_csv_end_page;   
	OUTPUT(r)->init_end_page = rlib_csv_init_end_page;
	OUTPUT(r)->init_output = rlib_csv_init_output;
	OUTPUT(r)->start_report = rlib_csv_start_report;
	OUTPUT(r)->end_report = rlib_csv_end_report;
	OUTPUT(r)->finalize_private = rlib_csv_finalize_private;
	OUTPUT(r)->spool_private = rlib_csv_spool_private;
	OUTPUT(r)->start_line = rlib_csv_start_line;
	OUTPUT(r)->end_line = rlib_csv_end_line;
	OUTPUT(r)->is_single_page = rlib_csv_is_single_page;
	OUTPUT(r)->start_output_section = rlib_csv_start_output_section;   
	OUTPUT(r)->end_output_section = rlib_csv_end_output_section; 
	OUTPUT(r)->get_output = rlib_csv_get_output;
	OUTPUT(r)->get_output_length = rlib_csv_get_output_length;
	OUTPUT(r)->set_working_page = rlib_csv_set_working_page;  
	OUTPUT(r)->free = rlib_csv_free;
}
