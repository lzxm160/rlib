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

#include "ralloc.h"
#include "rlib.h"

struct _private {
	char *top;
	int top_size;
	int top_total_size;
	char *bottom;
	int bottom_size;
	int bottom_total_size;
	int bg_backwards;
};

static void make_more_space_if_necessary(char **str, int *size, int *total_size, int len) {
	if(*total_size == 0) {
		*str = rcalloc(MAXSTRLEN, 1);
		*total_size = MAXSTRLEN;
	} else if((*size) + len > (*total_size)) {
		*str = rrealloc(*str, (*total_size)*2);
		*total_size = (*total_size) * 2;
	}		
}

static void print_text(rlib *r, char *text, int backwards) {
	char *str_ptr;
	int text_size = strlen(text);
	int *size;

	if(backwards) {
		make_more_space_if_necessary(&OUTPUT_PRIVATE(r)->bottom, &OUTPUT_PRIVATE(r)->bottom_size, 
			&OUTPUT_PRIVATE(r)->bottom_total_size, text_size);
		str_ptr = OUTPUT_PRIVATE(r)->bottom;
		size = &OUTPUT_PRIVATE(r)->bottom_size;
	} else {
		make_more_space_if_necessary(&OUTPUT_PRIVATE(r)->top, &OUTPUT_PRIVATE(r)->top_size, 
			&OUTPUT_PRIVATE(r)->top_total_size, text_size);
		str_ptr = OUTPUT_PRIVATE(r)->top;	
		size = &OUTPUT_PRIVATE(r)->top_size;
	}
	memcpy(str_ptr + (*size), text, text_size+1);
	*size = (*size) + text_size;
}

static float rlib_txt_get_string_width(rlib *r, char *text) {
	return 1;
}

static void rlib_txt_print_text(rlib *r, float left_origin, float bottom_origin, char *text, int backwards, int col) {
	OUTPUT_PRIVATE(r)->bg_backwards = backwards;
	print_text(r, text, backwards);
}


static void rlib_txt_set_fg_color(rlib *r, float red, float green, float blue) {}
static void rlib_txt_set_bg_color(rlib *r, float red, float green, float blue) {}
static void rlib_txt_hr(rlib *r, int backwards, float left_origin, float bottom_origin, float how_long, float how_tall, 
struct rgb *color, float indent, float length) {}
static void rlib_txt_draw_cell_background_start(rlib *r, float left_origin, float bottom_origin, float how_long, float how_tall, struct rgb *color) {}
static void rlib_txt_draw_cell_background_end(rlib *r) {}
static void rlib_txt_boxurl_start(rlib *r, float left_origin, float bottom_origin, float how_long, float how_tall, char *url) {}
static void rlib_txt_boxurl_end(rlib *r) {}
static void rlib_txt_drawimage(rlib *r, float left_origin, float bottom_origin, char *nname, char *type, float nwidth, 
	float nheight) {}

static void rlib_txt_set_font_point(rlib *r, int point) {}
static void rlib_txt_start_new_page(rlib *r) {
	r->position_bottom = 11-GET_MARGIN(r)->bottom_margin;
}

static void rlib_txt_init_end_page(rlib *r) {}

static void rlib_txt_end_text(rlib *r) {}

static void rlib_txt_init_output(rlib *r) {}

static void rlib_txt_begin_text(rlib *r) {}

static void rlib_txt_finalize_private(rlib *r) {
	r->length = OUTPUT_PRIVATE(r)->top_size + OUTPUT_PRIVATE(r)->bottom_size;
}

static void rlib_txt_spool_private(rlib *r) {
	ENVIRONMENT(r)->rlib_write_output(OUTPUT_PRIVATE(r)->top, strlen(OUTPUT_PRIVATE(r)->top));
	ENVIRONMENT(r)->rlib_write_output(OUTPUT_PRIVATE(r)->bottom, strlen(OUTPUT_PRIVATE(r)->bottom));
}

static void rlib_txt_start_line(rlib *r, int backwards) {}

static void rlib_txt_end_line(rlib *r, int backwards) {
	print_text(r, "\n", backwards);
}

static void rlib_txt_start_output_section(rlib *r) {}
static void rlib_txt_end_output_section(rlib *r) {}
static void rlib_txt_init_output_report(rlib *r) {}

static void rlib_txt_end_page(rlib *r) {
	r->current_page_number++;
	r->current_line_number = 1;
	rlib_init_page(r, FALSE);
}

static int rlib_txt_is_single_page(rlib *r) {
	return TRUE;
}

static int rlib_txt_free(rlib *r) {
	rfree(OUTPUT_PRIVATE(r));
	rfree(OUTPUT(r));
}


void rlib_txt_new_output_filter(rlib *r) {
	OUTPUT(r) = rmalloc(sizeof(struct output_filter));
	OUTPUT_PRIVATE(r) = rmalloc(sizeof(struct _private));
	bzero(OUTPUT_PRIVATE(r), sizeof(struct _private));
	OUTPUT_PRIVATE(r)->top = NULL;
	OUTPUT_PRIVATE(r)->top_size = 0;
	OUTPUT_PRIVATE(r)->top_total_size = 0;
	OUTPUT_PRIVATE(r)->bottom = NULL;
	OUTPUT_PRIVATE(r)->bottom_size = 0;
	OUTPUT_PRIVATE(r)->bottom_total_size = 0;
	
	OUTPUT(r)->do_align = TRUE;
	OUTPUT(r)->do_break = TRUE;

	OUTPUT(r)->rlib_get_string_width = rlib_txt_get_string_width;
	OUTPUT(r)->rlib_print_text = rlib_txt_print_text;
	OUTPUT(r)->rlib_set_fg_color = rlib_txt_set_fg_color;
	OUTPUT(r)->rlib_set_bg_color = rlib_txt_set_bg_color;
	OUTPUT(r)->rlib_hr = rlib_txt_hr;
	OUTPUT(r)->rlib_draw_cell_background_start = rlib_txt_draw_cell_background_start;
	OUTPUT(r)->rlib_draw_cell_background_end = rlib_txt_draw_cell_background_end;
	OUTPUT(r)->rlib_boxurl_start = rlib_txt_boxurl_start;
	OUTPUT(r)->rlib_boxurl_end = rlib_txt_boxurl_end;
	OUTPUT(r)->rlib_drawimage = rlib_txt_drawimage;
	OUTPUT(r)->rlib_set_font_point = rlib_txt_set_font_point;
	OUTPUT(r)->rlib_start_new_page = rlib_txt_start_new_page;
	OUTPUT(r)->rlib_end_page = rlib_txt_end_page;	
	OUTPUT(r)->rlib_init_end_page = rlib_txt_init_end_page;
	OUTPUT(r)->rlib_end_text = rlib_txt_end_text;
	OUTPUT(r)->rlib_init_output = rlib_txt_init_output;
	OUTPUT(r)->rlib_init_output_report = rlib_txt_init_output_report;
	OUTPUT(r)->rlib_begin_text = rlib_txt_begin_text;
	OUTPUT(r)->rlib_finalize_private = rlib_txt_finalize_private;
	OUTPUT(r)->rlib_spool_private = rlib_txt_spool_private;
	OUTPUT(r)->rlib_start_line = rlib_txt_start_line;
	OUTPUT(r)->rlib_end_line = rlib_txt_end_line;
	OUTPUT(r)->rlib_is_single_page = rlib_txt_is_single_page;
	OUTPUT(r)->rlib_start_output_section = rlib_txt_start_output_section;	
	OUTPUT(r)->rlib_end_output_section = rlib_txt_end_output_section;	
	OUTPUT(r)->rlib_free = rlib_txt_free;	
}
